///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Unit tests for megafauna herbivores.
/// \ingroup group_herbivory 
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 05-21-2017
///////////////////////////////////////////////////////////////////////////////////////

#include "catch.hpp" 
#include "herbiv_energetics.h"
#include "herbiv_foraging.h" 
#include "herbiv_forageclasses.h" 
#include "herbiv_forageenergy.h"
#include "herbiv_framework.h"
#include "herbiv_herbivore.h"
#include "herbiv_hft.h"
#include "herbiv_mortality.h"
#include "herbiv_outputclasses.h"
#include "herbiv_parameters.h"
#include "herbiv_paramreader.h"
#include "herbiv_reproduction.h"
#include "herbiv_testhabitat.h" 
#include <memory> // for std::auto_ptr

using namespace Fauna;
using namespace FaunaOut;
using namespace FaunaSim;

namespace {
	/// A dummy habitat that does nothing
	class DummyHabitat: public Habitat{
		public:
			/// Constructor with empty populations
			DummyHabitat(): 
				Habitat(std::auto_ptr<HftPopulationsMap>(new HftPopulationsMap())) {}
			virtual HabitatForage get_available_forage() const { return HabitatForage(); }
			int get_day_public()const{return get_day();}
	};

	/// A dummy herbivore that does nothing
	class DummyHerbivore: public HerbivoreInterface{
		public:
			DummyHerbivore(const Hft* hft, const double ind_per_km2,
					const double bodymass=30.0):
				hft(hft), ind_per_km2(ind_per_km2), bodymass(bodymass){}
			
			virtual void eat(const ForageType forage_type,
					const double kg_per_km2,
					const double digestibility){}

			virtual double get_bodymass() const{return 1.0;}

			virtual ForageMass get_forage_demands(
					const HabitatForage& available_forage)const{
				return demand;
			}

			virtual const Hft& get_hft()const{return *hft;}

			virtual double get_ind_per_km2()const{return ind_per_km2;}
			virtual double get_kg_per_km2()const{
				return bodymass*ind_per_km2;
			}

			virtual void simulate_day(const int day,
					double& offspring){offspring=0.0;}
		public:
			const ForageMass& get_demand()const{return demand;}
			void set_demand(const ForageMass& d){demand = d;}
		private:
			const Hft* hft;
			const double ind_per_km2; 
			const double bodymass;
			ForageMass demand;
	};

	/// A population of dummy herbivores
	class DummyPopulation: public PopulationInterface{
		private:
			const Hft* hft;
			std::vector<DummyHerbivore> vec;
		public:
			DummyPopulation(const Hft* hft):hft(hft){}

			/// creates one new herbivore object
			virtual void create_offspring(const double ind_per_km2){
				vec.push_back(DummyHerbivore(hft, ind_per_km2));
			}

			/// creates one new herbivore object
			virtual void establish(){
				vec.push_back(DummyHerbivore(hft,
							hft->establishment_density));
			}

			virtual const Hft& get_hft()const{return *hft;}

			virtual std::vector<const HerbivoreInterface*> get_list()const{
				std::vector<const HerbivoreInterface*> res;
				for (int i=0; i<vec.size(); i++)
					res.push_back(&vec[i]);
				return res;
			}
			virtual std::vector<HerbivoreInterface*> get_list(){ 
				std::vector<HerbivoreInterface*> res;
				for (int i=0; i<vec.size(); i++)
					res.push_back(&vec[i]);
				return res;
			}
	};

	/// Dummy class to test \ref HerbivoreBase
	class HerbivoreBaseDummy: public HerbivoreBase{
		public:
			virtual double get_ind_per_km2() const{ return ind_per_km2; }

			/// Establishment Constructor
			HerbivoreBaseDummy(
					const int age_days,
					const double body_condition,
					const Hft* hft, 
					const Sex sex):
				HerbivoreBase(age_days, body_condition, hft, sex),
				ind_per_km2(1.0){}

			/// Birth Constructor
			HerbivoreBaseDummy( const Hft* hft, const Sex sex):
				HerbivoreBase(hft, sex), ind_per_km2(1.0){}

			HerbivoreBaseDummy(const HerbivoreBaseDummy& other):
				HerbivoreBase(other), ind_per_km2(other.ind_per_km2){}
			HerbivoreBaseDummy& operator=(const HerbivoreBaseDummy& rhs){
				HerbivoreBase::operator=(rhs);
				ind_per_km2 = rhs.ind_per_km2;
				return *this;
			}
		protected:
			virtual void apply_mortality(const double mortality){
				ind_per_km2 *= mortality;
			}
		private:
			double ind_per_km2;
	};

	/// Create a simple, valid HftList
	HftList create_hfts(const int count, const Parameters& params){
		HftList hftlist;
		for (int i=0; i<count; i++){
			Hft hft;
			hft.is_included = true;
			// construct name for HFT
			std::ostringstream stream;
			stream << "hft"<<i;
			hft.name = stream.str();

			std::string msg;
			if (!hft.is_valid(params, msg))
				FAIL("create_hfts(): HFT is not valid:\n"<<msg);

			hftlist.insert(hft);
		}
		REQUIRE( hftlist.size() == count );
		return hftlist;
	}

	/// Sum of population individual density [ind/km²]
	double get_total_pop_density(const PopulationInterface& pop){
		double sum;
		ConstHerbivoreVector vec = pop.get_list();
		for (ConstHerbivoreVector::const_iterator itr = vec.begin();
				itr != vec.end(); itr++)
			sum += (*itr)->get_ind_per_km2();
		return sum;
	}

	/// \brief Check if the lengths of the modifiable and the 
	/// read-only population vectors match.
	bool population_lists_match(PopulationInterface& pop){
		// FIRST the read-only -> no chance for the population
		// object to change the list.
		ConstHerbivoreVector readonly = 
			((const PopulationInterface&) pop).get_list();
		HerbivoreVector modifiable = pop.get_list();
		return modifiable.size() == readonly.size();
	}

} // anonymous namespace 


// test cases in alphabetical order, please

TEST_CASE("Fauna::CohortPopulation", "") {
	// prepare parameters
	Parameters params;
	REQUIRE( params.is_valid() );

	// prepare HFT
	Hft hft = create_hfts(1, params)[0];
	hft.establishment_density = 10.0; // [ind/km²]
	hft.mortality_factors.clear(); // immortal herbivores
	REQUIRE( hft.is_valid(params) );

	// prepare creating object
	CreateHerbivoreCohort create_cohort(&hft, &params);

	// check exceptions
	CHECK_THROWS( CohortPopulation(create_cohort, -1.0) );
	
	// create cohort population
	const double THRESHOLD = 0.1;
	CohortPopulation pop(create_cohort, THRESHOLD);
	REQUIRE( pop.get_list().empty() );
	REQUIRE( population_lists_match( pop ) );
	REQUIRE( pop.get_hft() == hft );

	CHECK_THROWS( pop.create_offspring(-1.0) );

	SECTION("Establishment"){
		REQUIRE( pop.get_list().empty() ); // empty before
		pop.establish();
		REQUIRE( !pop.get_list().empty() ); // filled afterwards
		REQUIRE( population_lists_match(pop) );
		
		// There should be only one age class with male and female
		REQUIRE( pop.get_list().size() == 2 ); 

		// Does the total density match?
		REQUIRE( get_total_pop_density(pop) 
				== Approx(hft.establishment_density) );

		SECTION("Removal of dead cohorts with mortality"){
			// we will kill all herbivores in the list with a copy 
			// assignment trick

			// Let them die ...
			HerbivoreVector vec = pop.get_list();
			// call birth constructor with zero density
			HerbivoreCohort dead(&hft, SEX_FEMALE, 0.0);
			for (HerbivoreVector::iterator itr=vec.begin();
					itr!=vec.end(); itr++) {
				HerbivoreInterface* pint = *itr;
				HerbivoreCohort* pcohort = (HerbivoreCohort*) pint;
				pcohort->operator=(dead);
				REQUIRE( pcohort->get_ind_per_km2() == 0.0 );
			}
			// now they should be all dead
			CHECK( population_lists_match(pop) );
			CHECK( pop.get_list().size() == 0 );
		}
	}

	SECTION("Offspring"){
		const double DENS = 10.0; // [ind/km²]
		pop.create_offspring(DENS); 
		// There should be only one age class with male and female
		REQUIRE( pop.get_list().size() == 2 ); 
		CHECK( population_lists_match(pop) );
		// Does the total density match?
		REQUIRE( get_total_pop_density(pop) == Approx(DENS) );
		
		// add more offspring
		pop.create_offspring(DENS);
		REQUIRE( pop.get_list().size() == 2 ); 
		CHECK( population_lists_match(pop) );
		REQUIRE( get_total_pop_density(pop) == Approx(2.0*DENS) );

		// let the herbivores age (they are immortal)
		for (int i=0; i<365; i++) {
			HerbivoreVector::iterator itr;
			HerbivoreVector list = pop.get_list();
			double offspring_dump; // ignored
			for (itr=list.begin(); itr!=list.end(); itr++)
				(*itr)->simulate_day(i, offspring_dump);
		}
		// now they should have grown older, and if we add more
		// offspring, there should be new age classes
		pop.create_offspring(DENS);
		CHECK( pop.get_list().size() == 4 );
		CHECK( population_lists_match(pop) );
		REQUIRE( get_total_pop_density(pop) == Approx(3.0*DENS) );
	}

	SECTION("Removal of dead cohorts at establishment"){
		// establish in very low density
		hft.establishment_density = THRESHOLD / 2.0;
		pop.establish();
		CHECK( population_lists_match(pop) );
		CHECK( pop.get_list().empty() );
	}
}

