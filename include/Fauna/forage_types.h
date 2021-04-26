// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Definition of forage types (grass, browse, etc.)
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_FORAGE_TYPES_H
#define FAUNA_FORAGE_TYPES_H

#include <cassert>
#include <set>
#include <string>

namespace Fauna {

/// Different types of forage.
enum class ForageType {
  /// Forage type grass.
  Grass,
  /// Plants that are not edible for herbivores.
  Inedible  // KEEP THIS THE LAST ELEMENT!!
};

/// Number of elements in \ref ForageType.
const int FORAGE_TYPE_COUNT = (int)ForageType::Inedible + 1;

/// Set with all enum entries of \ref ForageType except
/// \ref ForageType::Inedible.
/**
 * This set serves mainly the purpose to iterate easily over
 * all forage types:
 * \code
 * for (const auto& forage_type : FORAGE_TYPES) {
 *   \/\/Do your calculations
 * }
 * \endcode
 */
extern const std::set<ForageType> FORAGE_TYPES;

/// Get a short, lowercase string identifier for a forage type.
/** The names are
 * - unique,
 * - lowercase,
 * - without blank spaces or tabs, newlines etc.,
 * - without comma oder underscore (which might be used to combine column names
 * in the output).
 */
const std::string& get_forage_type_name(const ForageType);

}  // namespace Fauna

#endif  // FAUNA_FORAGE_TYPES_H
