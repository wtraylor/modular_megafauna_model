///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivore interfaces and classes.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_diet.h"
#include "herbiv_energetics.h" // for FatmassEnergyBudget
#include "herbiv_forageenergy.h" // for GetNetEnergyContentInterface
#include "herbiv_foraging.h"
#include "herbiv_herbivore.h"
#include "herbiv_hft.h"
#include "herbiv_mortality.h"
#include "herbiv_reproduction.h"
#include <cfloat> // for DBL_MAX
#include <stdexcept>

using namespace Fauna;

//============================================================
// HerbivoreBase
//============================================================

HerbivoreBase::HerbivoreBase(
		const int age_days,
		const double body_condition,
		const Hft* hft, 
		const Sex sex):
	age_days(age_days),
	hft(hft), // can be NULL
	sex(sex) // always valid
{
	// Check validity of parameters
	if (hft == NULL)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"hft is NULL.");
	if (age_days == 0)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"Establishment constructor called with age_days==0. "
				"Use the birth constructor instead.");
	if (age_days < 0)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"age_days < 0");
	if (get_hft().mortality_factors.count(MF_LIFESPAN) && 
			age_days > get_hft().lifespan*365)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"age_days is greater than maximum lifespan.");
	if (body_condition > 1.0)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"body_condition > 1.0");
	if (body_condition < 0.0)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"body_condition < 0.0");

	// Create energy budget (validity check inside that class)
	energy_budget = std::auto_ptr<FatmassEnergyBudget>(
			new FatmassEnergyBudget(
				body_condition * get_max_fatmass(),// initial fat mass
				get_max_fatmass())); // maximum fat mass
}

HerbivoreBase::HerbivoreBase( const Hft* hft, const Sex sex):
	hft(hft), 
	sex(sex), 
	age_days(0)
{
	// Check validity of parameters
	if (hft == NULL)
		throw std::invalid_argument("Fauna::HerbivoreBase::HerbivoreBase() "
				"hft is NULL.");
	// Create energy budget (validity check inside that class)
	energy_budget = std::auto_ptr<FatmassEnergyBudget>(
			new FatmassEnergyBudget(
				get_hft().bodyfat_birth * get_hft().bodymass_birth,// initial fat mass
				get_max_fatmass())); // maximum fat mass
}

HerbivoreBase::HerbivoreBase(const HerbivoreBase& other):
	age_days(other.age_days),
	// create new energy budget instance
	energy_budget(new FatmassEnergyBudget(other.get_energy_budget())),
	hft(other.hft),
	sex(other.sex)
{
}

HerbivoreBase& HerbivoreBase::operator=(const HerbivoreBase& other){
	if (this != &other){
		age_days = other.age_days;
		// Create a new copy of FatmassEnergyBudget and release the
		// old one.
		energy_budget = std::auto_ptr<FatmassEnergyBudget>(
			new FatmassEnergyBudget(other.get_energy_budget()));
		hft = other.hft;
		sex = other.sex;
	}
	return *this; 
}

void HerbivoreBase::apply_mortality_factors_today(){
	// Sum of death proportions today. Because different mortality
	// factors are thought to be mutually exclusive (i.e. each death
	// event has exactly one causing factor), we just add them up.
	double mortality_sum = 0.0;

	// iterate through all mortality factors.
	std::set<MortalityFactor>::const_iterator itr;
	for (itr=get_hft().mortality_factors.begin();
			itr != get_hft().mortality_factors.end(); itr++){

		if (*itr == MF_BACKGROUND) {
			const GetBackgroundMortality background(
					get_hft().mortality_juvenile,
					get_hft().mortality);
			const double mortality = background(get_age_days());
			mortality_sum += mortality;
			// output:
			get_todays_output().mortality[MF_BACKGROUND] = mortality;
		}

		if (*itr == MF_LIFESPAN) {
			const GetSimpleLifespanMortality lifespan(
					get_hft().lifespan);
			const double mortality = lifespan(get_age_days());
			mortality_sum += mortality;
			// output:
			get_todays_output().mortality[MF_LIFESPAN] = mortality;
		}

		if (*itr == MF_STARVATION_ILLIUS2000 && 
				get_day_of_month(get_today()) == 0) 
		{
			// We apply this starvation model only at the first day of every 
			// month because it is designed for a monthly scheme.

			static const GetStarvationMortalityIllius2000 starv_illius;
			const double body_condition = get_fatmass()/get_max_fatmass();
			const double mortality = starv_illius(body_condition);
			mortality_sum += mortality;
			// output:
			get_todays_output().mortality[MF_STARVATION_ILLIUS2000] = mortality;
		}

		if (*itr == MF_STARVATION_THRESHOLD) {
			static const GetStarvationMortalityThreshold starv_thresh;
			const double mortality = starv_thresh(get_bodyfat());
			mortality_sum += mortality;
			// output:
			get_todays_output().mortality[MF_STARVATION_THRESHOLD] = mortality;
		}

	}
	// make sure that mortality does not exceed 1.0
	mortality_sum = fmin(1.0, mortality_sum);

	// Call pure virtual function, which is implemented by derived
	// classes
	apply_mortality(mortality_sum);
}