TEST_CASE("Fauna::DistributeForageEqually", "") {
	// PREPARE POPULATIONS
	const int HFT_COUNT = 5;
	const int IND_PER_HFT = 10;
	const int IND_TOTAL = HFT_COUNT*IND_PER_HFT; // dummy herbivores total
	const HftList hftlist = create_hfts(HFT_COUNT, Parameters());
	HftPopulationsMap popmap;
	for (HftList::const_iterator itr=hftlist.begin(); 
			itr!=hftlist.end(); itr++){
		// create new population
		std::auto_ptr<PopulationInterface> new_pop(
				new DummyPopulation(&*itr));
		// fill with herbivores
		for (int i=1; i<=IND_PER_HFT; i++)
			new_pop->create_offspring(1.0);
		// add newly created dummy population
		popmap.add(new_pop);
		//
	}

	// CREATE DEMAND MAP
	ForageDistribution demands;
	// loop through all herbivores and fill the distribution
	// object with pointer to herbivore and zero demands (to be
	// filled later)
	for (HftPopulationsMap::iterator itr_pop=popmap.begin();
			itr_pop != popmap.end(); itr_pop++){
		PopulationInterface& pop = **(itr_pop);
		HerbivoreVector vec = pop.get_list();
		// loop through herbivores in the population
		for (HerbivoreVector::iterator itr_her=vec.begin();
				itr_her != vec.end(); itr_her++){
			HerbivoreInterface* pherbivore = *itr_her;
			// create with zero demands
			static const ForageMass ZERO_DEMAND;
			demands[pherbivore] = ZERO_DEMAND;
		}
	}

	// PREPARE AVAILABLE FORAGE
	HabitatForage available;
	const double MASS_GRASS = 1.0; // [kg/km²]
	available.grass.set_mass(MASS_GRASS);
	// add new forage types here

	// our distrubition functor
	DistributeForageEqually distribute;

	SECTION("less demanded than available"){
		// SET DEMANDS
		ForageMass IND_DEMAND;
		IND_DEMAND.set_grass(MASS_GRASS/(IND_TOTAL+1));
		// add new forage types here
		for (ForageDistribution::iterator itr = demands.begin();
				itr != demands.end(); itr++) {
			DummyHerbivore* pherbivore = (DummyHerbivore*) itr->first;
			pherbivore->set_demand(IND_DEMAND);
			itr->second = IND_DEMAND;
		}
		
		// DISTRIBUTE
		distribute(available, demands);

		// CHECK
		// there must not be any change
		ForageMass sum;
		for (ForageDistribution::iterator itr = demands.begin();
				itr != demands.end(); itr++) {
			DummyHerbivore* pherbivore = (DummyHerbivore*) itr->first;
			CHECK( itr->second == pherbivore->get_demand() );
			sum += pherbivore->get_demand();
		}
		CHECK( sum <= available.get_mass() );
	}

	SECTION("More demanded than available"){
		// SET DEMANDS
		ForageMass total_demand;
		int i=0; // a counter to vary the demands a little
		for (ForageDistribution::iterator itr = demands.begin();
				itr != demands.end(); itr++) {
			DummyHerbivore* pherbivore = (DummyHerbivore*) itr->first;
			// define a demand that is in total somewhat higher than
			// what’s available and varies among the herbivores
			ForageMass ind_demand;
			ind_demand.set_grass(MASS_GRASS / IND_TOTAL 
					* (1.0 + (i%5)/5)); // just arbitrary
			pherbivore->set_demand(ind_demand);
			itr->second = ind_demand;
			total_demand += ind_demand;
			i++;
		}
		
		// DISTRIBUTE
		distribute(available, demands);

		// CHECK
		// each herbivore must have approximatly its equal share
		ForageMass sum;
		for (ForageDistribution::iterator itr = demands.begin();
				itr != demands.end(); itr++) {
			DummyHerbivore* pherbivore = (DummyHerbivore*) itr->first;
			CHECK( itr->second != pherbivore->get_demand() );
			sum += itr->second;
			// check each forage type individually because Approx()
			// is not defined for ForageMass
			{ // GRASS
				const double ind_portion = itr->second.get_grass();
				const double ind_demand = pherbivore->get_demand().get_grass();
				const double tot_portion = available.grass.get_mass();
				const double tot_demand = total_demand.get_grass();
				CHECK( ind_portion/tot_portion 
						== Approx(ind_demand/tot_demand).epsilon(0.05) );
			}
			// add more forage types here
		}
		// The sum may never exceed available forage
		CHECK( sum.get_grass() <= available.grass.get_mass() );
		CHECK( sum <= available.get_mass() );
	}
}

TEST_CASE("Fauna::Dummies", "") {
	Hft hft1;
	hft1.name="hft1";
	DummyHerbivore dummy1 = DummyHerbivore(&hft1, 1.0);
	DummyHerbivore dummy2 = DummyHerbivore(&hft1, 0.0);

	DummyPopulation pop = DummyPopulation(&hft1);
	pop.create_offspring(1.0);
	REQUIRE(pop.get_list().size() == 1);
}

