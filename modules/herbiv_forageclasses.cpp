///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Large herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_forageclasses.h"
#include <cassert> // for assert()

using namespace Fauna;

namespace{
	std::set<ForageType> get_all_forage_types(){
		std::set<ForageType> result;
		result.insert( FT_GRASS );
		// ADD NEW FORAGE TYPES HERE
		return result;
	}
}
// define global constant
namespace Fauna {
	const std::set<ForageType> FORAGE_TYPES = get_all_forage_types();
}

std::string Fauna::get_forage_type_name(const ForageType ft) {
	switch (ft){
		case FT_GRASS   : return "grass"; break;
		case FT_INEDIBLE: return "inedible";
		default         : throw std::logic_error(
													"Fauna::get_forage_type_name() "
													"Forage type is not implemented.");
	} 
}

//------------------------------------------------------------
// HABITATFORAGE
//------------------------------------------------------------

Digestibility HabitatForage::get_digestibility()const{

	Digestibility result;
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end(); ft++) {
		result.set(*ft, operator[](*ft).get_digestibility());
	}
	return result;
}

ForageMass HabitatForage::get_mass()const{
	ForageMass result;
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end(); ft++) {
		result.set(*ft, operator[](*ft).get_mass());
	}
	return result;
}

ForageBase HabitatForage::get_total() const{
	// Return object
	ForageBase result;

	const ForageMass mass = get_mass();

	result.set_mass( mass.sum() );

	// build weighted average only if total mass is greater zero
	if (result.get_mass() > 0.0) {
		// Create weighted average

		double dig_sum_weight = 0.0;
		// loop through each forage type
		for (ForageMass::const_iterator itr = mass.begin();
				itr != mass.end(); itr++){
			const ForageType ft = itr->first;
			dig_sum_weight += itr->second * operator[](ft).get_digestibility();
		}
		result.set_digestibility( dig_sum_weight / mass.sum() );
	} else 
		result.set_digestibility(0.0);
	return result;
}

HabitatForage HabitatForage::merge(const std::vector<const HabitatForage*> data){
	if (data.empty())
		throw std::invalid_argument("Fauna::HabitatForage::merge(): "
				"parameter data is empty.");
	HabitatForage result;

	ForageMass mass_sum;
	ForageValues<POSITIVE_AND_ZERO> dig_sum_weight;


	// Iterate over all data entries and accumulate the values in order
	// to build averages later.
	// Each value is weighted properly.
	for (int i=0; i<data.size(); i++)
	{
		const HabitatForage& item = *data[i];

		mass_sum += item.get_mass();
		const Digestibility digestibility = item.get_digestibility();

		// Loop through all edible forage types.
		for (Digestibility::const_iterator itr_ft = digestibility.begin();
				itr_ft != digestibility.end(); itr_ft++)
		{
			const ForageType ft = itr_ft->first;
			dig_sum_weight += itr_ft->second;
		}
	}

	// TODO

	return result;
}

