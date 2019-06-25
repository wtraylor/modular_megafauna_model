////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Instruction file parameters of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
////////////////////////////////////////////////////////////

#include "parameters.h"
#include <sstream>  // for is_valid() messages

using namespace Fauna;

bool Parameters::is_valid(std::string& messages) const {
  bool is_valid = true;

  // The message text is written into an output string stream
  std::ostringstream stream;

  //------------------------------------------------------------
  // add new checks in alphabetical order

  if (herbivore_establish_interval < 0) {
    stream << "herbivore_establish_interval must be >=0" << std::endl;
    is_valid = false;
  }

  if (herbivore_type == HT_INDIVIDUAL)
    if (habitat_area_km2 <= 0.0) {
      stream << "habitat_area_km2 must be >0 with individual herbiovres."
             << std::endl;
      is_valid = false;
    }

  //------------------------------------------------------------

  // convert stream to string
  messages = stream.str();

  return is_valid;
}