TEST_CASE("Fauna::FatmassEnergyBudget", "") {
	CHECK_THROWS( FatmassEnergyBudget(-1.0, 1.0) );
	CHECK_THROWS( FatmassEnergyBudget(0.0, 0.0) );
	CHECK_THROWS( FatmassEnergyBudget(0.0, -1.0) );
	CHECK_THROWS( FatmassEnergyBudget(1.1, 1.0) );


	const double INIT_FATMASS = 1.0; // initial fat mass
	const double MAX_FATMASS = 2.0;  // maximim fat mass
	FatmassEnergyBudget budget(INIT_FATMASS, MAX_FATMASS);

	// Initialization
	REQUIRE( budget.get_fatmass() == INIT_FATMASS );
	REQUIRE( budget.get_energy_needs() == 0.0 );

	// exceptions
	CHECK_THROWS( budget.metabolize_energy(-1.0) );
	CHECK_THROWS( budget.metabolize_energy(1000000.0) );
	CHECK_THROWS( budget.add_energy_needs(-1.0) );
	CHECK_THROWS( budget.set_max_fatmass(INIT_FATMASS/2.0) );
	CHECK_THROWS( budget.set_max_fatmass(-1.0) );

	SECTION("Anabolism"){
		budget.metabolize_energy(10.0);
		CHECK( budget.get_fatmass() > INIT_FATMASS );
	}

	const double ENERGY = 10.0;

	SECTION("Catabolism"){
		budget.add_energy_needs(ENERGY);
		budget.catabolize_fat();
		CHECK( budget.get_fatmass() < INIT_FATMASS );
	}

	SECTION("Metabolism"){
		budget.add_energy_needs(ENERGY);
		REQUIRE( budget.get_energy_needs() == ENERGY );
		budget.metabolize_energy(ENERGY);
		CHECK( budget.get_energy_needs() == Approx(0.0) );
	}

	SECTION("Metabolism and Anabolism"){
		budget.add_energy_needs(ENERGY/2.0);
		REQUIRE( budget.get_energy_needs() == Approx(ENERGY/2.0) );
		budget.metabolize_energy(ENERGY);
		CHECK( budget.get_energy_needs() == 0.0 );
		CHECK( budget.get_fatmass() > INIT_FATMASS ); 
		CHECK( budget.get_fatmass() < MAX_FATMASS );
	}

	SECTION("Merge"){
		budget.add_energy_needs(ENERGY);
		const double OTHER_FATMASS     = 3.0;
		const double OTHER_MAX_FATMASS = 4.0;
		const double OTHER_ENERGY      = 13.0;
		FatmassEnergyBudget other(OTHER_FATMASS, OTHER_MAX_FATMASS);
		other.add_energy_needs(OTHER_ENERGY);

		SECTION("Merge with equal weight"){
			budget.merge(other, 1.0, 1.0);
			CHECK( budget.get_energy_needs() == 
					Approx((ENERGY+OTHER_ENERGY)/2.0) );
			CHECK( budget.get_max_fatmass() == 
					Approx((MAX_FATMASS+OTHER_MAX_FATMASS)/2.0) );
			CHECK( budget.get_fatmass() == 
					Approx((INIT_FATMASS+OTHER_FATMASS)/2.0) );
		}

		SECTION("Merge with different weight"){
			const double W1 = 0.4;
			const double W2 = 1.2;
			budget.merge(other, W1, W2);
			CHECK( budget.get_energy_needs() == 
					Approx((ENERGY*W1+OTHER_ENERGY*W2)/(W1+W2)) );
			CHECK( budget.get_max_fatmass() == 
					Approx((MAX_FATMASS*W1+OTHER_MAX_FATMASS*W2)/(W1+W2)) );
			CHECK( budget.get_fatmass() == 
					Approx((INIT_FATMASS*W1+OTHER_FATMASS*W2)/(W1+W2)) );
		}

	}
}

TEST_CASE("Fauna::ForageMass", "") {
	ForageMass fm;

	// Initialization
	REQUIRE( Approx(fm.sum()) == 0.0);

	// exceptions
	CHECK_THROWS( fm.set_grass(-1.0) );
	CHECK_THROWS( fm.set_grass(NAN) );

	// test other forage types here, too


	const double GRASS = 2.0;
	fm.set_grass(GRASS);
	REQUIRE( Approx(fm.sum()) == fm.get_grass() );

	SECTION( "assignment" ){
		fm = 2.0;
		CHECK(fm.get_grass() == 2.0);
	}
	SECTION( "addition" ){
		const double A = 12.0;
		ForageMass fm2;
		fm2 = A;

		CHECK((fm+fm2).get_grass() == GRASS+A); 

		fm += fm2;
		CHECK(fm.get_grass() == GRASS+A);
	}
	SECTION( "multiplication" ){
		const double F = 1.5;

		CHECK((fm*F).get_grass() == GRASS*F);

		fm *= F;
		CHECK(fm.get_grass() == GRASS*F);
	}
	SECTION( "division" ){
		CHECK_THROWS(fm/0.0);
		CHECK_THROWS(fm/=0.0);

		const double D = 1.5;

		CHECK((fm/D).get_grass() == GRASS/D);

		fm /= D;
		CHECK(fm.get_grass() == GRASS/D);
	}
}

TEST_CASE("Fauna::GetBackgroundMortality", "") {
	CHECK_THROWS( GetBackgroundMortality(0.0, -1.0) );
	CHECK_THROWS( GetBackgroundMortality(1.0, 0.0) );
	CHECK_THROWS( GetBackgroundMortality(-1.0, -1.0) );
	CHECK_THROWS( GetBackgroundMortality(-1.0, 0.0) );
	CHECK_THROWS( GetBackgroundMortality(0.0, 1.1) );

	const GetBackgroundMortality get_zero(0.0, 0.0);
	CHECK( get_zero(0) == 0.0 );
	CHECK( get_zero(1*365) == 0.0 );
	CHECK( get_zero(4*365) == 0.0 );

	const double JUV = 0.3;
	const double ADULT = 0.1;
	const GetBackgroundMortality get_mort(JUV, ADULT);
	CHECK_THROWS( get_mort(-1) );
	REQUIRE( get_mort(1) > 0.0 );

	// Check that the daily mortality matches the annual one.
	double surviving_juveniles = 1.0;
	for (int d=0; d<365; d++)
		surviving_juveniles *= (1.0 - get_mort(d));
	CHECK( surviving_juveniles == Approx(1.0-JUV) );
	
	// Check that the daily mortality matches the annual one.
	double surviving_adults = 1.0;
	for (int d=365; d<2*365; d++)
		surviving_adults *= (1.0 - get_mort(d));
	CHECK( surviving_adults == Approx(1.0-ADULT) );
}

TEST_CASE("Fauna::GetNetEnergyContentDefault", "") {
	GetNetEnergyContentDefault ne_ruminant(DT_RUMINANT);
	GetNetEnergyContentDefault ne_hindgut(DT_HINDGUT);

	// exceptions
	CHECK_THROWS(ne_ruminant(FT_GRASS, -1.0));
	CHECK_THROWS(ne_ruminant(FT_GRASS, 2.0));

	CHECK(ne_ruminant(FT_INEDIBLE, 0.5) == 0.0);

	// higher digestibility ==> more energy
	CHECK(ne_ruminant(FT_GRASS, 0.5) > ne_ruminant(FT_GRASS, 0.3));
	CHECK(ne_hindgut(FT_GRASS, 0.5) > ne_hindgut(FT_GRASS, 0.3));

	// hindguts have lower efficiency
	CHECK(ne_hindgut(FT_GRASS, 0.5) < ne_ruminant(FT_GRASS, 0.5));
}

TEST_CASE("Fauna::get_random_fraction", "") {
	for (int i=0; i<100; i++){
		const double r = get_random_fraction();
		CHECK( r <= 1.0 );
		CHECK( r >= 0.0 );
	}
}

TEST_CASE("Fauna::GetSimpleLifespanMortality", "") {
	CHECK_THROWS( GetSimpleLifespanMortality(-1) );
	CHECK_THROWS( GetSimpleLifespanMortality(0) );
	const int LIFESPAN = 20;
	const GetSimpleLifespanMortality get_mort(LIFESPAN);
	CHECK_THROWS( get_mort(-1) );
	// some arbitrary numbers
	CHECK( get_mort(0)                == 0.0 );
	CHECK( get_mort(40)               == 0.0 );
	CHECK( get_mort(3*365)            == 0.0 );
	CHECK( get_mort(LIFESPAN*365 - 1) == 0.0 );
	CHECK( get_mort(LIFESPAN*365)     == 1.0 );
	CHECK( get_mort(LIFESPAN*365+10)  == 1.0 );
	CHECK( get_mort((LIFESPAN+1)*365) == 1.0 );
}

