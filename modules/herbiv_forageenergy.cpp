///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Energy content of herbivore forage.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_forageenergy.h"

using namespace Fauna;

// Definition of static members
const double GetNetEnergyContentDefault::ME_COEFFICIENT_GRASS
= 15.0;
const double GetNetEnergyContentDefault::DIGESTION_EFFICIENCY_HINDGUTS
= 0.93;

double GetNetEnergyContentDefault::get_per_forage_type(
		const ForageType forage_type, const double digestibility)const{
	if (!(digestibility>=0.0 && digestibility <= 1.0))
		throw std::invalid_argument("Fauna::GetNetEnergyContentDefault() "
				"Digestibility out of range");

	// PREPARE VARIABLES

	double ME = 0.0; // metabolizable energy

	if (forage_type == FT_INEDIBLE)
		return 0.0; 
	else if (forage_type == FT_GRASS) {
		ME = digestibility * ME_COEFFICIENT_GRASS;
	// ADD NEW FORAGE TYPES HERE
	} else
		throw std::logic_error("Fauna::GetNetEnergyContentDefault() "
				"Forage type is not implemented");

	const double e = digestion_efficiency;

	// COMPOSE THE FORMULA

	const double result = ME*(0.019*ME + 0.503)*e; // [MJ/kgDM] 

	assert(result >= 0.0);
	return result;
}
