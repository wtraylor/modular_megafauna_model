///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes defining the diet composition of herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_DIET_H
#define HERBIV_DIET_H

#include "herbiv_forageclasses.h" // for ForageTypeMap

namespace Fauna{

	
	/// Calculates proportions of different forage types in a herbivore’s diet.
	/**
	 * Strategy interface and function object (see 
	 * \ref sec_strategy).
	 */
	struct ComposeDietInterface{
		virtual ForageTypeMap operator()(
				const ForageTypeMap foragable_energy,
				const double total_energy_needs) = 0;
	};

	/// Compose a pure grass diet.
	class PureGrazerDiet: public ComposeDietInterface{
		virtual ForageTypeMap operator()(
				const ForageTypeMap foragable_energy,
				const double total_energy_needs){
			ForageTypeMap result;
			result[FT_GRASS] = foragable_energy[FT_GRASS];
			return result;
		}
	};
}

#endif // HERBIV_DIET_H