TEST_CASE("Fauna::GetStarvationMortalityIllius2000", "") {
	CHECK_THROWS( GetStarvationMortalityIllius2000(-0.1) );
	CHECK_THROWS( GetStarvationMortalityIllius2000(1.1) );

	SECTION("default standard deviation"){
		const GetStarvationMortalityIllius2000 get_mort;
		CHECK_THROWS( get_mort(-1.0) );
		CHECK_THROWS( get_mort(1.1) );

		// With full fat reserves there shouldn’t be any considerable
		// mortality
		CHECK( get_mort(1.0) == Approx(0.0) );

		// Mortality increases with lower body condition.
		CHECK( get_mort(0.1) > get_mort(0.2) );

		// Because of the symmetry of the normal distribution,
		// only half of the population actually falls below zero
		// fat reserves if the average is zero.
		CHECK( get_mort(0.0) == Approx(0.5) );
	}

	SECTION("compare standard deviations"){
		const GetStarvationMortalityIllius2000 get_mort1(0.1);
		const GetStarvationMortalityIllius2000 get_mort2(0.2);
		CHECK( get_mort1(0.1) < get_mort2(0.1) );
	}
}

TEST_CASE("Fauna::GetStarvationMortalityThreshold", "") {
	CHECK_THROWS( GetStarvationMortalityThreshold(-0.1) );
	CHECK_THROWS( GetStarvationMortalityThreshold(1.1) );
	SECTION("default threshold"){
		GetStarvationMortalityThreshold get_mort;
		CHECK_THROWS( get_mort(-0.1) );
		CHECK_THROWS( get_mort(1.1) );
		CHECK( get_mort(0.0) == 1.0);
		CHECK( get_mort(0.1) == 0.0);
	}
	SECTION("custom threshold"){
		GetStarvationMortalityThreshold get_mort(0.05);
		CHECK( get_mort(0.0) == 1.0 );
		CHECK( get_mort(0.04) == 1.0 );
		CHECK( get_mort(0.05) == 0.0 );
		CHECK( get_mort(0.06) == 0.0 );
	}
}

TEST_CASE("Fauna::GrassForage", "") {
	GrassForage grass;
	CHECK_THROWS(grass.set_fpc(1.2));
	CHECK_THROWS(grass.set_fpc(-0.2));
	CHECK_THROWS(grass.set_mass(-0.2));
	CHECK_THROWS(grass.set_digestibility(-0.2));
	CHECK_THROWS(grass.set_digestibility(1.2));
}

TEST_CASE("Fauna::HabitatForage", "") {
	HabitatForage hf1 = HabitatForage();

	// Initialization
	REQUIRE( hf1.get_total().get_mass() == Approx(0.0) );
	REQUIRE( hf1.get_total().get_digestibility()   == Approx(0.0) );

	SECTION( "adding forage" ) {
		hf1.grass.set_mass(10.0);
		hf1.grass.set_digestibility(0.5);
		hf1.grass.set_fpc(0.3);

		REQUIRE( hf1.get_total().get_mass() == 10.0 );
		REQUIRE( hf1.get_total().get_digestibility()   == 0.5 );

		REQUIRE( Approx( hf1.grass.get_sward_density() )
				== hf1.grass.get_mass() / hf1.grass.get_fpc());

		std::vector<const HabitatForage*> hf_vector;
		hf_vector.push_back(&hf1);

		SECTION( "merging 2 similar forage objects" ) { 
			HabitatForage hf2 = hf1;
			hf_vector.push_back(&hf2); 
			const HabitatForage m = HabitatForage::merge(hf_vector);

			// There should be no change in the average 

			// dry matter
			CHECK( Approx(m.grass.get_mass())       
					== hf1.grass.get_mass() );
			CHECK( Approx(m.get_total().get_mass()) 
					== hf1.get_total().get_mass() );

			// digestibility
			CHECK( Approx(m.grass.get_digestibility())         
					== hf1.grass.get_digestibility() );
			CHECK( Approx(m.get_total().get_digestibility())   
					== hf1.get_total().get_digestibility() );

			// For FPC-specific values the same:
			CHECK( Approx(m.grass.get_fpc())           
					== hf1.grass.get_fpc() );
			CHECK( Approx(m.grass.get_sward_density()) 
					== hf1.grass.get_sward_density() );
		}

		SECTION( "merging 2 forage objects with different mass and fpc" ) {
			HabitatForage hf2 = hf1;
			hf2.grass.set_mass(20.0);
			hf2.grass.set_fpc(0.6);

			hf_vector.push_back(&hf2);

			const HabitatForage m = HabitatForage::merge(hf_vector);

			CHECK( Approx(m.grass.get_mass()) 
					== (hf1.grass.get_mass() + hf2.grass.get_mass()) / 2.0);
			CHECK( Approx(m.grass.get_fpc()) 
					== (hf1.grass.get_fpc() + hf2.grass.get_fpc()) / 2.0);

			CHECK( Approx(m.grass.get_digestibility())
					== (hf1.grass.get_digestibility() * hf1.grass.get_mass()
						+ hf2.grass.get_digestibility() * hf2.grass.get_mass())
					/ (hf1.grass.get_mass() + hf2.grass.get_mass() ));

			CHECK( Approx(m.grass.get_sward_density())
					== (hf1.grass.get_sward_density() * hf1.grass.get_fpc()
						+ hf2.grass.get_sward_density() * hf2.grass.get_fpc())
					/ (hf1.grass.get_fpc() + hf2.grass.get_fpc()) );
		}
	}
}

