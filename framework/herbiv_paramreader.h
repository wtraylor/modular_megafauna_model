////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Reading instruction file parameters for the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
////////////////////////////////////////////////////////////
#ifndef HERBIV_PARAMREADER_H
#define HERBIV_PARAMREADER_H

#include "herbiv_hft.h"        // for Hft and HftList
#include "herbiv_parameters.h" // for Parameters
#include <list>                // for MandatoryParamList
#include <string>              // for strparam

namespace Fauna {
	// forward declarations
	class Hft;
	
	/// Reads \ref Parameters from the instruction file using plib library.
	/**
	 * Implemented as \ref sec_singleton.
	 * The global instance may only be accessed from \ref framework()
	 * and \ref parameters.cpp. The herbivory module itself does
	 * not use this class.
	 */
	class ParamReader{
		public:

			/// Check global parameters within the plib framework.
			/** 
			 * Helper function to \ref plib_callback().
			 * This function only checks if mandatory items have been
			 * parsed and converts strings to numeric values.
			 * \see \ref Parameters::is_valid()
			 *
			 * Any parameter can only be mandatory if 
			 * \ref Parameters::ifherbivory is true.
			 *
			 * \warning Call this *only* from \ref plib_callback()
			 * because otherwise the plib functions cause addressing errors.
			 *
			 * \param callback The callback code defined with
			 * \ref declareitem().
			 * \param ppft The reference to the \ref Pft object that
			 * is currently parsed. 
			 */
			void callback(const int callback, Pft* ppft);

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
			void declare_parameters(const int id, 
					const std::string& setname, Pft* ppft, 
					const bool is_help); 

			/// Get global singleton instance
			static ParamReader& get_instance(){
				static ParamReader global_instance;
				return global_instance;
			}

			/// Get the list of \ref Hft objects as read from instruction file
			const HftList& get_hftlist()const{return hftlist;}

			/// Get the parameters as read from instruction file.
			const Parameters& get_params()const{return params;}
		private:
			/// A parameter that must not be omitted.
			struct MandatoryParam{
				std::string param;
				std::string required_by;
				/// Constructor
				MandatoryParam(std::string param, std::string required_by):
					param(param), required_by(required_by){}
			}; 
			typedef std::list<MandatoryParam> MandatoryParamList;

			/// Check with \ref itemparsed() through list of parameters
			/** Prints error messages. 
			 * \param prefix prefix for the error message.
			 */
			bool check_mandatory(const MandatoryParamList& list,
					const std::string& prefix);

			/// The \ref Hft object that’s currently being parsed.
			Hft current_hft;

			HftList hftlist;
			MandatoryParamList mandatory_global_params;
			MandatoryParamList mandatory_hft_params;
			MandatoryParamList mandatory_pft_params;
			Parameters params;

			/// Holds the currently parsed string parameter
			std::string strparam;

			ParamReader(){}                     // hide constructor
			ParamReader(ParamReader const&);    // deleted copy constructor
			void operator=(ParamReader const&); // deleted assignment constructor
	};

}

#endif // HERBIV_PARAMREADER_H
