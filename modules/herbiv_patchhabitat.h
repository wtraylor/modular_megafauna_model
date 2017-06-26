///////////////////////////////////////////////////////////////////////////////////////
/// \file herbiv_patchhabitat.h
/// \brief Implementation of a herbivore habitat for a \ref Patch.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_PATCHHABITAT_H
#define HERBIV_PATCHHABITAT_H

#include "herbiv_habitat.h"

// forward declarations
class Patch;

namespace Fauna {
	/// Class with herbivore habitat functionality for an LPJ-GUESS \ref Patch.
	/** 
	 * Objects of this class are supposed to be owned by the
	 * corresponding \ref Patch object which takes care of 
	 * instantiating and releasing.
	 *
	 * \note There are no unit tests for this class because the
	 * class \ref Patch cannot reasonably be instantiated in a
	 * unit test without the whole LPJ-GUESS framework.
	 *
	 * \ingroup group_herbivory 
	 */
	class PatchHabitat : public Habitat{
	public:
		/// Get currently available herbivore forage in the \ref Patch.
		/** \return in kg/km² */
		virtual HabitatForage get_available_forage() const;
		/// Remove eaten forage from the \ref Patch.
		/** \param eaten_forage in kg/m² */
		virtual void remove_eaten_forage(const ForageMass& eaten_forage);
		
		/// Constructor.
		PatchHabitat(Patch& patch):
			patch(patch)
		{}
	private:
		/// Reference to the patch.
		Patch& patch;

	};

}
#endif // HERBIV_PATCHHABITAT_H
