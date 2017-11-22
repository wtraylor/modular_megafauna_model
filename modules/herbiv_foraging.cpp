////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Foraging models and digestive contraints of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_foraging.h"

using namespace Fauna;
//============================================================
// Free Functions
//============================================================

ForageMass Fauna::get_max_intake_as_total_mass(
		const ForageFraction& mj_proportions,
		const ForageEnergyContent& mj_per_kg,
		const double kg_total)
{
	if (kg_total < 0.0)
		throw std::invalid_argument( "Fauna::get_max_intake_as_total_mass() "
				"Parameter `kg_total` is negative.");
	if (kg_total == 0.0)
		return ForageMass(0.0);

	// The energy-wise proportions of the diet are given by 
	// `mj_proportions`. Now we need to obtain the mass-wise 
	// composition.
	const ForageFraction mass_proportions = 
		convert_mj_to_kg_proportionally(mj_per_kg, mj_proportions);

	// Multiply the maximum foraging with the mass-wise proportions.
	// To get the maximum intake for each individual forage type.
	return kg_total * (mass_proportions / mass_proportions.sum());
}

//============================================================
// GetForageDemands
//============================================================

GetForageDemands::GetForageDemands(const Hft* hft, const Sex sex) :
	hft(hft), 
	sex(sex),
	today(-1) // indicate that day has not been initialized.
{ 
	if (hft == NULL)
		throw std::invalid_argument(
				"Fauna::GetForageDemands::GetForageDemands() "
				"Parameter `hft` is NULL.");
}

ForageFraction GetForageDemands::get_diet_composition()const
{
	// Initialize result with zero and let the algorithms set their
	// particular forage types.
	ForageFraction result(0.0);

	if (get_hft().diet_composer == DC_PURE_GRAZER) {
		result.set(FT_GRASS, 1.0); // put all into grass
		// ** Add new diet composer algorithms here in else-if statements. **
	} else {
		throw std::logic_error(
				"Fauna::GetForageDemands::get_diet_composition() "
				"The selected algorithm for diet composition "
				"(Hft::diet_composer) is not implemented."); 
	}

	// Check the result, but leave some rounding error tolerance.
	assert(result.sum() >= 0.999 && result.sum() <= 1.001); 
	return result;
}

ForageMass GetForageDemands::get_max_digestion()const
{
	assert(today > -1); // check that init_today() has been called

	if (get_hft().digestive_limit == DL_NONE) {
		return ForageMass(100000); 
	}

	
	if (get_hft().digestive_limit == DL_BODYMASS_FRACTION) {
		return get_max_intake_as_total_mass(
				diet_composition,
				energy_content,
				bodymass * get_hft().digestion_bodymass_fraction);
	}


	if (get_hft().digestive_limit == DL_ILLIUS_GORDON_1992) {
		// Check that we are only handling grass here. This should be
		// already checked in Hft::is_valid().
		assert(diet_composer == DC_PURE_GRAZER);

		// function object
		const GetDigestiveLimitIlliusGordon1992 get_digestive_limit(
				get_bodymass_adult(), get_hft().digestion_type);

		// calculate the digestive limit [MJ/ind/day]
		const ForageEnergy limit_mj = 
			get_digestive_limit(bodymass, digestibility);

		// Convert energy to kg dry matter
		// kg * MJ/kg = kg; where zero values remain zero values even
		// on division by zero.
		const ForageMass limit_kg = limit_mj.divide_safely(
				energy_content, 0.0);

		// Set the maximum foraging limit [kgDM/ind/day]
		return limit_kg;
	} 
	// ** add new digestive constraints in new else-if statements here **
	else {
		throw std::logic_error(
				"Fauna::GetForageDemands::get_max_digestion() "
				"The value for `Hft::digestive_limit` is not implemented.");
	}

}

