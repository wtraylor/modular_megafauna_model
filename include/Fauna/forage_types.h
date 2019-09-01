//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Definition of forage types (grass, browse, etc.)
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
//////////////////////////////////////////////////////////////////////////
#ifndef FAUNA_FORAGE_TYPES_H
#define FAUNA_FORAGE_TYPES_H

#include <cassert>
#include <set>
#include <string>

namespace Fauna {

/// Different types of forage.
enum ForageType {
  /// Forage type grass.
  FT_GRASS,
  /// Plants that are not edible for herbivores.
  FT_INEDIBLE
};

/// Set with all enum entries of \ref ForageType.
/**
 * This set serves mainly the purpose to iterate easily over
 * all forage types:
 * \code
 * for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
 *      ft != FORAGE_TYPES.end(); ft++)
 * {
 *   const ForageType forage_type = *ft;
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
