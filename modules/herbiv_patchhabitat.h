///////////////////////////////////////////////////////////////////////////////////////
/// \file 
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
class Pft;

namespace Fauna {
	// forward declarations
	class Parameters;

	/// Class with herbivore habitat functionality for an LPJ-GUESS \ref Patch.
	/** 
	 * Objects of this class are supposed to be owned by the
	 * corresponding \ref Patch object which takes care of 
	 * instantiating and releasing.
	 *
	 * \note There are no unit tests for this class because the
	 * class \ref Patch cannot reasonably be instantiated in a
	 * unit test without the whole LPJ-GUESS framework.
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

	/// Herbivory-related parameters of a \ref Pft object.
	struct PftParams{

		/// Proportional carbon content in dry matter forage
		/** 
		 * Needed to convert \ref Individual::cmass_leaf  and \ref Individual::anpp 
		 * to dry matter forage biomass.
		 * Does not need to be defined if \ref forage_type is \ref Fauna::FT_INEDIBLE.
		 */
		double c_in_dm_forage;

		/// Fractional digestibility of herbivore forage for ruminants
		/** 
		 * Does not need to be defined if \ref forage_type is \ref Fauna::FT_INEDIBLE.
		 * \see sec_herbiv_digestibility
		 */
		double digestibility;

		/// Forage type of this plant type.
		/** Use \ref Fauna::FT_INEDIBLE to exclude it from being eaten.*/
		Fauna::ForageType forage_type;

		/// Whether the vegetation of this Pft is edible.
		bool is_edible()const{ return forage_type != Fauna::FT_INEDIBLE; }

		//------------------------------------------------------------
		
		/// Constructor
		PftParams(const Pft& pft):pft(pft){}

		/// Check if the parameters are valid
		/**
		 * \param[in] params Global parameters of the herbivory module.
		 * \param[out] messages Warning and error messages.
		 * \return true if everything is valid, false if not
		 */
		bool is_valid(const Parameters& params, std::string& messages)const;
		private:
			const Pft& pft;
	};

}
#endif // HERBIV_PATCHHABITAT_H