ComposeDietInterface& HerbivoreBase::compose_diet()const{
	switch (get_hft().diet_composer){
		case DC_PURE_GRAZER:
			static PureGrazerDiet pure_grazer;
			return pure_grazer;
			// add new diets here
		default:
			throw std::logic_error("Fauna::HerbivoreBase::compose_diet() "
					"Selected diet composer not implemented.");
	}
}

void HerbivoreBase::eat(				
		const ForageMass& kg_per_km2,
		const Digestibility& digestibility){
	if (get_ind_per_km2() == 0.0 && kg_per_km2 == 0.0)
		throw std::logic_error("Fauna::HerbivoreBase::eat() "
				"This herbivore has no individuals and cannot be fed.");

	// convert forage from *per km²* to *per individual*
	assert( get_ind_per_km2() != 0.0 );
	const ForageMass kg_per_ind = kg_per_km2 / get_ind_per_km2();

	// net energy in the forage [MJ/ind]
	// Divide mass by energy content and set any forage with zero
	// energy content to zero mass.
	const ForageEnergy mj_per_ind = 
		get_net_energy_content(digestibility) * kg_per_ind;

	// send energy to energy model
	get_energy_budget().metabolize_energy(mj_per_ind.sum());

	// Add to output
	get_todays_output().eaten_forage  += kg_per_ind;
	get_todays_output().energy_intake += mj_per_ind;
}

double HerbivoreBase::get_bodyfat()const{
	return get_energy_budget().get_fatmass() / get_bodymass();
}

double HerbivoreBase::get_bodymass() const{
	return get_energy_budget().get_fatmass() + get_lean_bodymass();
}

double HerbivoreBase::get_bodymass_adult() const{
	if (get_sex() == SEX_MALE)
		return get_hft().bodymass_male; 
	else
		return get_hft().bodymass_female;
}

double HerbivoreBase::get_fatmass() const{
	return get_energy_budget().get_fatmass();
}

double HerbivoreBase::get_lean_bodymass()const{
	return get_potential_bodymass() * (1.0-get_hft().bodyfat_max);
}

