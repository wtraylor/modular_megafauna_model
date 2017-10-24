////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Instruction file parameters of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
////////////////////////////////////////////////////////////
#ifndef HERBIV_PARAMETERS_H
#define HERBIV_PARAMETERS_H

#include <stdexcept>
#include <cassert> // for assert()

// forward declarations
class Pft;

namespace Fauna {

	/// Type of digestibility model, corresponding to one implementation of \ref Fauna::GetDigestibility
	enum DigestibilityModelType{
		/// Use \ref PftDigestibility.
		DM_PFT_FIXED,
		/// Use \ref DigestibilityPachzelt2013.
		DM_PFT_PACHZELT2013
	};

	/// Parameter for selecting algorithm for forage distribution among herbivores
	enum ForageDistributionAlgorithm{
		/// Equal forage distribution: \ref Fauna::DistributeForageEqually
		FD_EQUALLY
	};

	/// Parameter for selecting the class implementing \ref Fauna::HerbivoreInterface.
	enum HerbivoreType{
		/// Use class \ref HerbivoreCohort
		HT_COHORT,
		/// Use class \ref HerbivoreIndividual
		HT_INDIVIDUAL
	};

	/// Parameters for the herbivory module.
	struct Parameters{
		// alphabetical order

		/// Minimum individual density [ind/km²] for a living herbivore cohort.
		double dead_herbivore_threshold; 

		/// How the forage digestibility of \ref Individual objects is calculated
		DigestibilityModelType digestibility_model;

		/// Algorithm for how to distribute available forage among herbivores
		ForageDistributionAlgorithm forage_distribution; 

		/// Simulation years without herbivores (as part of spinup). 
		int free_herbivory_years;

		/// Habitat area [km²].
		/** Only relevant if \ref herbivore_type == \ref HT_INDIVIDUAL. */
		double habitat_area_km2;

		/// Days between establishment check for herbivores.
		int herbivore_establish_interval;

		/// Which kind of herbivore class to use
		HerbivoreType herbivore_type;

		/// Whether herbivory is enabled.
		bool ifherbivory;

		/// Constructor with default (valid!) settings
		Parameters(): // alphabetical order
			habitat_area_km2(100.0),
			dead_herbivore_threshold(0.1),
			forage_distribution(FD_EQUALLY),
			free_herbivory_years(0),
			herbivore_establish_interval(30),
			herbivore_type(HT_COHORT),
			ifherbivory(false)
		{
			// Make sure that the default values are implemented
			// correctly
			assert(is_valid()); 
		}

		/// Check if the parameters are valid
		/**
		 * \param[out] messages Warning and error messages.
		 * \return true if everything is valid, false if not
		 */
		bool is_valid(std::string& messages)const;

		/// Check if the parameters are valid
		/** \return true if everything is valid, false if not */
		bool is_valid()const{
			std::string dump;
			return is_valid(dump);
		}
		/// @}
	};

}

#endif // HERBIV_PARAMETERS_H
