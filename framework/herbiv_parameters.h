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

// forward declarations
class Pft;

namespace Fauna {

	
	/// Type of digestibility model, corresponding to one class
	/** \ingroup group_herbivory */
	enum DigestibilityModelType{
		/// \ref PftDigestibility : The fixed value \ref Pft::digestibility is taken.
		DM_PFT_FIXED
	};


	/// Parameter for selecting algorithm for forage distribution among herbivores
	/** \ingroup group_herbivory */
	enum ForageDistributionAlgorithm{
		/// Equal forage distribution
		/** \see \ref Simulator::distribute_forage_equally() */
		FD_EQUALLY
	};

	/// Parameter for selecting the class implementing \ref HerbivoreInterface.
	/** \ingroup group_herbivory */
	// TODO document
	enum HerbivoreType{
		HT_COHORT,
		HT_INDIVIDUAL
	};

	/// Parameters for the herbivory module.
	/** \ingroup group_herbivory */
	struct Parameters{
			/// Minimum mass density [kg/km²] for a living herbivore object.
			double dead_herbivore_threshold; 

			/// Whether herbivory is enabled.
			bool ifherbivory;

			/// Algorithm for how to distribute available forage among herbivores
			ForageDistributionAlgorithm forage_distribution; 

			/// Simulation years without herbivores (as part of spinup). 
			int free_herbivory_years;

			/// How the forage digestibility is calculated
			DigestibilityModelType dig_model;

			/// Which kind of herbivore class to use
			HerbivoreType herbivore_type;

			/// Constructor with default settings
			Parameters(): // alphabetical order
				ifherbivory(false),
				forage_distribution(FD_EQUALLY),
				free_herbivory_years(0){}

			/// Check if the parameters are valid
			/**
			 * \param[out] messages Warning and error messages.
			 * \return true if everything is valid, false if not
			 */
			bool is_valid(std::string& messages)const;
	};

}

#endif // HERBIV_PARAMETERS_H
