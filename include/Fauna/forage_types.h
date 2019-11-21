/**
 * \file
 * \brief Definition of forage types (grass, browse, etc.)
 * \copyright ...
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
  Inedible
};

/// Set with all enum entries of \ref ForageType.
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
