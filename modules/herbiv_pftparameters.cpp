///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivory-related PFT parameters
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "guess.h"
#include "herbiv_pftparameters.h"
#include <sstream> // for is_valid() messages

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

	// convert stream to string
	messages = stream.str();

	return is_valid;
}

