// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Definition of forage types (grass, browse, etc.)
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "forage_types.h"

#include <stdexcept>

using namespace Fauna;

namespace {
std::set<ForageType> get_all_forage_types() {
  std::set<ForageType> result;
  result.insert(ForageType::Grass);
  // ADD NEW FORAGE TYPES HERE
  return result;
}
}  // namespace

// define global constant
namespace Fauna {
const std::set<ForageType> FORAGE_TYPES = get_all_forage_types();
}

const std::string& Fauna::get_forage_type_name(const ForageType ft) {
  switch (ft) {
    case ForageType::Grass:
      static const std::string grass("grass");
      return grass;
    case ForageType::Inedible:
      static const std::string inedible("inedible");
      return inedible;
    default:
      throw std::logic_error(
          "Fauna::get_forage_type_name() "
          "Forage type is not implemented.");
  }
}