ForageMass GetForageDemands::get_max_foraging()const
{
	assert(today > -1); // check that init_today() has been called

	// set the maximum, and then let the foraging limit algorithms
	// reduce the maximum by using ForageMass::min()
	ForageMass result(10000); // [kgDM/ind/day]
	// (Note that using DBL_MAX here does not work because converting it to
	//  energy may result in INFINITY values.)

	// Go through all forage intake limits
	std::set<ForagingLimit>::const_iterator itr;
	for (itr = get_hft().foraging_limits.begin();
			itr != get_hft().foraging_limits.end();
			itr++) 
	{ 
		if (*itr == FL_ILLIUS_OCONNOR_2000) {
			// Check that we are only handling grass here. This should be
			// already checked in Hft::is_valid().
			assert(diet_composer == DC_PURE_GRAZER);

			// create function object for maximum intake
			const GetDigestiveLimitIlliusGordon1992 get_digestive_limit(
					get_bodymass_adult(), get_hft().digestion_type);

			// Create functional response with digestive limit as maximum.
			const HalfMaxIntake half_max(
					get_hft().half_max_intake_density*1000.0, // gDM/m² to kgDM/km²
					get_digestive_limit(bodymass, digestibility)[FT_GRASS]);

			// Like Pachzelt (2013), we use the whole-habitat grass density,
			// not the ‘sward density’.
			const double grass_limit_mj = half_max.get_intake_rate(
					available_forage.grass.get_mass()); // [MJ/day]

			double grass_limit_kg;
			if (energy_content[FT_GRASS] > 0.0)
				grass_limit_kg = grass_limit_mj / energy_content[FT_GRASS];
			else
				grass_limit_kg = 0.0; // no energy ⇒ no feeding

			// The Illius & O’Connor (2000) model applies only to grass, and
			// hence we only constrain the grass part of `result`.
			result.set(FT_GRASS, min(result[FT_GRASS], grass_limit_kg));
		} else
			// ADD MORE LIMITS HERE IN NEW ELSE-IF STATEMENTS
			throw std::logic_error(
					"Fauna::GetForageDemands::get_max_foraging() "
					"One of the selected foraging limits is not implemented.");
	}
	return result;
}

void GetForageDemands::init_today(
		const int day,
		const HabitatForage& _available_forage,
		const ForageEnergyContent& _energy_content,
		const double _bodymass)
{
	if (_bodymass > get_bodymass_adult())
		throw std::invalid_argument("Fauna::GetForageDemands::init_today() "
				"Parameter \"bodymass\" is greater than HFT adult body mass.");
	if (_bodymass <= 0.0)
		throw std::invalid_argument("Fauna::GetForageDemands::init_today() "
				"Parameter \"bodymass\" is zero or negative.");
	if (day < 0)
		throw std::invalid_argument("Fauna::GetForageDemands::init_today() "
				"Parameter \"day\" is negative");
	if (day >= 365)
		throw std::invalid_argument("Fauna::GetForageDemands::init_today() "
				"Parameter \"day\" is greater than 364.");

	// init today’s variables
	available_forage = _available_forage;
	bodymass         = _bodymass;
	digestibility    = _available_forage.get_digestibility();
	energy_content   = _energy_content;
	energy_needs     = 0.0;
	today            = day;

	// Diet composition
	diet_composition = get_diet_composition();

	// Initialize with extreme number and then reduce it to actual maxima.
	max_intake = ForageMass(10000);

	// Reduce maximum intake by foraging limits.
	max_intake.min(get_max_foraging());

	// Reduce maximum intake by digestive limits.
	max_intake.min(get_max_digestion());
}

int GetForageDemands::get_today()const{
	if (today == -1) // initial value from constructor
		throw std::logic_error("Fauna::GetForageDemands::get_today() "
				"Current day not yet initialized. Has `init_today()` "
				"been called first?");
	assert(today>=0 && today<365);
	return today;
}

ForageMass GetForageDemands::operator()(const double _energy_needs)
{
	if (today == -1)
		throw std::logic_error("Fauna::GetForageDemands::operator()() "
				"This day has not yet been initialized. The function "
				"init_today() must be called before operator()().");
	if (_energy_needs < 0.0)
		throw std::invalid_argument("Fauna::GetForageDemands::operator()() "
				"Parameter `energy_needs` is negative.");

	energy_needs = _energy_needs;

	// No hunger ⇒ no demands
	if (energy_needs == 0.0)
		return ForageMass(0.0);

	//------------------------------------------------------------------
	// CONVERT MASS TO ENERGY

	// The maximum intake of each forage type as net energy [MJ/ind]
	const ForageEnergy max_energy_intake = max_intake * energy_content;

	// The total maximum forage intake over all forage types [kgDM/km²].
	const double max_energy_intake_sum = max_energy_intake.sum();

	// No eating capacity ⇒ no demands
	if (max_energy_intake_sum == 0.0)
		return ForageMass(0.0);

	//------------------------------------------------------------------
	// COMPOSE ENERGY FRACTIONS OF DIET

	// Find the forage type that is limiting the total intake the most:
	// This is the forage type where the preferred fraction 
	// (↦ diet_composition) is furthest away from the fraction in the
	// possible intake (↦ max_energy_intake).

	// The fraction for a forage type of possible intake to de
	double min_fraction = 1.0;

	// Iterate through all forage types.
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end();
			ft++)
	{
		if (diet_composition[*ft] > 0)
			min_fraction = min(min_fraction, 
					(diet_composition[*ft] * max_energy_intake_sum) /
					max_energy_intake[*ft]);
	}

	// The maximum energy intake with the forage types composed in
	// the same fraction as in `diet_composition` [MJ/ind].
	const ForageMass max_energy_intake_comp = 
		max_energy_intake * (min_fraction * diet_composition);

	// Desired forage types cannot be eaten ⇒ no demands
	if (max_energy_intake_comp.sum() == 0.0)
		return ForageMass(0.0);

	//------------------------------------------------------------------
	// REDUCE TO ACTUAL ENERGY NEEDS
	
	// The fraction by what we need to reduce the energy intake to meet
	// the actual needs.
	const double energy_reduction = min(1.0,
			energy_needs / max_energy_intake_comp.sum());

	// This is our finally demanded energy [MJ/ind].
	const ForageMass actual_energy_intake =
		max_energy_intake_comp * energy_reduction;

	//------------------------------------------------------------------
	// CONVERT BACK FROM ENERGY TO MASS

	// convert MJ/ind to kgDM/ind
	return actual_energy_intake.divide_safely(energy_content, 0.0);
}

