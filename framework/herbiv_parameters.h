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

namespace Fauna {

	/// Type of digestibility model, corresponding to one implementation of \ref Fauna::GetDigestibility
	enum DigestibilityModelType{
		/// Use \ref PftDigestibility.
		DM_PFT_FIXED,
		/// Use \ref DigestibilityFromNPP.
		DM_NPP,
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

	/// How to convert snow water equivalent to snow depth.
	enum SnowDepthModel{
		/// Assume a constant 10:1 ratio of snow water equivalent to snow depth.
		SD_TEN_TO_ONE
	};

	/// Parameters for the herbivory module.
	struct Parameters{
		// alphabetical order

		/// How the forage digestibility of \ref Individual objects is calculated
		DigestibilityModelType digestibility_model;

		/// Algorithm for how to distribute available forage among herbivores.
		/** Default: \ref FD_EQUALLY */
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

		/// How to convert snow water equivalent from LPJ-GUESS to snow depth.
		/** Default: \ref SD_TEN_TO_ONE */
		SnowDepthModel snow_depth_model;

		/// Constructor with default (valid!) settings
		Parameters(): // alphabetical order
			forage_distribution(FD_EQUALLY),
			free_herbivory_years(0),
			habitat_area_km2(100.0),
			herbivore_establish_interval(365),
			herbivore_type(HT_COHORT),
			ifherbivory(false),
			snow_depth_model(SD_TEN_TO_ONE)
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
