///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Test simulations for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
///////////////////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_TESTSIMULATION_H
#define HERBIV_TESTSIMULATION_H

#include <vector>
#include "herbiv_testhabitat.h" // for SimpleHabitat::Parameters
#include "herbiv_outputmodule.h"      // for HerbivoryOutput

namespace Fauna{
	// forward declaration
	class HftList;
	class Parameters;
}
namespace FaunaSim{
	/// Performs test simulations for herbivores outside of the LPJ-GUESS vegetation model
	/** 
	 * \see \ref sec_singleton for an explanation of the design pattern used.
	 * \see \ref sec_herbiv_testsimulations
	 */
	class Framework{
		public:
			/// Get singleton instance of the class.
			/** Creates the object on first call. */
			static Framework& get_instance(){
				static Framework instance;
				return instance;
			}

			/// Run a simulation
			/** Call this only after ins file has been read.
			 * At all critical points, exceptions are caught, but there
			 * is no guarantee that no exception *slips through the cracks*.
			 * \param global_params Instruction file parameters from
			 * the herbivory module not specific to the test simulation.
			 * \param hftlist List of HFTs
			 * \return true on success, false on failure
			 */
			bool run(const Fauna::Parameters& global_params, const Fauna::HftList& hftlist);

			/// Check if all mandatory parameters have been read, terminates on error.
			/**
			 * This is a substitute for \ref plib_callback() in \ref parameters.cpp.
			 * Uses \ref fail() to terminate.
			 */
			void plib_callback(int callback);

			/// Declare instruction file parameters.
			/** Fills \ref mandatory_parameters.*/
			virtual void declare_parameters();
		private:
			/// Create a new habitat according to preferences.
			std::auto_ptr<Habitat> create_habitat()const;

			/// Output module
			GuessOutput::HerbivoryOutput herbiv_out;

			/// Parameter values from instruction file
			struct {
				std::string outputdirectory;
				int nyears;
				int nhabitats_per_group, ngroups;
				SimpleHabitat::Parameters habitat;
			} params; 

			/// List of mandatory instruction file parameters.
			std::vector<std::string> mandatory_parameters;

			/// Number of decimal places in output tables.
			static const int COORDINATES_PRECISION;
		
			/// Constructor, declares parameters
			Framework(){declare_parameters();}

			/// Deleted copy constructor
			Framework(Framework const&); //don’t implement, it’s deleted
			/// Deleted assignment constructor
			void operator=(Framework const&); //don’t implement, it’s deleted
	};
}

#endif // HERBIV_TESTSIMULATION_H
