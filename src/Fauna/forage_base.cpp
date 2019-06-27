//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Base class for all forage types.
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
//////////////////////////////////////////////////////////////////////////
#include "forage_base.h"
#include "utils.h"

using namespace Fauna;

ForageBase& ForageBase::merge_base(const ForageBase& other,
                                   const double this_weight,
                                   const double other_weight) {
  if (this == &other) return *this;
  set_digestibility(average(this->get_digestibility(),
                            other.get_digestibility(), this_weight,
                            other_weight));
  set_mass(
      average(this->get_mass(), other.get_mass(), this_weight, other_weight));
  return *this;
}

void ForageBase::set_mass(const double dm) {
  if (dm < 0.0)
    throw std::invalid_argument(
        "Fauna::ForageBase::set_mass(): "
        "Dry matter is smaller than zero.");
  if (get_nitrogen_mass() > dm)
    throw std::logic_error(
        "Fauna::ForageBase::set_mass() "
        "Dry matter is set to a value smaller than "
        "nitrogen mass. Decrease nitrogen mass first.");
  dry_matter_mass = dm;
}

void ForageBase::set_nitrogen_mass(const double n_mass) {
  if (n_mass < 0.0)
    throw std::invalid_argument(
        "Fauna::ForageBase::set_nitrogen_mass(): "
        "Nitrogen mass is smaller than zero.");
  if (n_mass > get_mass())
    throw std::logic_error(
        "Fauna::ForageBase::set_mass() "
        "Nitrogen mass is set to a value greater than "
        "the dry matter mass. Increase dry matter first.");
  nitrogen_mass = n_mass;
}