//============================================================
// HalfMaxIntake
//============================================================

HalfMaxIntake::HalfMaxIntake(
					const double half_max_density,
					const double max_intake):
	half_max_density(half_max_density),
	max_intake(max_intake)
{
	if (!(half_max_density > 0.0))
		throw std::invalid_argument("Fauna::HalfMaxIntake::HalfMaxIntake() "
				"Parameter `half_max_density` is not a positive number.");
	if (!(max_intake > 0.0))
		throw std::invalid_argument("Fauna::HalfMaxIntake::HalfMaxIntake() "
				"Parameter `max_intake` is not a positive number.");
}

double HalfMaxIntake::get_intake_rate(const double density)const{
	if (!(density >= 0.0))
		throw std::invalid_argument("Fauna::HalfMaxIntake::get_intake_rate() "
				"Parameter `density` must be zero or a positive number.");
	return max_intake * density / (half_max_density + density);
}

//============================================================
// GetDigestiveLimitIlliusGordon1992
//============================================================

GetDigestiveLimitIlliusGordon1992::GetDigestiveLimitIlliusGordon1992( 
		const double bodymass_adult,
		const DigestionType digestion_type):
	bodymass_adult(bodymass_adult),
	digestion_type(digestion_type)
{
	if (bodymass_adult <= 0.0)
		throw std::invalid_argument(
				"Fauna::GetDigestiveLimitIlliusGordon1992::GetDigestiveLimitIlliusGordon1992() "
				"Parameter `bodymass_adult` <= zero.");
}

const ForageEnergy GetDigestiveLimitIlliusGordon1992::operator()(
		const double bodymass,
		const Digestibility& digestibility)const{
	if (bodymass <= 0.0)
		throw std::invalid_argument("Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
				"Parameter `bodymass` <= zero.");
	if (bodymass > bodymass_adult)
		throw std::invalid_argument("Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
				"bodymass > bodymass_adult");

	typedef ForageValues<POSITIVE_AND_ZERO> ParameterConstant;

	ParameterConstant i, j, k;

	// Initialize constants
	bool initialized = false;
	if (!initialized){
		if (FORAGE_TYPES.size() > 1) // change this if adding new forage types
			throw std::logic_error(
					"Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
					"Not all forage types are implemented.");

		if (digestion_type == DT_RUMINANT){
			i.set(FT_GRASS, 0.034);
			j.set(FT_GRASS, 3.565);
			k.set(FT_GRASS, 0.077);
			// ADD NEW FORAGE TYPES HERE
		} else if (digestion_type == DT_HINDGUT){
			i.set(FT_GRASS, 0.108);
			j.set(FT_GRASS, 3.284);
			k.set(FT_GRASS, 0.080);
			// ADD NEW FORAGE TYPES HERE
		} else throw std::logic_error(
					"Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
					"Digestion type not implemented.");
		initialized = true;
	}

	ForageEnergy result;

	// rename variables to match formula
	const Digestibility& d = digestibility;
	const double& M_ad = bodymass_adult; // [kg]
	const double& M = bodymass; // [kg]
	const double u_g = pow( M/M_ad, .75);

	// Because of power calculations we cannot use the
	// arithmetic operators of ForageValues<>, but need to 
	// iterate over all forage types.
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end(); ft++)
	{
		const ForageType f = *ft;
		if (digestibility[f] > 0.0)
			result.set(f,
					i[f] * exp(j[f]*d[f]) * pow(M_ad, k[f]*exp(d[f]) + .73) * u_g);
		else
			result.set(f, 0.0); // zero digestibility -> zero energy
	}

	return result;
}
