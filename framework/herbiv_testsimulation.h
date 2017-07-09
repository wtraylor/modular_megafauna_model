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
#include "herbiv_testhabitat.h" // for TestGrass::Settings
#include "herbiv_output.h" // for HerbivoryOutput

namespace Fauna{
	// forward declaration
	class HftList;
	class Parameters;

	/// Performs test simulations for herbivores outside of the LPJ-GUESS vegetation model
	/** \see \ref sec_singleton for an explanation of the design pattern used.
	 */
	class TestSimulator{
		public:
			/// Get singleton instance of the class.
			/** Creates the object on first call. */
			static TestSimulator& get_instance(){
				static TestSimulator instance;
				return instance;
			}

			/// Run a simulation
			/** Call this only after ins file has been read.
			 * \param global_params Instruction file parameters from
			 * the herbivory module not specific to the test simulation.*/
			void run(const Parameters& global_params, const HftList& hftlist);


			/// Check if all mandatory parameters have been read, terminates on error.
			/**
			 * This is a substitute for \ref plib_callback() in \ref parameters.cpp.
			 * Uses \ref fail() to terminate.
			 */
			void plib_callback(int callback);

			/// Declare instruction file parameters.
			/** Fills \ref mandatory_parameters.*/
			virtual void declare_parameters();
		protected:
			/// Output module
			GuessOutput::HerbivoryOutput herbiv_out;

			/// Parameter values from instruction file
			struct InstructionParameters{
				std::string outputdirectory;
				int nyears;
				int nhabitats_per_group, ngroups;
				TestHabitatSettings settings;
			} params; 

			/// List of mandatory instruction file parameters.
			std::vector<std::string> mandatory_parameters;

			/// Number of decimal places in output tables.
			static const int COORDINATES_PRECISION;
		private:
			/// Constructor, declares parameters
			TestSimulator(){declare_parameters();}

			/// Deleted copy constructor
			TestSimulator(TestSimulator const&); //don’t implement, it’s deleted
			/// Deleted assignment constructor
			void operator=(TestSimulator const&); //don’t implement, it’s deleted
	};

}

#endif // HERBIV_TESTSIMULATION_H