ForageMass HerbivoreBase::get_max_foraging(
		const HabitatForage& available_forage)const
{ 
	const Digestibility digestibility = available_forage.get_digestibility();

	// set the maximum, and then let the foraging limit algorithms
	// reduce the maximum by using fmin()
	ForageMass result(10000); // [kgDM/ind/day]
	// (Note that using DBL_MAX here does not work because converting it to
	//  energy may result in INFINITY values.)

	// Go through all forage intake limits
	std::set<ForagingLimit>::const_iterator itr;
	for (itr=get_hft().foraging_limits.begin(); 
			itr!=get_hft().foraging_limits.end();
			itr++) 
	{ 
		if (*itr == FL_DIGESTION_ILLIUS_1992) {
			// function object
			const GetDigestiveLimitIllius1992 get_digestive_limit(
					get_bodymass_adult(), get_hft().digestion_type);

			// calculate the digestive limit [MJ/day]
			const ForageEnergy limit_mj = 
				get_digestive_limit(get_bodymass(), digestibility);

			const ForageEnergyContent energy_content = 
				get_net_energy_content( digestibility);

			// Convert energy to kg dry matter
			// kg * MJ/kg = kg; where zero values remain zero values even
			// on division by zero.
			const ForageMass limit_kg = limit_mj.divide_safely(
					energy_content, 0.0);

			// Set the maximum foraging limit [kgDM/day]
			result.min(limit_kg);

		} else if (*itr == FL_ILLIUS_OCONNOR_2000) {
			// create function object for maximum intake
			const GetDigestiveLimitIllius1992 get_digestive_limit(
					get_bodymass_adult(), get_hft().digestion_type);

			// Create functional response with digestive limit as maximum.
			const HalfMaxIntake half_max(
					get_hft().half_max_intake_density*1000.0, // gDM/m² to kgDM/km²
					get_digestive_limit(get_bodymass(), digestibility)[FT_GRASS]);

			// Like Pachzelt (2013), we use the whole-habitat grass density,
			// not the ‘sward density’.
			const double grass_limit_mj = half_max.get_intake_rate(
					available_forage.grass.get_mass()); // [MJ/day]

			const ForageEnergyContent energy_content = 
				get_net_energy_content( digestibility);

			double grass_limit_kg;
			if (energy_content[FT_GRASS] > 0.0)
				grass_limit_kg = grass_limit_mj / energy_content[FT_GRASS];
			else
				grass_limit_kg = 0.0; // no energy ⇒ no feeding

			// The Illius & O’Connor (2000) model applies only to grass, and
			// hence we only constrain the grass part of `result`.
			result.set(FT_GRASS, min(result[FT_GRASS], grass_limit_kg));
		} else
			// ADD MORE LIMITS HERE IN NEW IF-STATEMENTS
			throw std::logic_error("Fauna::HerbivoreBase::get_forage_demands_ind() "
					"One of the selected foraging limits is not implemented.");
	}
	return result;
}

ForageMass HerbivoreBase::get_forage_demands(
		const HabitatForage& available_forage)const{

	// ----------------- PREPARE VARIABLES ------------------------

	const Digestibility digestibility = available_forage.get_digestibility();

	const ForageEnergyContent energy_content = 
		get_net_energy_content(digestibility); // [MJ/kg]

	// ----------------- HOW MUCH CAN BE FORAGED? -----------------

	// for each forage type independently: the mass that an 
	// individual could forage (not regarding energy needs) [kg/ind]
	const ForageMass foragable_mass =
		HerbivoreBase::get_max_foraging(available_forage);

	// energy equivalent to foragable_mass [MJ/ind]
	const ForageEnergy foragable_energy = foragable_mass * energy_content; 

	// ----------------- COMPOSE DIET -----------------------------

	// energy demands [MJ/ind] for expenditure plus fat anabolism
	const double total_energy_demands =
		get_energy_budget().get_energy_needs()
		+ get_energy_budget().get_max_anabolism_per_day();

	// compose the diet according to preferences
	const ForageEnergy diet_energy = compose_diet()(
			foragable_energy,
			total_energy_demands);

	// now convert energy back to mass
	// Any forage type with zero energy content will get zero mass.
	const	ForageMass diet_mass = 
		diet_energy.divide_safely(energy_content, 0.0); // [kg/ind]

	// Finally: Convert the demand per individual [kg/ind]
	// to demand per area [kg/km²]
	return diet_mass * get_ind_per_km2();
}

double HerbivoreBase::get_kg_per_km2() const{
	return get_bodymass() * get_ind_per_km2();
}

double HerbivoreBase::get_max_fatmass() const{
	return get_potential_bodymass() * get_hft().bodyfat_max;
}

ForageEnergyContent HerbivoreBase::get_net_energy_content(					
		const Digestibility& digestibility)const{

	if (get_hft().net_energy_model == NE_DEFAULT){
		const GetNetEnergyContentDefault get_default(
				get_hft().digestion_type);
		return get_default(digestibility);

		// ADD NEW NET ENERGY MODELS HERE
		// in new if statements
	}else
		throw std::logic_error(
				"Fauna::HerbivoreBase::get_net_energy_content() "
				"Selected net energy model not implemented.");

}

