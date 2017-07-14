///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Unit tests for megafauna herbivores.
/// \ingroup group_herbivory 
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 05-21-2017
///////////////////////////////////////////////////////////////////////////////////////

#include "catch.hpp" 
#include "herbiv_foraging.h" 
#include "herbiv_forageclasses.h" 
#include "herbiv_framework.h"
#include "herbiv_herbivore.h"
#include "herbiv_hft.h"
#include "herbiv_parameters.h"
#include "herbiv_testhabitat.h" 
#include <memory> // for std::auto_ptr
#include <vector>

using namespace Fauna;
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
			DummyHerbivore(const Hft& hft, const double density,
					const ForageMass demand=ForageMass()):
				hft(hft), density(density), demand(demand){}
			
			virtual void eat(const ForageMass& forage){}

			virtual double get_bodymass() const{return 1.0;}

			virtual ForageMass get_forage_demands(
					const int day,
					const HabitatForage& available_forage)const{
				return demand;
			}

			virtual const Hft& get_hft()const{return hft;}

			virtual double get_mass_density()const{return density;}

			virtual void simulate_day(const int day,
					double& offspring){offspring=0.0;}
		private:
			const Hft& hft;
			/// mass density
			const double density; 
			/// demanded forage
			const ForageMass demand;
	};
	
	/// A population of dummy herbivores
	class DummyPopulation: public PopulationInterface{
		private:
			const Hft& hft;
			std::vector<DummyHerbivore> vec;
		public:
			DummyPopulation(const Hft& hft):hft(hft){}

			virtual void create(const double density, const int age=0){
				vec.push_back(DummyHerbivore(get_hft(), density));
			}

			virtual const Hft& get_hft()const{return hft;}

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

			virtual void remove_dead(){}
	};

	/// Create a simple HftList
	HftList create_hfts(const int count){
		HftList hftlist;
		for (int i=0; i<count; i++){
			Hft hft;
			hft.is_included = true;
			// construct name for HFT
			std::ostringstream stream;
			stream << "hft"<<i;
			hft.name = stream.str();
		}
		return hftlist;
	}
} // namespace Fauna


// test cases in alphabetical order, please

TEST_CASE("Fauna::Dummies", "") {
	Hft hft1;
	hft1.name="hft1";
	DummyHerbivore dummy1 = DummyHerbivore(hft1, 1.0);
	DummyHerbivore dummy2 = DummyHerbivore(hft1, 0.0);

	DummyPopulation pop = DummyPopulation(hft1);
	pop.create(1.0);
	REQUIRE(pop.get_list().size() == 1);
}

