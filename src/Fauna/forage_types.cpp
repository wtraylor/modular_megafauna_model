//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Definition of forage types (grass, browse, etc.)
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
//////////////////////////////////////////////////////////////////////////
#include "forage_types.h"
#include <stdexcept>

using namespace Fauna;

namespace {
std::set<ForageType> get_all_forage_types() {
  std::set<ForageType> result;
  result.insert(FT_GRASS);
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
    case FT_GRASS:
      static const std::string grass("grass");
      return grass;
    case FT_INEDIBLE:
      static const std::string inedible("inedible");
      return inedible;
    default:
      throw std::logic_error(
          "Fauna::get_forage_type_name() "
          "Forage type is not implemented.");
  }
}

