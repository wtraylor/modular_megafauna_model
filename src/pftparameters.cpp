//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivory-related PFT parameters
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "guess.h"
#include "pftparameters.h"
#include "parameters.h" // for Parameters
#include <sstream>             // for is_valid() messages

using namespace Fauna;
//
//============================================================
// PftParams
//============================================================

bool PftParams::is_valid(const Parameters& params,
		std::string& messages)const{

	bool is_valid = true;

	// The message text is written into an output string stream
	std::ostringstream stream;

	if (forage_type == FT_GRASS && pft.lifeform != GRASS){
		stream << "forage_type=\"grass\", but lifeform!=\"grass\""
			<< std::endl;
		is_valid = false;
	}

	if (forage_type != FT_INEDIBLE) {

		if (params.digestibility_model == DM_PFT_FIXED ||
				params.digestibility_model == DM_NPP) {
			if (digestibility <= 0.0){
				stream << "digestibility <= 0.0" << std::endl;
				is_valid = false;
			}
			if (digestibility > 1.0){
				stream << "digestibility > 1.0" << std::endl;
				is_valid = false;
			}
		}

		if (params.digestibility_model == DM_NPP) {
			if (digestibility_dead <= 0.0){
				stream << "digestibility_dead <= 0.0" << std::endl;
				is_valid = false;
			}
			if (digestibility_dead > 1.0){
				stream << "digestibility_dead > 1.0" << std::endl;
				is_valid = false;
			}
			if (digestibility_dead > digestibility) {
				stream << "digestibility_dead > digestibility" << std::endl;
				is_valid = false;
			}
		}

		if (inaccessible_forage < 0.0){
			stream << "inaccessible_forage < 0.0" << std::endl;
			is_valid = false;
		}
	}

	// convert stream to string
	messages = stream.str();

	return is_valid;
}

bool PftParams::is_valid(const Parameters& params)const{
	// Just call the other overload function and dump the messages.
	std::string dump;
	return is_valid(params, dump);
}
