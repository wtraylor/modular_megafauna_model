////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Instruction file parameters of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_parameters.h" 
#include <sstream> // for is_valid() messages

using namespace Fauna;

bool Parameters::is_valid(std::string& messages)const{
	bool is_valid = true;
	
	// The message text is written into an output string stream
	std::ostringstream stream;

	//------------------------------------------------------------
	// add new checks in alphabetical order

	if (herbivore_type==HT_COHORT && dead_herbivore_threshold <= 0){
		stream << "dead_herbivore_threshold must be >0" << std::endl;
		is_valid = false;
	}

	if (free_herbivory_years < 0){
		stream << "free_herbivory_years must be >= 0" << std::endl;
		is_valid = false;
	}

	//------------------------------------------------------------

	// convert stream to string
	messages = stream.str();

	return is_valid;
}

