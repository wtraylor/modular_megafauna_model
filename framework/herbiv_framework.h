///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FRAMEWORK_H
#define HERBIV_FRAMEWORK_H

#include "herbiv_digestibility.h" // for DigestibilityModelType

namespace Fauna{
	// Forward declarations
	class Habitat;
	class Parameters;

	/// Central herbivory framework class.
	class Simulator{
	public:
		/// Constructor, initializing simulation settings.
		Simulator(const Parameters& params);

		/// Simulate all herbivore interactions for the current day.
		/**
		 * Call this even if you don’t want herbivores in your model
		 * because it prepares the output data which might be
		 * used by \ref HerbivoryOutput.
		 * \param day_of_year Current day of year (0 = Jan 1st)
		 * \param habitat The Habitat to simulate
		 * \param do_herbivores Whether to perform herbivore simulations.
		 * If false, only the output data of the habitats are updated.
		 */
		void simulate_day(const int day_of_year, Habitat& habitat,
				const bool do_herbivores);
	protected:
		const Parameters& params;
	};
}
#endif // HERBIV_FRAMEWORK_H
