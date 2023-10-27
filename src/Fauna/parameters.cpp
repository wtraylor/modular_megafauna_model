// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Global parameters for the megafauna library.
 * \copyright LGPL-3.0-or-later
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

  if (one_hft_per_habitat && (herbivore_type != HerbivoreType::Cohort)) {
    stream << "'simulation.one_hft_per_habitat' is only defined for "
              "'simulation.herbivore_type = \"cohort\"'."
           << std::endl;
    is_valid = false;
  }

  if (herbivore_establish_interval < 0) {
    stream << "herbivore_establish_interval must be >=0" << std::endl;
    is_valid = false;
  }

  for (const auto ft : FORAGE_TYPES)
    if (forage_gross_energy[ft] == 0.0)
      stream << "forage.gross_energy." << get_forage_type_name(ft)
             << " is zero. "
             << "This forage type will effectively be deactivated."
             << std::endl;

  if (output_text_tables.mass_density_per_hft)
    stream
        << "The option 'mass_density_per_hft' in 'output.text_tables.tables' "
           "is deprecated. Use 'mass_density' instead."
        << std::endl;

  //------------------------------------------------------------

  // convert stream to string
  messages = stream.str();

  return is_valid;
}
