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

// forward declarations
class Pftlist;

namespace Fauna {
	// forward declarations
	class Hft;

	/// Helper function to extract comma-separated substrings.
	/**
	 * \param strparam String of comma-separated substrings.
	 * \return List of substrings, leading and trailing whitespaces
	 * removed.
	 */
	std::list<std::string> parse_comma_separated_param(
			const std::string& strparam);

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
			 * This function only checks if mandatory items have been parsed and 
			 * converts strings to numeric values.
			 * \see \ref Parameters::is_valid()
			 *
			 * After all parameters are read, it also checks if all HFTs are
			 * valid (see \ref Hft::is_valid()).
			 *
			 * Any parameter can only be mandatory if \ref Parameters::ifherbivory 
			 * is true.
			 *
			 * \warning Call this *only* from \ref plib_callback()
			 * because otherwise the plib functions cause addressing errors.
			 *
			 * \param callback The callback code defined with
			 * \ref declareitem().
			 * \param ppft The reference to the \ref Pft object that
			 * is currently parsed. 
			 * \throw std::invalid_argument if `ppft==NULL`
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
			 * \throw std::invalid_argument if `ppft==NULL`
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
			/** \return A set of **valid** \ref Hft objects
			 * \throw std::logic_error if parsing is not completed */
			const HftList& get_hftlist()const{
				if (!parsing_completed())
					throw std::logic_error("Fauna::ParamReader::get_hftlist() "
							"Parsing of instruction files not completed yet.");
				for (HftList::const_iterator itr=hftlist.begin();
						itr!=hftlist.end(); itr++)
					assert(itr->is_valid(get_params()));
				return hftlist;
			}

			/// Get the parameters as read from instruction file.
			/** \return Valid parameters object.
			 * \throw std::logic_error if parsing is not completed */
			const Parameters& get_params()const{
				if (!parsing_completed())
					throw std::logic_error("Fauna::ParamReader::get_params() "
							"Parsing of instruction files not completed yet.");
				assert(params.is_valid());
				return params;
			}

			/// Whether parsing of instruction file is completed.
			bool parsing_completed()const{return completed;}
		private:
			/// A parameter that must not be omitted.
			struct MandatoryParam{
				std::string param;
				std::string required_by;
				/// Constructor with `required_by`
				MandatoryParam(std::string param, std::string required_by):
					param(param), required_by(required_by){}
				/// Simple constructor for always mandatory parameters.
				MandatoryParam(std::string param): param(param){}
			}; 
			typedef std::list<MandatoryParam> MandatoryParamList;

			/// Check both global and HFT parameters.
			/**
			 * \param[out] fatal Returns true if the simulation cannot be run
			 * with given parameters.
			 * \param[out] msg Error and warning messages.
			 * \see \ref Fauna::Hft::is_valid()
			 * \see \ref Fauna::Parameters::is_valid()
			 */
			void check_all_params(bool& fatal, std::string& msg)const;

			/** \copydoc check_all_params(bool&, std::string&)const
			 * Each \ref Fauna::PftParams object in the list of PFTs is checked
			 * and additionally, whether there is any edible forage defined.
			 * \param[in] pftlist Lift of \ref Pft objects. The \ref Pftlist
			 * object will not be changed.
			 * \see \ref Fauna::PftParams::is_valid()
			 */
			void check_all_params(Pftlist& pftlist,
					bool& fatal, std::string& msg)const;

			/// Check with \ref itemparsed() through list of parameters
			/** Prints error messages. 
			 * \param list List of mantdatory parameters
			 * \param prefix prefix for the error message.
			 * \return true if every parameter was parsed, false if 
			 * at least one was missing.
			 * \see \ref itemparsed()
			 */
			bool check_mandatory(const MandatoryParamList& list,
					const std::string& prefix);

			/// The \ref Hft object that’s currently being parsed.
			Hft current_hft;

			HftList hftlist;
			Parameters params;
			bool completed;

			/** @{ \name Temporary storage for parameters. */

			/// Holds two integer numbers.
			int integer_pair[2];

			/// Holds two double floating-point numbers.
			double double_pair[2];

			/// Holds the currently parsed string parameter.
			std::string strparam;

			/** @} */ // Temporyar storage for parameters

			ParamReader():completed(false) {}                     // hide constructor
			ParamReader(ParamReader const&);    // deleted copy constructor
			void operator=(ParamReader const&); // deleted assignment constructor
	};
}

#endif // HERBIV_PARAMREADER_H
