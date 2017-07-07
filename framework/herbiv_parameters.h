////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Instruction file parameters of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
////////////////////////////////////////////////////////////
#ifndef HERBIV_PARAMETERS_H
#define HERBIV_PARAMETERS_H

#include <string>
#include <vector> // for mandatory_hft_params
#include <stdexcept>

// forward declarations
class Pft;

namespace Fauna {

	// forward declarations
	class Hft;
	
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

	/// Parameter for selecting
	/** \ingroup group_herbivory */
	// TODO document
	enum HerbivoreType{
		HT_COHORT,
		HT_INDIVIDUAL
	};

	/// Parameters manager for the herbivory module.
	/**
	 * This class has a singleton global instance (see \ref sec_singleton),
	 * but can also be instantiated locally.
	 *
	 * But the global instance shall only be accessed in 
	 * \ref parameters.cpp and \ref framework.cpp for reading the
	 * parameters from the instruction script. The herbivory module
	 * itself does not use the global instance.
	 * \see \ref sec_herbiv_parameters
	 * \ingroup group_herbivory
	 */
	class Parameters{
		public:
			/// Get the global singleton instance
			static Parameters& get_global(){
				static Parameters global_instance;
				return global_instance;
			}

		public: 
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

		public: 
			/// Constructor with default settings
			Parameters(): // alphabetical order
				ifherbivory(false),
				forage_distribution(FD_EQUALLY),
				free_herbivory_years(0){}
			
			/// Declare parameters within the plib framework.
			/** Helper function to \ref plib_declarations().
			 * \warning Call this *only* from \ref plib_declarations() 
			 * \param id The type identifier of the currently parsed block.
			 * \param setname The name of the currently parsed block.
			 * \param ppft Pointer to the currently parsed \ref Pft
			 * object if id=\ref BLOCK_PFT, NULL otherwise. 
			 * \param is_help True if only help message is printed --> No
			 * assignments and validity checks, only parameters declared.
			 */
			static void declare_parameters(const int id, 
					const std::string& setname, Pft* ppft, const bool is_help); 

			/// Check global parameters within the plib framework.
			/** 
			 * Helper function to \ref plib_callback().
			 * \warning Call this *only* from \ref plib_callback()
			 * because otherwise the plib functions cause addressing errors.
			 *
			 * \param callback The callback code defined with
			 * \ref declareitem().
			 * \param ppft The reference to the \ref Pft object that
			 * is currently parsed. 
			 */
			static void callback(const int callback, Pft* ppft);

			/// Initialise herbivory-related parameters in a new Pft
			static void init_pft(Pft& pft);

		private: 
			/// The \ref Hft object that’s currently being parsed.
			static Hft current_hft;

			/// List of names of mandatory HFT parameters
			static std::vector<std::string> mandatory_hft_params;

			/// Holds the currently parsed string parameter
			static std::string strparam;
	};

}

#endif // HERBIV_PARAMETERS_H