TEST_CASE("Fauna::HerbivoreBase", "") {
	// Since HerbivoreBase cannot be instantiated directly, we
	// test the relevant functionality in HerbivoreBaseDummy.

	// PREPARE SETTINGS
	Parameters params;
	REQUIRE(params.is_valid());
	Hft hft = create_hfts(1, params)[0];
	REQUIRE(hft.is_valid(params));

	// Let’s throw some exceptions
	CHECK_THROWS( HerbivoreBaseDummy(-1, 0.5, &hft, // age_days
			SEX_MALE) );
	CHECK_THROWS( HerbivoreBaseDummy(100, 0.5, NULL,  // hft== NULL
			SEX_MALE) );
	CHECK_THROWS( HerbivoreBaseDummy(100, 1.1, &hft, //body_conditon
			SEX_MALE) );
	CHECK_THROWS( HerbivoreBaseDummy(100, -0.1, &hft,//body_conditon
			SEX_MALE) );


	SECTION("Body mass"){

		SECTION("Birth"){
			// call the birth constructor
			const HerbivoreBaseDummy birth(&hft, SEX_MALE );

			REQUIRE( &birth.get_hft() == &hft );
			REQUIRE( birth.get_age_days() == 0 );
			REQUIRE( birth.get_age_years() == 0 );


			const double lean_bodymass_birth 
				= hft.bodymass_birth * (1.0-hft.bodyfat_birth);
			const double pot_bodymass_birth
				= lean_bodymass_birth / (1.0-hft.bodyfat_max);
			// body mass
			CHECK( birth.get_bodymass() == Approx(hft.bodymass_birth) );
			CHECK( birth.get_potential_bodymass() 
					== Approx(pot_bodymass_birth) );
			CHECK( birth.get_lean_bodymass() 
					== Approx(lean_bodymass_birth) );
			// fat mass
			CHECK( birth.get_bodyfat() == Approx(hft.bodyfat_birth) );
			CHECK( birth.get_max_fatmass() 
					== Approx(pot_bodymass_birth * hft.bodyfat_max) );
		}

		SECTION("Pre-adult"){
			const double BODY_COND = 1.0;
			SECTION("pre-adult male"){
				const int AGE_YEARS = hft.maturity_age_phys_male/2;
				const int AGE_DAYS = AGE_YEARS * 365;
				const HerbivoreBaseDummy male_young(
						AGE_DAYS, BODY_COND, &hft, SEX_MALE );
				REQUIRE( male_young.get_age_days() == AGE_DAYS );
				REQUIRE( male_young.get_age_years() == AGE_YEARS );
				CHECK( male_young.get_bodymass()   < hft.bodymass_male );
				CHECK( male_young.get_bodymass()   > hft.bodymass_birth );
				CHECK( male_young.get_fatmass() / male_young.get_max_fatmass()
						== Approx(BODY_COND) );
			}

			SECTION("pre-adult female"){
				const int AGE_YEARS = hft.maturity_age_phys_female/2;
				const int AGE_DAYS = AGE_YEARS * 365;
				const HerbivoreBaseDummy female_young( AGE_DAYS,
						BODY_COND, &hft, SEX_FEMALE );
				REQUIRE( female_young.get_age_days() == AGE_DAYS );
				REQUIRE( female_young.get_age_years() == AGE_YEARS );
				CHECK( female_young.get_bodymass() < hft.bodymass_female);
				CHECK( female_young.get_bodymass() > hft.bodymass_birth );
				CHECK( female_young.get_fatmass() / female_young.get_max_fatmass()
						== Approx(BODY_COND) );
			}
		}

		SECTION("Adult with full fat") {
			const double BODY_COND  = 1.0; // optimal body condition
			SECTION("Adult male with full fat") {
				const int AGE_YEARS = hft.maturity_age_phys_male;
				const int AGE_DAYS = AGE_YEARS * 365;
				const HerbivoreBaseDummy male_adult(
						AGE_DAYS, BODY_COND, &hft, SEX_MALE );
				// AGE
				REQUIRE( male_adult.get_age_days() == AGE_DAYS );
				REQUIRE( male_adult.get_age_years() == AGE_YEARS );
				// BODY MASS
				CHECK( male_adult.get_bodymass()   
						== Approx(hft.bodymass_male) );
				CHECK( male_adult.get_potential_bodymass()
						== male_adult.get_bodymass() );
				CHECK( male_adult.get_lean_bodymass()
						== Approx(hft.bodymass_male * (1.0-hft.bodyfat_max)) );
				// FAT MASS
				CHECK( male_adult.get_max_fatmass() 
						== Approx(hft.bodyfat_max*hft.bodymass_male) );
				CHECK( male_adult.get_bodyfat()
						== Approx(hft.bodyfat_max) );
				CHECK( male_adult.get_fatmass() / male_adult.get_max_fatmass()
						== Approx(BODY_COND) );
			}
			SECTION("Adult female with full fat"){
				const int AGE_YEARS = hft.maturity_age_phys_female;
				const int AGE_DAYS = AGE_YEARS * 365;
				const HerbivoreBaseDummy female_adult(
						hft.maturity_age_phys_male*365, 
						BODY_COND, &hft, SEX_FEMALE );
				// AGE
				REQUIRE( female_adult.get_age_days() == AGE_DAYS );
				REQUIRE( female_adult.get_age_years() == AGE_YEARS );
				// BODY MASS
				CHECK( female_adult.get_bodymass() 
						== Approx(hft.bodymass_female) );
				CHECK( female_adult.get_potential_bodymass()
						== female_adult.get_bodymass() );
				CHECK( female_adult.get_lean_bodymass()
						== Approx(hft.bodymass_female * (1.0-hft.bodyfat_max)) );
				// FAT MASS
				CHECK( female_adult.get_max_fatmass() 
						== Approx(hft.bodyfat_max*hft.bodymass_female) );
				CHECK( female_adult.get_bodyfat()
						== Approx(hft.bodyfat_max) );
				CHECK( female_adult.get_fatmass() / female_adult.get_max_fatmass()
						== Approx(BODY_COND) );
			}
		}

		SECTION("Adult with low fat") {
			const double BODY_COND  = 0.3; // poor body condition

			SECTION("Male") {
				const HerbivoreBaseDummy male_adult(
						hft.maturity_age_phys_male*365, 
						BODY_COND, &hft, SEX_MALE );
				// BODY MASS
				CHECK( male_adult.get_potential_bodymass()
						== Approx(hft.bodymass_male) );
				CHECK( male_adult.get_lean_bodymass() + male_adult.get_max_fatmass()
						== Approx(male_adult.get_potential_bodymass()) );
				// FAT MASS
				CHECK( male_adult.get_max_fatmass() 
						== Approx(hft.bodyfat_max*hft.bodymass_male) );
				CHECK( male_adult.get_fatmass() / male_adult.get_max_fatmass()
						== Approx(BODY_COND) );
			}

			SECTION("Female"){
				const HerbivoreBaseDummy female_adult(
						hft.maturity_age_phys_male*365, 
						BODY_COND, &hft, SEX_FEMALE );
				// BODY MASS
				CHECK( female_adult.get_potential_bodymass()
						== Approx(hft.bodymass_female) );
				CHECK( female_adult.get_lean_bodymass() + female_adult.get_max_fatmass()
						== Approx(female_adult.get_potential_bodymass()) );
				// FAT MASS
				CHECK( female_adult.get_max_fatmass() 
						== Approx(hft.bodyfat_max*hft.bodymass_female) );
				CHECK( female_adult.get_fatmass() / female_adult.get_max_fatmass()
						== Approx(BODY_COND) );
			}
		} 
	}
}

TEST_CASE("Fauna::HerbivoreCohort", "") {
	// PREPARE SETTINGS
	Parameters params;
	REQUIRE(params.is_valid());
	Hft hft = create_hfts(1, params)[0];
	REQUIRE(hft.is_valid(params));

	// exceptions (only specific to HerbivoreCohort)
	// initial density negative
	CHECK_THROWS( HerbivoreCohort(10, 0.5, &hft, 
				SEX_MALE, -1.0) ); 

	const double BC = 0.5; // body condition
	const int AGE = 3 * 365;
	const double DENS = 10.0; // [ind/km²]

	// constructor (only test what is specific to HerbivoreCohort)
	REQUIRE( HerbivoreCohort(AGE, BC, &hft, 
				SEX_MALE, DENS).get_ind_per_km2() == Approx(DENS) );

	SECTION("is_same_age()"){
		REQUIRE( AGE%365 == 0 );
		const HerbivoreCohort cohort1(AGE, BC, &hft, 
				SEX_MALE, DENS);
		// very similar cohort
		CHECK( cohort1.is_same_age(
					HerbivoreCohort(AGE, BC, &hft, 
						SEX_MALE, DENS)));
		// in the same year
		CHECK( cohort1.is_same_age(
					HerbivoreCohort(AGE+364, BC, &hft, 
						SEX_MALE, DENS)));
		// the other is younger
		CHECK( ! cohort1.is_same_age(
					HerbivoreCohort(AGE-364, BC, &hft, 
						SEX_MALE, DENS)));
		// the other is much older
		CHECK( ! cohort1.is_same_age(
					HerbivoreCohort(AGE+366, BC, &hft, 
						SEX_MALE, DENS)));
	}

	SECTION("merge"){
		HerbivoreCohort cohort(AGE, BC, &hft, SEX_MALE, DENS);

		SECTION("exceptions"){
			SECTION("wrong age"){ // wrong age
				HerbivoreCohort other(AGE+365, BC, &hft, SEX_MALE, DENS);
				CHECK_THROWS( cohort.merge(other) );
			}
			SECTION("wrong sex"){ // wrong sex
				HerbivoreCohort other(AGE, BC, &hft, SEX_FEMALE, DENS);
				CHECK_THROWS( cohort.merge(other) );
			}
			SECTION("wrong HFT"){ // wrong HFT
				Hft hft2 = create_hfts(2, params)[1];
				REQUIRE( hft2 != hft );
				HerbivoreCohort other(AGE, BC, &hft2, SEX_MALE, DENS);
				CHECK_THROWS( cohort.merge(other) );
			}
		}
		
		SECTION("merge whole cohort"){
			const double old_bodymass = cohort.get_bodymass();
			const double BC2 = BC+0.1; // more fat in the other cohort
			const double DENS2 = DENS*1.5;
			HerbivoreCohort other(AGE, BC2, &hft, SEX_MALE, DENS2);
			cohort.merge(other);
			// The other cohort is gone
			CHECK( other.get_kg_per_km2() == 0.0 );
			// More fat => more bodymass
			CHECK( cohort.get_bodymass() > old_bodymass );
			// That’s all we can test from the public methods...
		}
	}

	SECTION("mortality"){
	}
}