double HerbivoreBase::get_potential_bodymass()const{
	// age of physical maturity in years
	const double maturity_age =
			((get_sex()==SEX_MALE) ? 
			get_hft().maturity_age_phys_male
			: get_hft().maturity_age_phys_female);

	if (get_age_years() >= maturity_age) 
		return get_bodymass_adult();
	else {
		// CALCULATE BODY MASS FOR PRE-ADULTS

		// lean weight at birth
		const double birth_leanmass = 
			get_hft().bodymass_birth * (1.0-get_hft().bodyfat_birth);

		// potential full mass at birth
		assert( 1.0-get_hft().bodyfat_max > 0.0 );
		const double birth_potmass =
			birth_leanmass / (1.0 - get_hft().bodyfat_max);

		// age fraction from birth to physical maturity
		assert(maturity_age > 0.0);
		const double fraction = 
			(double) get_age_days() / (maturity_age*365.0);

		// difference from birth to adult
		const double difference = 
			get_bodymass_adult() - birth_potmass;

		return birth_potmass + fraction * difference;
	} 
}

double HerbivoreBase::get_todays_expenditure()const{
	switch (get_hft().expenditure_model){
		case EM_TAYLOR_1981:
			return get_expenditure_taylor_1981(
					get_bodymass(),
					get_bodymass_adult());
			// ADD MORE MODELS HERE
		default:
			throw std::logic_error("Fauna::HerbivoreBase::get_todays_expenditure() "
					"Selected expenditure model is not implemented");
	};
}

double HerbivoreBase::get_todays_offspring_proportion()const{
	if (get_sex() == SEX_MALE ||
			get_age_years() < get_hft().maturity_age_sex) 
		return 0.0;

	// Several models use a BreedingSeason object, so we create one right
	// away.
	const BreedingSeason breeding_season(get_hft().breeding_season_start,
			get_hft().breeding_season_length);

	// choose the model
	if (get_hft().reproduction_model == RM_ILLIUS_2000){
		// create our model object
		const ReproductionIllius2000 illius_2000( breeding_season,
				get_hft().reproduction_max);
		// get today’s value
		return illius_2000.get_offspring_density(
				get_today(), 
				get_energy_budget().get_fatmass()/get_max_fatmass());
	} 
	else if (get_hft().reproduction_model == RM_CONST_MAX){
		const ReproductionConstMax const_max( breeding_season,
				get_hft().reproduction_max);
		return const_max.get_offspring_density(get_today());
		// ADD NEW MODELS HERE
		// in new if statements
	} 
	else
		throw std::logic_error(
				"Fauna::HerbivoreBase::get_todays_offspring_proportion() "
				"Reproduction model not implemented.");

}

void HerbivoreBase::simulate_day(const int day, double& offspring){
	if (day < 0 || day >= 365)
		throw std::invalid_argument("Fauna::HerbivoreBase::simulate_day() "
				"Argument \"day\" out of range.");

	/// - Set current day.
	today = day;
	
	/// - Increase age.
	age_days++;

	/// - Update maximum fat mass in \ref Fauna::FatmassEnergyBudget.
	get_energy_budget().set_max_fatmass(get_max_fatmass());

	/// - Add new output.
	get_todays_output().reset();
	get_todays_output().inddens   = get_ind_per_km2();
	get_todays_output().age_years = get_age_years();
	get_todays_output().massdens  = get_kg_per_km2();
	get_todays_output().bodyfat   = get_bodyfat();

	/// - Catabolize fat to compensate unmet energy needs.
	get_energy_budget().catabolize_fat();
	
	/// - Add energy needs for today.
	const double todays_expenditure = get_todays_expenditure();
	get_energy_budget().add_energy_needs(todays_expenditure);
	get_todays_output().expenditure = todays_expenditure;

	/// - Calculate offspring.
	offspring = get_todays_offspring_proportion() * get_ind_per_km2();

	/// - Apply mortality factor.
	apply_mortality_factors_today();
}

//============================================================
// HerbivoreIndividual
//============================================================

HerbivoreIndividual::HerbivoreIndividual(
		const int age_days,
		const double body_condition,
		const Hft* hft, 
		const Sex sex,
		const double area_km2
		):
	HerbivoreBase(age_days, body_condition, hft, sex),
	area_km2(area_km2), dead(false)
{
	if (area_km2 <= 0.0)
		throw std::invalid_argument("Fauna::HerbivoreIndividual::HerbivoreIndividual() "
				"area_km2 <=0.0");
}

HerbivoreIndividual::HerbivoreIndividual(
		const Hft* hft, 
		const Sex sex,
		const double area_km2
		):
	HerbivoreBase(hft, sex), area_km2(area_km2), dead(false)
{
	if (area_km2 <= 0.0)
		throw std::invalid_argument("Fauna::HerbivoreIndividual::HerbivoreIndividual() "
				"area_km2 <=0.0");
}

