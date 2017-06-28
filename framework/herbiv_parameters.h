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
#include "herbiv_digestibility.h"

// forward declarations
class Pft;

namespace Fauna {

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
	 */
	class Parameters{
		public:
			/// Get the global singleton instance
			static Parameters& get_global(){
				static Parameters global_instance;
				return global_instance;
			}

		public: 
			/// Whether herbivory is enabled.
			bool ifherbivory;

			/// Simulation years without herbivores (as part of spinup). 
			int free_herbivory_years;

			/// How the forage digestibility is calculated
			DigestibilityModelType dig_model;

		public: 
			/// Constructor with default settings
			Parameters():
				ifherbivory(false),
				free_herbivory_years(0),
				dig_model(DM_UNDEFINED) {}
			
			/// Declare parameters within the plib framework.
			/** Helper function to \ref plib_declarations().
			 * \warning Call this *only* from \ref plib_declarations() 
			 * \param id The type identifier of the currently parsed block.
			 * \param setname The name of the currently parsed block.
			 * \param ppft Pointer to the currently parsed \ref Pft
			 * object if id=\ref BLOCK_PFT, NULL otherwise. 
			 */
			static void declare_parameters(const int id, 
					const std::string& setname, Pft* ppft); 

			/// Check global parameters within the plib framework.
			/** 
			 * Helper function to \ref plib_callback().
			 * \warning Call this *only* from \ref plib_callback()
			 * because otherwise the plib functions cause addressing errors.
			 *
			 * \param callback The callback code defined with
			 * \ref declareitems().
			 * \param ppft The reference to the \ref Pft object that
			 * is currently parsed. 
			 */
			static void callback(const int callback, Pft* ppft);

			/// Initialise herbivory-related parameters in a new Pft
			static void init_pft(Pft& pft);

		private: 
			/// Holds the currently parsed string parameter
			static std::string strparam;
	};

}

#endif // HERBIV_PARAMETERS_H