TEST_CASE("Fauna::ForageMass", "") {
	ForageMass fm;

	// Initialization
	REQUIRE( Approx(fm.sum()) == 0.0);

	// exceptions
	CHECK_THROWS( fm.set_grass(-1.0) );
	CHECK_THROWS( fm.set_grass(NAN) );

	SECTION( "adding grass" ){ 
		fm.set_grass(2.0);
		REQUIRE( Approx(fm.sum()) == fm.get_grass() );
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

TEST_CASE("Fauna::HabitatOutputData","") {
	HabitatOutputData h1,h2,h3;

	const std::vector<HabitatOutputData> empty_vec;
	CHECK_THROWS(HabitatOutputData::merge(empty_vec));

	SECTION("Merge only valid objects") {
		h1.is_valid = h2.is_valid = h3.is_valid = true;
		std::vector<HabitatOutputData> vec;
		vec.push_back(h1);
		vec.push_back(h2);
		vec.push_back(h3);

		CHECK( HabitatOutputData::merge(vec).is_valid == true );
		CHECK_THROWS( HabitatOutputData::merge(vec, -1));
		CHECK_THROWS( HabitatOutputData::merge(vec, 0, 20));
	}

	SECTION("Merge valid objects with an invalid object") {
		h1.is_valid = h2.is_valid = true;
		h3.is_valid = false;
		std::vector<HabitatOutputData> vec;
		vec.push_back(h1);
		vec.push_back(h2);
		vec.push_back(h3);
		
		CHECK( HabitatOutputData::merge(vec).is_valid == false );
	}

}

TEST_CASE("Fauna::Hft",""){
	Hft hft = Hft();
	CHECK( hft.name == "" );
	std::string msg;

	// not valid without name
	CHECK_FALSE( hft.is_valid(Fauna::Parameters(), msg) );
}

TEST_CASE("Fauna::HftList",""){
	HftList hftlist;

	// check initial size
	REQUIRE(hftlist.size()==0);

	// invalid access
	CHECK_THROWS(hftlist[1]);
	CHECK_THROWS(hftlist["abc"]);

	// add Hft without name
	CHECK_THROWS(hftlist.insert( Hft() ));

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
		pops[i] = new DummyPopulation(hfts[i]);
		for (int j=0; j<NHERBIS; j++)
			pops[i]->create(hfts[i].establishment_density); 

		REQUIRE(pops[i]->get_list().size() == NHERBIS);
		// add them to the map
		map.add(pops[i]);
	}

	REQUIRE(map.size() == NPOP);

	// throw some exceptions
	CHECK_THROWS(map.add(NULL));
	CHECK_THROWS(map.add(pops[0]));

	// check if iterator access works
	HftPopulationsMap::const_iterator itr = map.begin();
	while (itr != map.end()) {
		const PopulationInterface& pop = **itr;
		bool found = false; 
		// check against HFTs (order in the map is not defined)
		for (int i=0; i<NPOP; i++)
			if (pop.get_hft() == hfts[i]) {
				found = true;
				const HerbivoreInterface& herbiv = **pop.get_list().begin();
				// check if density is the same and herbivore access works
				CHECK(herbiv.get_mass_density()  == hfts[i].establishment_density ); 
			}
		CHECK(found);
		itr++;
	}

	// check random access
	for (int i=0; i<NPOP; i++)
		CHECK(&map[hfts[i]] == pops[i]);
	CHECK_THROWS(map[Hft()]); // unnamed Hft is not in map
}

TEST_CASE("Fauna::Simulator", "") {
	Fauna::Parameters params;

	// prepare HFT list
	HftList hftlist = create_hfts(3); 
	
	Simulator sim(params, hftlist);

	// Check create_populations()
	std::auto_ptr<HftPopulationsMap> pops = sim.create_populations();
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
	Simulator sim(Fauna::Parameters(), create_hfts(4)); 
	SimpleHabitat habitat(sim.create_populations(), settings);
	
	SECTION("Initialization") {
		CHECK( habitat.get_available_forage().grass.get_fpc() 
			== Approx(settings.grass.fpc) );
		CHECK( habitat.get_available_forage().grass.get_mass() 
			== Approx(settings.grass.init_mass) );
		CHECK( habitat.get_available_forage().grass.get_digestibility()
			== Approx(settings.grass.digestibility) );

		// output should be invalid
		CHECK( !habitat.get_annual_output().is_valid );
		const std::vector<HabitatOutputData> monthly_output
			= habitat.get_monthly_output();
		for (int i=0; i<monthly_output.size(); i++)
			CHECK( !monthly_output[i].is_valid );
	}

	SECTION("Annual output with no growth") {
		for (int d=0; d<365; d++)
			habitat.init_todays_output(d);

		const HabitatOutputData annual_output = habitat.get_annual_output();
		const std::vector<HabitatOutputData> monthly_output
			= habitat.get_monthly_output();
		// output should all be valid now by the end of the year
		REQUIRE( annual_output.is_valid );
		for (int i=0; i<monthly_output.size(); i++)
			REQUIRE( monthly_output[i].is_valid );

		// because there was no growth and decay, the forage stayed the same
		CHECK( annual_output.available_forage.grass.get_mass() == 
				Approx(settings.grass.init_mass) );
		CHECK( annual_output.available_forage.grass.get_fpc() == 
				Approx(settings.grass.fpc) );
		CHECK( annual_output.available_forage.grass.get_digestibility() == 
				Approx(settings.grass.digestibility) );
	}

}

TEST_CASE("FaunaSim::HabitatGroup","") {
	// Make sure the group creates its habitats
	HabitatGroup group(1.0,1.0); // lon,lat
	group.reserve(5);
	for (int i=1; i<5; i++) {
		// add a habitat
		group.add(new DummyHabitat());
		CHECK( group.size() == i );
		CHECK( group.get_habitat_references().size() == i ); 
	}
	// Make sure the references are pointing correctly to the objects
	const std::vector<const Habitat*> refs = group.get_habitat_references();
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
			group.add(new DummyHabitat());
		}
		CHECK(gl.size() == i);
	}
	// Don’t allow adding a group with same coordinates twice
	CHECK_THROWS( gl.add(new HabitatGroup(1,1)) );
}
