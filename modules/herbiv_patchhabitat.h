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
	class GetSnowDepth;

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
		 * injection).
		 * \param snow_depth_model Strategy object for
		 * calculating the snow depth in the patch (constructor
		 * injection).
		 * \see \ref sec_inversion_of_control
		 * \throw std::invalid_argument If `digestibility_model` or
		 * `snow_depth_model` is a NULL pointer.
		 */
		PatchHabitat( 
				Patch& patch, 
				std::auto_ptr<GetDigestibility> digestibility_model,
				std::auto_ptr<GetSnowDepth> snow_depth_model);

	public: // ----- Fauna::Habitat implementation -----
		/** \copydoc Habitat::add_excreted_nitrogen()
		 * Add all nitrogen to the plant-available soil pool
		 * (\ref Soil::nmass_avail) and count the nitrogen input as flux 
		 * (\ref Fluxes::EXCRETEDN).
		*/
		virtual void add_excreted_nitrogen(const double kgN_per_km2);

		/** \copydoc Habitat::get_available_forage()
		 * \see \ref GetDigestibility
		 * \see \ref Individual::get_forage_mass()
		 */
		virtual HabitatForage get_available_forage() const;

		/** \copybrief Habitat::get_environment()
		 * - Snow: Calculated from \ref Soil::snowpack, employing the given
		 *   \ref SnowDepthModel algorithm.
		 */
		virtual HabitatEnvironment get_environment() const;

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
		std::auto_ptr<GetSnowDepth> get_snow_depth;
	private:
		/// Reference to the patch.
		Patch& patch;
	};
}
#endif // HERBIV_PATCHHABITAT_H
