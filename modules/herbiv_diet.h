///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes defining the diet composition of herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_DIET_H
#define HERBIV_DIET_H

#include "herbiv_forageclasses.h" // for ForageEnergy

namespace Fauna{

	
	/// Calculates proportions of different forage types in a herbivore’s diet.
	/**
	 * Strategy interface and function object (see \ref sec_strategy).
	 */
	struct ComposeDietInterface{
		/// Calculate diet proportions.
		/**
		 * \param foragable_energy Net energy [MJ/ind] for each forage
		 * type that the herbivore could *potentially* harvest *if* it
		 * would exclusively eat that one forage type.
		 * \param total_energy_needs Total net energy needs [MJ/ind] 
		 * that the herbivore to meet.
		 * \return Energy [MJ/ind] of each forage type that the 
		 * herbivore chooses to eat. The sum must not exceed
		 * `total_energy_needs`!
		 */
		virtual ForageEnergy operator()(
				const ForageEnergy& foragable_energy,
				const double total_energy_needs) = 0;
	};

	/// Compose a pure grass diet.
	class PureGrazerDiet: public ComposeDietInterface{
		virtual ForageEnergy operator()(
				const ForageEnergy& foragable_energy,
				const double total_energy_needs){
			ForageEnergy result;
			// put all energy into grass.
			result.set(FT_GRASS, foragable_energy[FT_GRASS]);
			return result;
		}
	};
}

#endif // HERBIV_DIET_H

