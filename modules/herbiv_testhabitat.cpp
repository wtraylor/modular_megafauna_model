///////////////////////////////////////////////////////////////////////////////////////
/// \file    herbiv_testhabitat.cpp
/// \brief   \ref Fauna::Habitat implementations for testing purpose.
/// \ingroup group_herbivory
/// \author  Wolfgang Pappa, Senckenberg BiK-F
/// \date    June 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_testhabitat.h"

using namespace Fauna;

void TestGrass::grow_daily(const int day_of_year){
	assert( day_of_year >= 0 );
	assert( day_of_year < 365 );
	forage.set_fpc(settings.fpc);
	forage.set_digestibility(settings.digestibility);

	/// available dry matter
	const double dm_avail   = forage.get_mass();
	/// total dry matter
	const double dm_total   = dm_avail + settings.reserve;
	/// proportional net increase of total dry matter
	const double net_growth = settings.growth - settings.decay;

	/// Total grass maximum dry matter
	const double total_saturation = settings.saturation + settings.reserve;

	/// Absolute change in total dry matter
	const double dm_total_change 
		= dm_total * net_growth * (1.0 - dm_total / total_saturation);

	/// new total dry matter
	const double dm_total_new = dm_total + dm_total_change;
	/// new available dry matter
	const double dm_avail_new = std::max( dm_total_new - settings.reserve, 0.0 );
	// With negative net growth, the available dry matter can drop below zero.
	// That’s why max() is used to here
	
	forage.set_mass(dm_avail_new);
}

TestHabitatGroup::TestHabitatGroup(
		const double lon, const double lat,
		const int nhabitats,
		const TestHabitatSettings settings):
	lon(lon), lat(lat) {
		assert( nhabitats >= 1 );
		for (int i=0; i< nhabitats; i++)
			habitats.push_back( TestHabitat(settings) );
	}

void TestHabitat::init_todays_output(const int today){
	assert( today >= 0 );
	assert( today <= 365 );
	// Call parent function
	Habitat::init_todays_output(today);
	grow_daily(today);
}
void TestHabitat::remove_eaten_forage(const ForageMass& eaten_forage){
	// call parent class implementation
	Habitat::remove_eaten_forage(eaten_forage);

	// remove eaten grass from temporary object and assign it.
	GrassForage new_grass = grass.get_forage();
	new_grass.set_mass(new_grass.get_mass() - eaten_forage.grass);
	grass.set_forage(new_grass);
}
