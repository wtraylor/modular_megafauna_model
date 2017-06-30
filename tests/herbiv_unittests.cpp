///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Unit tests for megafauna herbivores.
/// \ingroup group_herbivory 
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 05-21-2017
///////////////////////////////////////////////////////////////////////////////////////

#include "catch.hpp" 
#include "herbiv_foraging.h" 
#include "herbiv_testhabitat.h" 
#include "herbiv_hft.h"
#include <vector>


using namespace Fauna;

TEST_CASE("Fauna::HabitatForage", "Tests the HabitatForage class") {
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

	SECTION("Merge only valid objects") {
		h1.is_valid = h2.is_valid = h3.is_valid = true;
		std::vector<HabitatOutputData> vec;
		vec.push_back(h1);
		vec.push_back(h2);
		vec.push_back(h3);

		CHECK( HabitatOutputData::merge(vec).is_valid == true );
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

TEST_CASE("Fauna::ForageMass", "Tests the ForageMass functions") {
	ForageMass fm;

	// Initialization
	REQUIRE( Approx(fm.sum()) == 0.0);

	SECTION( "adding grass" ){ 
		fm.grass = 2.0;
		REQUIRE( Approx(fm.sum()) == fm.grass );
	}
}

TEST_CASE("Fauna::TestGrass", "") {
	TestHabitatSettings::Grass grass_settings;
	grass_settings.reserve   = 2.0; // just an arbitrary positive number
	grass_settings.init_mass = 1.0;
	grass_settings.saturation = 10*grass_settings.init_mass;

	const int day = 1; // day of the year
	
	SECTION("Grass initialization"){
		TestGrass grass(grass_settings);
		CHECK( grass.get_forage().get_mass() == Approx(grass_settings.init_mass) );
		CHECK( grass.get_forage().get_digestibility()
				== Approx(grass_settings.digestibility));
		CHECK( grass.get_forage().get_fpc()
				== Approx(grass_settings.fpc));
	}
	// Let the grass grow for one day and compare before and after 

	SECTION("No grass growth"){
		grass_settings.growth = 0.0;
		grass_settings.decay  = 0.0;

		TestGrass grass(grass_settings);


		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() == Approx(before.get_mass()) );
	}

	SECTION("Positive grass growth"){
		grass_settings.growth = 0.1;
		grass_settings.decay  = 0.0;

		TestGrass grass(grass_settings);

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

		TestGrass grass(grass_settings);

		// Let the grass grow for one day and check it’s greater

		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() == Approx(before.get_mass()) );
	}

	SECTION("Negative grass growth"){
		grass_settings.growth =  0.1;
		grass_settings.decay  = 2.0 * grass_settings.growth;

		TestGrass grass(grass_settings);

		// Let the grass grow for one day and check it’s greater

		const GrassForage before = grass.get_forage();
		grass.grow_daily(day);
		const GrassForage after = grass.get_forage();

		CHECK( after.get_mass() < before.get_mass() );
	}
}

TEST_CASE("Fauna::TestHabitat", "") {
	TestHabitatSettings settings;
	settings.grass.init_mass     = 1.0;
	settings.grass.growth        = 0.0;
	settings.grass.saturation    = 3.0;
	settings.grass.digestibility = 0.5;
	
	TestHabitat habitat(settings);
	
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

TEST_CASE("Fauna::TestHabitatGroup","") {
	// Make sure the group creates its habitats
	for (int i=1; i<5; i++) {
		TestHabitatGroup group = TestHabitatGroup(
				1.0,1.0, // lon,lat
				i, // number of habitats
				TestHabitatSettings());
		CHECK( group.get_habitats().size() == i );
		CHECK( group.get_habitat_references().size() == i );

		// Make sure the references are pointing correctly to the objects
		const std::vector<const Habitat*> refs = group.get_habitat_references();
		const std::vector<TestHabitat> &objects = group.get_habitats();
		for (int j=0; j<refs.size(); j++)
			CHECK( refs[j] == &objects[j]);
	}
}

TEST_CASE("Fauna::HftList",""){
	HftList& hftlist = HftList::get_global();

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
	hft2.lifespan *= 2; // change a property outside list
	REQUIRE(hftlist[hft2.name].lifespan != hft2.lifespan);
	hftlist.insert(hft2); // replace existing
	CHECK( hftlist[hft2.name].lifespan == hft2.lifespan );
	
	// remove excluded
	hftlist.remove_excluded();
	CHECK(hftlist.size()==1);
	CHECK(hftlist.contains(hft1.name)); // hft1 included
	CHECK_FALSE(hftlist.contains(hft2.name)); // hft2 NOT included

	// close the list
	Hft hft3;
	hft3.name = "hft3";
	hftlist.close();
	CHECK_THROWS(hftlist.insert(hft3));
	CHECK_THROWS(hftlist.remove_excluded());
}

TEST_CASE("Fauna::Hft",""){
	Hft hft = Hft();
	CHECK( hft.name == "" );
	std::string msg;

	// not valid without name
	CHECK_FALSE( hft.is_valid(msg) );
}

