///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Large herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_forageclasses.h"
#include "assert.h"

using namespace Fauna;

std::string Fauna::get_forage_type_name(const ForageType ft) {
	switch (ft){
		case FT_GRASS   : return "grass"; break;
		case FT_INEDIBLE: return "inedible";
		default         : return "undefined";
	} 
}

//------------------------------------------------------------
// HABITATFORAGE
//------------------------------------------------------------

HabitatForage HabitatForage::merge(const std::vector<const HabitatForage*> data){
	if (data.empty())
		throw std::invalid_argument("Fauna::HabitatForage::merge(): "
				"parameter data is empty.");
	HabitatForage result;

	/// Sums of item values to build average later
	double gr_mass_sum, gr_fpc_sum, gr_dig_sum,  gr_dig_sum_weight = 0.0;

	// Iterate over all data entries and accumulate the values in order to build
	// averages later.
	// Each value is weighted properly.
	for (int i=0; i<data.size(); i++){
		const HabitatForage& item = *data[i];
		// GRASS
		gr_dig_sum_weight += item.grass.get_digestibility() * item.grass.get_mass();
		gr_dig_sum        += item.grass.get_digestibility();
		gr_mass_sum       += item.grass.get_mass();
		gr_fpc_sum        += item.grass.get_fpc();
		// add other forage types here
	}

	// Build weighted averages
	if (gr_mass_sum > 0.0) 
		result.grass.set_digestibility( gr_dig_sum_weight / gr_mass_sum );
	
	// add other forage types here

	// Build simple averages 
	const double count = (double) data.size(); // count>0 because of prior exception check
	assert(count>0);
	result.grass.set_mass( gr_mass_sum / count );
	result.grass.set_fpc( gr_fpc_sum / count); 
	if (gr_mass_sum <= 0.0)
		result.grass.set_digestibility( gr_dig_sum / count );
	// add other forage types here 

	return result;
}

ForageBase HabitatForage::get_total() const{
	/// Return object
	ForageBase result;
	/// Sum of digestibility of all forage types (scaled by mass)
	double dig_sum_weight, dig_sum;

	// add other forage types here
	
	// Dry matter
	result.set_mass( grass.get_mass() );

	// Digestibility
	dig_sum_weight = grass.get_digestibility() * grass.get_mass();
	dig_sum = grass.get_digestibility();

	// build weighted average only if total mass is greater zero
	if (result.get_mass() > 0.0)
		result.set_digestibility( dig_sum_weight / result.get_mass() );
	else
		result.set_digestibility( dig_sum / (double) FORAGE_TYPE_COUNT );
	return result;
}
