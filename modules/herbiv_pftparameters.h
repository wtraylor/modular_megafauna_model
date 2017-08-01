///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivory-related PFT parameters
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_PFTPARAMETERS_H
#define HERBIV_PFTPARAMETERS_H

#include "herbiv_forageclasses.h"

// forward declarations
class Pft;

namespace Fauna {
	// forward declarations
	class Parameters;

	/// Herbivory-related parameters of a \ref Pft object.
	struct PftParams{

		/// Proportional carbon content in dry matter forage
		/** 
		 * Needed to convert \ref Individual::cmass_leaf  and \ref Individual::anpp 
		 * to dry matter forage biomass.
		 * Does not need to be defined if \ref forage_type is \ref Fauna::FT_INEDIBLE.
		 */
		double c_in_dm_forage;

		/// Fractional digestibility of herbivore forage for ruminants
		/** 
		 * Does not need to be defined if \ref forage_type is \ref Fauna::FT_INEDIBLE.
		 * \see sec_herbiv_digestibility
		 */
		double digestibility;

		/// Forage type of this plant type.
		/** Use \ref Fauna::FT_INEDIBLE to exclude it from being eaten.*/
		Fauna::ForageType forage_type;

		/// Whether the vegetation of this Pft is edible.
		bool is_edible()const{ return forage_type != Fauna::FT_INEDIBLE; }

		//------------------------------------------------------------
		
		/// Constructor with default parameters
		/** It can not be guaranteed that the values are valid because
		 * they might depend on global parameters. */
		PftParams(const Pft& pft):pft(pft),
		c_in_dm_forage(0.5),
		digestibility(0.5),
		forage_type(FT_INEDIBLE){}

		/// Check if the parameters are valid
		/**
		 * \param[in] params Global parameters of the herbivory module.
		 * \param[out] messages Warning and error messages.
		 * \return true if everything is valid, false if not
		 */
		bool is_valid(const Parameters& params, std::string& messages)const;

		/// Check if the parameters are valid
		/**
		 * \param[in] params Global parameters of the herbivory module.
		 * \return true if everything is valid, false if not
		 */
		bool is_valid(const Parameters& params)const{
			std::string dump;
			return is_valid(params, dump);
		} 
		private:
			const Pft& pft;
	};

}
#endif // HERBIV_PFTPARAMETERS_H
