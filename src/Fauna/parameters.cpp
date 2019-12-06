/**
 * \file
 * \brief Global parameters for the megafauna library.
 * \copyright ...
 * \date 2019
 */
#include "parameters.h"
#include <sstream>

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

  if (herbivore_type == HerbivoreType::Individual)
    if (habitat_area_km2 <= 0.0) {
      stream << "habitat_area_km2 must be >0 with individual herbiovres."
             << std::endl;
      is_valid = false;
    }

  for (const auto ft : FORAGE_TYPES)
    if (metabolizable_energy[ft] == 0.0)
      stream << "metabolizable_energy." << get_forage_type_name(ft)
             << " is zero. "
             << "This forage type will effectively be deactivated."
             << std::endl;

  //------------------------------------------------------------

  // convert stream to string
  messages = stream.str();

  return is_valid;
}