TEST_CASE("Fauna::HerbivoreIndividual", "") {
	// PREPARE SETTINGS
	Parameters params;
	REQUIRE(params.is_valid());
	Hft hft = create_hfts(1, params)[0];
	REQUIRE(hft.is_valid(params));

	const double BC   = 0.5;  // body condition
	const int AGE     = 842;  // som arbitrary number [days]
	const double AREA = 10.0; // [km²]

	// exceptions (only specific to HerbivoreIndividual)
	// invalid area
	CHECK_THROWS( HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, -1.0) ); 
	CHECK_THROWS( HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, 0.0) ); 
	CHECK_THROWS( HerbivoreIndividual(&hft, SEX_MALE, -1.0) ); 
	CHECK_THROWS( HerbivoreIndividual(&hft, SEX_MALE, 0.0) ); 

	// birth constructor 
	REQUIRE( HerbivoreIndividual(&hft, 
				SEX_MALE, AREA).get_area_km2() == Approx(AREA) );
	// establishment constructor
	REQUIRE( HerbivoreIndividual(AGE, BC, &hft, 
				SEX_MALE, AREA).get_area_km2() == Approx(AREA) );

	SECTION("Mortality"){
		hft.mortality_factors.insert(MF_STARVATION_THRESHOLD);

		// create with zero fat reserves
		const double BC_DEAD = 0.0; // body condition
		HerbivoreIndividual ind(AGE, BC_DEAD, &hft, SEX_MALE, AREA);

		// after one simulation day it should be dead
		double offspring_dump;
		ind.simulate_day(0, offspring_dump);
		CHECK( ind.is_dead() );
	}
	// NOTE: We cannot test mortality because it is a stochastic
	// event.
}

TEST_CASE("Fauna::Hft",""){
	Hft hft = Hft();
	std::string msg;

	SECTION("not valid without name"){
		hft.name = "";
		CHECK_FALSE( hft.is_valid(Fauna::Parameters(), msg) );
	}
}

TEST_CASE("Fauna::HftList",""){
	HftList hftlist;

	// check initial size
	REQUIRE(hftlist.size()==0);

	// invalid access
	CHECK_THROWS(hftlist[1]);
	CHECK_THROWS(hftlist[-1]);
	CHECK_THROWS(hftlist["abc"]);

	// add Hft without name
	Hft noname;
	noname.name="";
	CHECK_THROWS(hftlist.insert(noname));

	// add some real HFTs
	Hft hft1; 
	hft1.name="hft1";
	hft1.is_included = true;
	REQUIRE_NOTHROW(hftlist.insert(hft1));
	REQUIRE(hftlist.size()==1);
	REQUIRE(hftlist[0].name == "hft1");
	REQUIRE(hftlist.begin()->name == "hft1");

	Hft hft2;
	hft2.name = "hft2";
	hft2.is_included = false;
	REQUIRE_NOTHROW( hftlist.insert(hft2) );
	REQUIRE( hftlist.size()==2 );
	REQUIRE_NOTHROW( hftlist[1] );

	// find elements
	CHECK(hftlist["hft2"].name == "hft2");
	CHECK(hftlist["hft1"].name == "hft1");
	CHECK(hftlist.contains("hft1"));
	CHECK(hftlist.contains("hft2"));
	CHECK_FALSE(hftlist.contains("abc"));

	// substitute element 
	hft2.lifespan += 2; // change a property outside list
	REQUIRE(hftlist[hft2.name].lifespan != hft2.lifespan);
	hftlist.insert(hft2); // replace existing
	CHECK( hftlist[hft2.name].lifespan == hft2.lifespan );
	
	// remove excluded
	hftlist.remove_excluded();
	CHECK(hftlist.size()==1);
	CHECK(hftlist.contains(hft1.name)); // hft1 included
	CHECK_FALSE(hftlist.contains(hft2.name)); // hft2 NOT included
}

TEST_CASE("Fauna::HftPopulationsMap", "") {
	HftPopulationsMap map;
	const int NPOP    = 3; // populations count
	const int NHERBIS = 5; // herbivores count

	// create some HFTs
	Hft hfts[NPOP];
	hfts[0].name = "hft1";
	hfts[1].name = "hft2";
	hfts[2].name = "hft3";
	for (int i=0; i<NPOP; i++)
		hfts[i].establishment_density = (double) i;

	// create some populations with establishment_density
	DummyPopulation* pops[NPOP];
	for (int i=0; i<NPOP; i++) {
		// Create population object
		std::auto_ptr<PopulationInterface> new_pop(
				new DummyPopulation(&hfts[i]));
		pops[i] = (DummyPopulation*) new_pop.get();

		// Create herbivores
		for (int j=0; j<NHERBIS; j++)
			pops[i]->establish();

		REQUIRE(pops[i]->get_list().size() == NHERBIS);
		// add them to the map -> transfer ownership
		map.add(new_pop);
	}

	REQUIRE(map.size() == NPOP);
	REQUIRE( map.get_all_herbivores().size() == NPOP*NHERBIS );

	// throw some exceptions
	CHECK_THROWS(map.add(std::auto_ptr<PopulationInterface>()));
	CHECK_THROWS(map.add(std::auto_ptr<PopulationInterface>(
					new DummyPopulation(&hfts[0])))); // HFT already exists

	// check if iterator access works
	// ... for populations
	HftPopulationsMap::const_iterator itr = map.begin();
	while (itr != map.end()) {
		const PopulationInterface& pop = **itr;
		bool found = false; 
		// check against HFTs (order in the map is not defined)
		for (int i=0; i<NPOP; i++)
			if (pop.get_hft() == hfts[i]) {
				found = true;
				const HerbivoreInterface& herbiv = **pop.get_list().begin();
				// check if herbivore access works (that no bad memory
				// access is thrown or so)
				herbiv.get_ind_per_km2();
			}
		CHECK(found);
		itr++;
	}
	// ... for herbivore list
	HerbivoreVector all = map.get_all_herbivores();
	for (HerbivoreVector::iterator itr=all.begin(); 
			itr != all.end(); itr++){
		const HerbivoreInterface& herbiv=**itr;
		// check if herbivore access works (that no bad memory
		// access is thrown or so)
		herbiv.get_ind_per_km2();
	}


	// check random access
	for (int i=0; i<NPOP; i++)
		CHECK(&map[hfts[i]] == pops[i]);
	CHECK_THROWS(map[Hft()]); // unnamed Hft is not in map
}

