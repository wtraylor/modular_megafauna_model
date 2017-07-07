//////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////
#ifndef HERBIV_DIGESTIBILITY_H
#define HERBIV_DIGESTIBILITY_H

#include "assert.h"

// Forward declarations of LPJ-GUESS classes
class Individual;

namespace Fauna {

	/// Abstract base class (interface) for calculating forage digestibility
	/** 
	 * This class implements the following design patterns:
	 * \ref sec_singleton, \ref sec_strategy.
	 * \ingroup group_herbivory **/
	class DigestibilityModel{
	public:
		/// Get current forage digestibility for one plant individual.
		/** Implement this in any derived digestibility model class. */
		virtual double get_digestibility(const Individual& individual) const = 0;

		/// Get the global instance
		static const DigestibilityModel& get_model(){
			assert(global_model != NULL);
			return *global_model;
		}

		/// Set global model
		/** Call this before \ref get_model() !
		 * Call it only once!*/
		static void init_global(DigestibilityModel& instance){
			assert( global_model == NULL ); // assert it’s called only once
			global_model = &instance;
		}

	private:
		/// Global class instance.
		static DigestibilityModel* global_model;
	};

	/// Digestibility model using \ref Pft::digestibility.
	/** 
	 * Digestibility is a fixed value for each \ref Pft.
	 * \ingroup group_herbivory
	 */
	class PftDigestibility : public DigestibilityModel{
	public:
		/// Returns \ref Pft::digestibility.
		double get_digestibility(const Individual& individual) const;
	};
}
#endif // HERBIV_DIGESTIBILITY_H
