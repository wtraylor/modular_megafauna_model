///////////////////////////////////////////////////////////////////////////////////////
/// \file    
/// \brief   \ref Fauna::Habitat implementations for testing purpose.
/// \ingroup group_herbivory
/// \author  Wolfgang Pappa, Senckenberg BiK-F
/// \date    June 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_testhabitat.h"
#include <sstream> // for is_valid() messages

using namespace Fauna;
using namespace FaunaSim;

//============================================================
// LogisticGrass
//============================================================

bool LogisticGrass::Parameters::is_valid(std::string& msg)const{
	// The message text is written into an output string stream
	std::ostringstream stream;
	bool is_valid = true;

	if (decay < 0.0){
		stream << "decay < 0.0" << std::endl;
		is_valid = false;
	}

	if (digestibility < 0.0){
		stream << "digestibility < 0.0" << std::endl;
		is_valid = false;
	}

	if (digestibility > 1.0){
		stream << "digestibility > 1.0" << std::endl;
		is_valid = false;
	}

	if (fpc < 0.0){
		stream << "fpc < 0.0" << std::endl;
		is_valid = false;
	}

	if (fpc > 1.0){
		stream << "fpc > 1.0" << std::endl;
		is_valid = false;
	}

	if (growth < 0.0){
		stream << "growth < 0.0" << std::endl;
		is_valid = false;
	}

	if (init_mass < 0.0){
		stream << "init_mass < 0.0" << std::endl;
		is_valid = false;
	}

	if (reserve <= 0.0){
		stream << "reserve <= 0.0" << std::endl;
		is_valid = false;
	}

	if (saturation < 0.0){
		stream << "saturation < 0.0" << std::endl;
		is_valid = false;
	}

	if (init_mass > saturation){
		stream << "init_mass > saturation" << std::endl;
		is_valid = false;
	}

	msg = stream.str();
	return is_valid;
}

void LogisticGrass::grow_daily(const int day_of_year){
	if (day_of_year>=365 || day_of_year<0)
		throw std::invalid_argument("FaunaSim::LogisticGrass::grow_daily() "
				"day_of_year out of range");
	forage.set_fpc(settings.fpc);
	forage.set_digestibility(settings.digestibility);

	// available dry matter
	const double dm_avail   = forage.get_mass();
	// total dry matter
	const double dm_total   = dm_avail + settings.reserve;
	// proportional net increase of total dry matter
	const double net_growth = settings.growth - settings.decay;

	// Total grass maximum dry matter
	const double total_saturation = settings.saturation + settings.reserve;

	// Absolute change in total dry matter
	const double dm_total_change 
		= dm_total * net_growth * (1.0 - dm_total / total_saturation);

	// new total dry matter
	const double dm_total_new = dm_total + dm_total_change;
	// new available dry matter
	const double dm_avail_new = std::max( dm_total_new - settings.reserve, 0.0 );
	// With negative net growth, the available dry matter can drop below zero.
	// That’s why max() is used to here
	
	forage.set_mass(dm_avail_new);
}

//============================================================
// SimpleHabitat
//============================================================

void SimpleHabitat::init_day(const int today){
	// Call parent function
	Habitat::init_day(today);
	grow_daily(today);
}

void SimpleHabitat::remove_eaten_forage(const ForageMass& eaten_forage){
	// call parent class implementation
	Habitat::remove_eaten_forage(eaten_forage);

	// create temporary grass object. Remove eaten mass from it, and
	// then assign it to the actual grass object.
	GrassForage new_grass = grass.get_forage();
	if (new_grass.get_mass() - eaten_forage.get_grass() < 0.0)
		throw std::logic_error("FaunaSim::SimpleHabitat::remove_eaten_forage() "
				"Eaten grass exceeds available grass.");
	new_grass.set_mass(new_grass.get_mass() - eaten_forage.get_grass());
	grass.set_forage(new_grass);
}