HerbivoreIndividual::HerbivoreIndividual(
		const HerbivoreIndividual& other):
	HerbivoreBase(other),
	area_km2(other.area_km2),
	dead(other.dead)
{ }

HerbivoreIndividual& HerbivoreIndividual::operator=(
		const HerbivoreIndividual& other){
	HerbivoreBase::operator=(other);
	area_km2 = other.area_km2;
	dead = other.dead;
	return *this;
}

void HerbivoreIndividual::apply_mortality(const double mortality){
	if (mortality < 0.0 || mortality > 1.0)
		throw std::invalid_argument("Fauna::HerbivoreCohort::apply_mortality() "
				"Parameter \"mortality\" out of range.");
	// Save some calculations for the simple cases of 0.0 and 1.0
	if (mortality==0.0)
		return;
	if (mortality==1.0){
		dead = true;
		return;
	}
	// Death is a stochastic event 
	if (get_random_fraction() < mortality)
		dead = true;
}

//============================================================
// HerbivoreCohort
//============================================================

HerbivoreCohort::HerbivoreCohort(
		const int age_days,
		const double body_condition,
		const Hft* hft, 
		const Sex sex,
		const double ind_per_km2
		):
	HerbivoreBase(age_days, body_condition, hft, sex),
	ind_per_km2(ind_per_km2)
{
	if (ind_per_km2 < 0.0)
		throw std::invalid_argument("Fauna::HerbivoreIndividual::HerbivoreIndividual() "
				"ind_per_km2 <0.0");
}

HerbivoreCohort::HerbivoreCohort(
		const Hft* hft, 
		const Sex sex,
		const double ind_per_km2
		):
	HerbivoreBase(hft, sex), // parent establishment constructor
	ind_per_km2(ind_per_km2)
{
	if (ind_per_km2 < 0.0)
		throw std::invalid_argument("Fauna::HerbivoreIndividual::HerbivoreIndividual() "
				"ind_per_km2 <0.0");
}

HerbivoreCohort::HerbivoreCohort(const HerbivoreCohort& other):
	HerbivoreBase(other),
	ind_per_km2(other.ind_per_km2)
{ }

HerbivoreCohort& HerbivoreCohort::operator=(const HerbivoreCohort& other){
	HerbivoreBase::operator=(other);
	ind_per_km2 = other.ind_per_km2;
	return *this;
}

void HerbivoreCohort::apply_mortality(const double mortality){
	if (mortality < 0.0 || mortality > 1.0)
		throw std::invalid_argument("Fauna::HerbivoreCohort::apply_mortality() "
				"Parameter \"mortality\" out of range.");
	// change of individual density [ind/km²]
	const double ind_change = - mortality * get_ind_per_km2();
	// apply the change and make sure that the density does not
	// drop below zero because of precision artefacts
	ind_per_km2 = max(0.0, ind_per_km2 + ind_change);
	assert(ind_per_km2 >= 0.0);
}

void HerbivoreCohort::merge(HerbivoreCohort& other){
	if (!is_same_age(other))
		throw std::invalid_argument("Fauna::HerbivoreCohort::merge() "
				"The other cohort is not the same age.");
	if (this->get_sex() != other.get_sex())
		throw std::invalid_argument("Fauna::HerbivoreCohort::merge() "
				"The other cohort is not the same sex.");
	if (this->get_hft() != other.get_hft())
		throw std::invalid_argument("Fauna::HerbivoreCohort::merge() "
				"The other cohort is not the same HFT.");

	// Merge energy budget
	this->get_energy_budget().merge(
			other.get_energy_budget(),
			this->get_ind_per_km2(),
			other.get_ind_per_km2());

	// sum up density
	this->ind_per_km2 += other.ind_per_km2;
	// Change density in other object
	other.ind_per_km2 = 0.0;
}

// REFERENCES
// Illius, A. W. & O'Connor, T. G. (2000). Resource heterogeneity and ungulate population dynamics. Oikos, 89, 283-294.
// Adrian Pachzelt, Anja Rammig, Steven Higgins & Thomas Hickler (2013). Coupling a physiological grazer population model with a generalized model for vegetation dynamics. Ecological Modelling, 263, 92 - 102.