TEST_CASE("Fauna::IndividualPopulation", "") {
	const double AREA = 10.0; // habitat area [km²]
	// prepare parameters
	Parameters params;
	REQUIRE(params.is_valid());

	// prepare HFT
	const int ESTABLISH_COUNT = 100; // [ind]
	Hft hft = create_hfts(1, params)[0];
	hft.establishment_density = ESTABLISH_COUNT / AREA; // [ind/km²]
	hft.mortality_factors.clear(); // immortal herbivores
	REQUIRE( hft.is_valid(params) );

	// prepare creating object
	CreateHerbivoreIndividual create_ind(&hft, &params, AREA);

	SECTION("Create empty population"){
		IndividualPopulation pop(create_ind);
		REQUIRE( pop.get_list().empty() );
		REQUIRE( population_lists_match( pop ) );
		REQUIRE( pop.get_hft() == hft );
	}

	SECTION("Establishment"){
		IndividualPopulation pop(create_ind);
		pop.establish();
		REQUIRE( !pop.get_list().empty() ); 
		CHECK( population_lists_match(pop) );
		// Do we have the exact number of individuals?
		CHECK( pop.get_list().size() == ESTABLISH_COUNT ); 
		// Does the total density match?
		CHECK( get_total_pop_density(pop) 
				== Approx(hft.establishment_density) );

		SECTION("Removal of dead individuals"){
			// kill all herbivores in the list with a copy assignment
			// trick
			hft.mortality_factors.insert(MF_STARVATION_THRESHOLD);
			// create a dead individual
			const int AGE=10;
			const double BC=0.0; // starved to death!
			const double AREA=10.0;
			HerbivoreIndividual dead(AGE, BC, &hft, SEX_FEMALE, AREA);
			double offspring_dump;
			dead.simulate_day(0, offspring_dump);
			REQUIRE( dead.is_dead() );

			// copy assign it to every ind. in the list
			{
				HerbivoreVector list = pop.get_list();
				for (HerbivoreVector::iterator itr=list.begin();
						itr!=list.end(); itr++) {
					HerbivoreInterface* pint = *itr;
					HerbivoreIndividual* pind = (HerbivoreIndividual*) pint;
					pind->operator=(dead);
					REQUIRE( pind->is_dead() );
				}
			}

			// now the list should be empty both the read-only and the
			// writable
			const IndividualPopulation& const_pop = pop;
			ConstHerbivoreVector const_list = const_pop.get_list();
			CHECK( const_list.size() == 0 );
			HerbivoreVector list = pop.get_list();
			CHECK( const_list.size() == 0 );
		}

	}

	SECTION("Establishment with odd number"){
		IndividualPopulation pop(create_ind);
		// Reduce establishment density by one individual.
		// The population should round up to have even sex ratio.
		hft.establishment_density -= 1.0/AREA; // [ind/km²]
		pop.establish();
		REQUIRE( !pop.get_list().empty() ); 
		CHECK( population_lists_match(pop) );
		// Do we have the exact number of individuals?
		CHECK( pop.get_list().size() == ESTABLISH_COUNT ); 

		// Does the total density match?
		// 1 ind. should be created extra, but not more
		CHECK( get_total_pop_density(pop) 
				>= Approx(hft.establishment_density) );
		CHECK( get_total_pop_density(pop) 
				<= Approx(hft.establishment_density+1.0/AREA) );
	}

	SECTION("Offspring"){
		IndividualPopulation pop(create_ind);
		const double IND_DENS = 2.0; // [ind/km²]
		const int IND_COUNT = IND_DENS * AREA; // [ind]

		CHECK_THROWS( pop.create_offspring(-1.0) );

		pop.create_offspring(IND_DENS);
		REQUIRE( pop.get_list().size() == IND_COUNT );
		CHECK( population_lists_match(pop) );
		REQUIRE( get_total_pop_density(pop) == Approx(IND_DENS) );

		// add more offspring
		pop.create_offspring(IND_DENS);
		REQUIRE( pop.get_list().size() == 2*IND_COUNT );
		CHECK( population_lists_match(pop) );
		REQUIRE( get_total_pop_density(pop) == Approx(2.0*IND_DENS) );
	}

}

TEST_CASE("Fauna::Parameters", ""){
	// defaults must be valid.
	REQUIRE( Parameters().is_valid() );
}

TEST_CASE("Fauna::parse_comma_separated_param", "") {
	CHECK( parse_comma_separated_param("").empty() );
	CHECK( *parse_comma_separated_param("abc").begin() == "abc" );
	CHECK( *parse_comma_separated_param("abc,def").begin()
			== "abc" );
	CHECK( *(++parse_comma_separated_param("abc,def").begin())
			== "def" );
	CHECK( *parse_comma_separated_param("abc, def").begin()
			== "abc" );
	CHECK( *(++parse_comma_separated_param("abc, def").begin())
			== "def" );
	CHECK( *parse_comma_separated_param("abc,    def  ").begin()
			== "abc" );
	CHECK( *(++parse_comma_separated_param(" abc,    def").begin())
			== "def" );
}

TEST_CASE("Fauna::ReproductionIllius2000", "") {
	// NOTE: We are more tolerant with the Approx() function of
	// the CATCH framework (by adjusting Approx().epsilon().

	const double INC = 1.0;
	const int START = 100;
	const int LENGTH = 90;
	const double OPT = 1.0; // optimal body condition

	// exceptions
	SECTION("exceptions"){
		CHECK_THROWS(ReproductionIllius2000(-1 , 1  , 1.0));
		CHECK_THROWS(ReproductionIllius2000(365, 0  , 1.0));
		CHECK_THROWS(ReproductionIllius2000(0  , 0  , 1.0));
		CHECK_THROWS(ReproductionIllius2000(0  , -1 , 1.0));
		CHECK_THROWS(ReproductionIllius2000(0  , 366, 1.0));
		CHECK_THROWS(ReproductionIllius2000(0  , 363, -1.0));
		ReproductionIllius2000 rep(START, LENGTH, INC);
		CHECK_THROWS( rep.get_offspring_density(-1, OPT) );
		CHECK_THROWS( rep.get_offspring_density(365, OPT) );
		CHECK_THROWS( rep.get_offspring_density(START, -0.1) );
		CHECK_THROWS( rep.get_offspring_density(START, 1.1) );
	}

	SECTION("check breeding season"){
		ReproductionIllius2000 rep(START, LENGTH, INC);

		// within season
		CHECK( rep.get_offspring_density(START, OPT) > 0.0 );
		CHECK( rep.get_offspring_density(START+LENGTH, OPT) > 0.0 );

		// before breeding season
		CHECK( rep.get_offspring_density(START-1, OPT) == 0.0 );

		// after season
		CHECK( rep.get_offspring_density(START+LENGTH+1, OPT) == 0.0 );
	}

	SECTION("check breeding season extending over year boundary"){
		const int LENGTH2 = 360;
		ReproductionIllius2000 rep(START, LENGTH2, INC);
		const int END = (START+LENGTH2) % 365;
		
		// within season
		CHECK( rep.get_offspring_density(START, OPT) > 0.0 );
		CHECK( rep.get_offspring_density(END, OPT) > 0.0 );

		// before breeding season
		CHECK( rep.get_offspring_density(START-1, OPT) == 0.0 );

		// after season
		CHECK( rep.get_offspring_density(END+1, OPT) == 0.0 );
	}

	SECTION("higher annual increase makes more offspring"){
		const double INC2 = INC * 1.5;
		REQUIRE( INC2 > INC );
		ReproductionIllius2000 rep1(START, LENGTH, INC);
		ReproductionIllius2000 rep2(START, LENGTH, INC2);
		CHECK( rep1.get_offspring_density(START, OPT)
				< rep2.get_offspring_density(START, OPT));
		CHECK( rep1.get_offspring_density(START, OPT) < INC );
		CHECK( rep2.get_offspring_density(START, OPT) < INC2 );
	}

	SECTION("better body condition makes more offspring"){
		const double BAD = OPT/2.0; // bad body condition
		ReproductionIllius2000 rep(START, LENGTH, INC);
		CHECK( rep.get_offspring_density(START, BAD)
				<  rep.get_offspring_density(START, OPT) );
	}

	SECTION("one-day season length -> all offspring at once"){
		const double BAD = OPT/2.0; // bad body condition
		ReproductionIllius2000 rep(START, 1, INC);
		CHECK( rep.get_offspring_density(START, OPT) 
				== Approx(INC).epsilon(0.05) );
		CHECK( rep.get_offspring_density(START, BAD) < INC);
	}

	SECTION("Sum of offspring over year must be max. annual increase"){
		ReproductionIllius2000 rep(START, LENGTH, INC);
		// sum over whole year
		double sum_year = 0.0;
		for (int d=0; d<365; d++)
			sum_year += rep.get_offspring_density((START+d)%364, OPT);
		CHECK( sum_year == Approx(INC).epsilon(0.05) );

		// sum over the breeding season only
		double sum_season = 0.0;
		for (int d=START; d<START+LENGTH; d++)
			sum_season += rep.get_offspring_density(d, OPT);
		CHECK( sum_season == Approx(INC).epsilon(0.05) );
	}
}

