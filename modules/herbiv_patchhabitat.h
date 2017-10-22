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

namespace Fauna {
	// forward declarations
	class GetDigestibility;

	/// Number of days over which phenology is averaged in plant individuals.
	/**
	 * \see \ref Individual::get_average_phenology()
	 */
	const int PHEN_AVG_PERIOD = 30; // 1 month

	/// Class with herbivore habitat functionality for an LPJ-GUESS \ref Patch.
	/** 
	 * Any object of this class are supposed to be owned by the
	 * corresponding \ref Patch object, which takes care of 
	 * instantiating and releasing.
	 *
	 * \note There are no unit tests for this class because the
	 * class \ref Patch cannot reasonably be instantiated in a
	 * unit test without the whole LPJ-GUESS framework.
	 *
	 * \see \ref sec_herbiv_lpjguess_integration
	 */
	class PatchHabitat : public Habitat{
	public:
		/// Constructor
		/**
		 * \param patch The one-to-one relationship to the patch
		 * \param digestibility_model Strategy object for
		 * calculating the digestibility of forage (constructor
		 * injection)
		 * \see \ref sec_inversion_of_control
		 */
		PatchHabitat( 
				Patch& patch, 
				std::auto_ptr<GetDigestibility> digestibility_model);

	public: // ----- Fauna::Habitat implementation -----
		/** \copydoc Habitat::get_available_forage()
		 * \see \ref GetDigestibility
		 * \see \ref Individual::get_forage_mass()
		 */
		virtual HabitatForage get_available_forage() const;

		/// Update at the start of the day.
		/**
		 * Update average phenology in all plant individuals by calling
		 * \ref Individual::update_average_phenology().
		 */
		virtual void init_day(const int today);

		/** \copydoc Habitat::remove_eaten_forage()
		 * \see \ref Individual::reduce_forage_mass()
		 * \see \ref sec_herbiv_forageremoval
		 */
		virtual void remove_eaten_forage(const ForageMass& eaten_forage);
	protected:
		std::auto_ptr<GetDigestibility> get_digestibility;
	private:
		/// Reference to the patch.
		Patch& patch;
	};
}
#endif // HERBIV_PATCHHABITAT_H