TEST_CASE("Fauna::Simulator", "") {
	Fauna::Parameters params;
	REQUIRE( params.is_valid() );

	// prepare HFT list
	HftList hftlist = create_hfts(3, params); 
	
	Simulator sim(params, hftlist);

	// Check create_populations()
	std::auto_ptr<HftPopulationsMap> pops = sim.create_populations();
	REQUIRE( pops.get() != NULL );
	CHECK(pops->size() == hftlist.size());
	// find all HFTs
	HftList::const_iterator itr_hft = hftlist.begin();
	while (itr_hft != hftlist.end()){
		bool found_hft = false;
		HftPopulationsMap::const_iterator itr_pop = pops->begin();
		while (itr_pop != pops->end()) {
			if ((*itr_pop)->get_hft() == *itr_hft)
				found_hft = true;
			itr_pop++;
		}
		CHECK(found_hft);
		itr_hft++;
	}

	// Check simulate_day()
	DummyHabitat habitat;
	CHECK_THROWS( sim.simulate_day(-1, habitat, true) );
	CHECK_THROWS( sim.simulate_day(366, habitat, true) );
	const bool do_herbivores = true;
	for (int d=0; d<365; d++){
		sim.simulate_day(d,habitat, do_herbivores);
		// NOTE: So far, only the day is checked!
		CHECK(habitat.get_day_public() == d);
	}
}

TEST_CASE("FaunaSim::LogisticGrass", "") {
	LogisticGrass::Parameters grass_settings;
	grass_settings.reserve   = 2.0; // just an arbitrary positive number
	grass_settings.init_mass = 1.0;
	grass_settings.saturation = 10*grass_settings.init_mass;

	const int day = 1; // day of the year
	
	SECTION("Grass initialization"){
		LogisticGrass grass(grass_settings);
		CHECK( grass.get_forage().get_mass() == Approx(grass_settings.init_mass) );
		CHECK( grass.get_forage().get_digestibility()
				== Approx(grass_settings.digestibility));
		CHECK( grass.get_forage().get_fpc()
				== Approx(grass_settings.fpc));

		// exceptions
		CHECK_THROWS( grass.grow_daily(-1) );
		CHECK_THROWS( grass.grow_daily(365) );
	}
	// Let the grass grow for one day and compare before and after 

	SECTION("No grass growth"){
		grass_settings.growth = 0.0;
		grass_settings.decay  = 0.0;

		LogisticGrass grass(grass_settings);


		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() == Approx(before.get_mass()) );
	}

	SECTION("Positive grass growth"){
		grass_settings.growth = 0.1;
		grass_settings.decay  = 0.0;

		LogisticGrass grass(grass_settings);

		// Let the grass grow for one day and check it’s greater

		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() > before.get_mass() );

		// Let it grow for very long and check that it reaches saturation
		for (int i=0; i<1000000; i++) 
			grass.grow_daily(i%365);
		CHECK( grass.get_forage().get_mass() == Approx(grass_settings.saturation) );
	}

	SECTION("Neutral grass growth"){
		grass_settings.growth = 0.1;
		grass_settings.decay  = grass_settings.growth;

		LogisticGrass grass(grass_settings);

		// Let the grass grow for one day and check it’s greater

		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() == Approx(before.get_mass()) );
	}

	SECTION("Negative grass growth"){
		grass_settings.growth =  0.1;
		grass_settings.decay  = 2.0 * grass_settings.growth;

		LogisticGrass grass(grass_settings);

		// Let the grass grow for one day and check it’s greater

		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() < before.get_mass() );
	}
}

TEST_CASE("FaunaSim::SimpleHabitat", "") {
	SimpleHabitat::Parameters settings;
	settings.grass.init_mass     = 1.0;
	settings.grass.growth        = 0.0;
	settings.grass.saturation    = 3.0;
	settings.grass.digestibility = 0.5;

	// create a habitat with some populations
	const Fauna::Parameters params;
	Simulator sim(params, create_hfts(4, params)); 
	SimpleHabitat habitat(sim.create_populations(), settings);
	
	SECTION("Initialization") {
		CHECK( habitat.get_available_forage().grass.get_fpc() 
			== Approx(settings.grass.fpc) );
		CHECK( habitat.get_available_forage().grass.get_mass() 
			== Approx(settings.grass.init_mass) );
		CHECK( habitat.get_available_forage().grass.get_digestibility()
			== Approx(settings.grass.digestibility) );
	}

	SECTION("Remove forage"){
		const HabitatForage avail = habitat.get_available_forage();

		SECTION("Remove some forage"){
			ForageMass eaten;
			eaten.set_grass(avail.grass.get_mass() * 0.5);
			habitat.remove_eaten_forage(eaten);
			CHECK( habitat.get_available_forage().grass.get_mass()
					== Approx(avail.grass.get_mass() - eaten.get_grass()) );
			// more forage types here
		}

		SECTION("Remove all forage"){
			ForageMass eaten;
			eaten.set_grass(avail.grass.get_mass());
			habitat.remove_eaten_forage(eaten);
			CHECK( habitat.get_available_forage().grass.get_mass() == 0.0);
			// more forage types here
		}

		SECTION("Remove more forage than is available"){
			ForageMass too_much;
			too_much.set_grass(avail.grass.get_mass() * 1.1);
			CHECK_THROWS( habitat.remove_eaten_forage(too_much) );
			// more forage types here
		}
	}

	// TODO output?
}

TEST_CASE("FaunaSim::HabitatGroup","") {
	// Make sure the group creates its habitats
	HabitatGroup group(1.0,1.0); // lon,lat
	group.reserve(5);
	for (int i=1; i<5; i++) {
		// add a habitat
		group.add(std::auto_ptr<Habitat>(new DummyHabitat()));
		CHECK( group.size() == i );
		CHECK( group.get_habitat_references().size() == i ); 
	}
	// Make sure the references are pointing correctly to the objects
	const std::vector<Habitat*> refs = group.get_habitat_references();
	HabitatGroup::const_iterator itr = group.begin();
	int j=0;
	while (itr != group.end()){
		CHECK( refs[j] == *itr );
		j++;
		itr++;
	}
}

TEST_CASE("FaunaSim::HabitatGroupList","") {
	// Make sure the group creates its habitats
	HabitatGroupList gl;
	gl.reserve(5);

	// add some habitat groups
	for (int i=1; i<5; i++) {
		HabitatGroup& group = gl.add(new HabitatGroup(i,i));
		for (int j=1; j<4; j++) {
			// add a habitat
			group.add(std::auto_ptr<Habitat>(new DummyHabitat()));
		}
		CHECK(gl.size() == i);
	}
	// Don’t allow adding a group with same coordinates twice
	CHECK_THROWS( gl.add(new HabitatGroup(1,1)) );
}

// TEST_CASE("Fauna::is_first_day_of_month()", ""){
// 	CHECK_THROWS( is_first_day_of_month(-1) );
// 	CHECK_THROWS( is_first_day_of_month(365) );
// 	CHECK( is_first_day_of_month(0) );
// 	CHECK_FALSE( is_first_day_of_month(1) );
// 	CHECK_FALSE( is_first_day_of_month(44) );
// 	CHECK( is_first_day_of_month(-1+31) );
// 	CHECK( is_first_day_of_month(-1+31+28) );
// 	CHECK( is_first_day_of_month(-1+31+28+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30+31+30+31) );
// }
