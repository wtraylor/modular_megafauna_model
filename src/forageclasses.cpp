//////////////////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Large herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
//////////////////////////////////////////////////////////////////////////

#include "forageclasses.h"
#include <cassert>  // for assert()
#include "config.h"

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

ForageValues<POSITIVE_AND_ZERO> Fauna::foragefractions_to_foragevalues(
    const ForageFraction& fractions) {
  ForageValues<POSITIVE_AND_ZERO> result;
  for (ForageFraction::const_iterator i = fractions.begin();
       i != fractions.end(); i++)
    result.set(i->first, i->second);
  return result;
}

ForageFraction Fauna::foragevalues_to_foragefractions(
    const ForageValues<POSITIVE_AND_ZERO> values, const double tolerance) {
  if (tolerance < 0.0)
    throw std::invalid_argument(
        "Fauna::foragevalues_to_foragefractions() "
        "Parameter `tolerance` is negative.");

  ForageFraction result;
  for (ForageFraction::const_iterator i = values.begin(); i != values.end();
       i++) {
    double v = i->second;
    if (v > 1.0) {
      if (v <= 1.0 + tolerance)
        v = 1.0;
      else
        throw std::invalid_argument(
            "Fauna::foragevalues_to_foragefractions() "
            "One forage value exceeds 1.0 and cannot be converted to "
            "a fraction.");
    }

    result.set(i->first, v);
  }
  return result;
}

ForageFraction Fauna::convert_mj_to_kg_proportionally(
    const ForageEnergyContent& mj_per_kg,
    const ForageFraction& mj_proportions) {
  ForageValues<POSITIVE_AND_ZERO> kg_proportions =
      foragefractions_to_foragevalues(mj_proportions)
          .divide_safely(mj_per_kg, 0.0);

  const double kg_prop_sum = kg_proportions.sum();
  const double mj_prop_sum = mj_proportions.sum();

  if (kg_prop_sum > 0.0) {
    kg_proportions = kg_proportions * (mj_prop_sum / kg_prop_sum);

    return foragevalues_to_foragefractions(kg_proportions, 0.01);
  } else
    return ForageFraction(0.0);
}

//------------------------------------------------------------
// FORAGEBASE
//------------------------------------------------------------

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

//------------------------------------------------------------
// GRASSFORAGE
//------------------------------------------------------------

GrassForage& GrassForage::merge(const GrassForage& other,
                                const double this_weight,
                                const double other_weight) {
  if (this == &other) return *this;
  // merge generic properties
  merge_base(other, this_weight, other_weight);

  // merge grass-specific properties
  // (don’t call get_fpc() here, but use private member variable
  //  directly, so that no validity check is done)
  set_fpc(average(this->fpc, other.fpc, this_weight, other_weight));
  return *this;
}

//------------------------------------------------------------
// HABITATFORAGE
//------------------------------------------------------------

Digestibility HabitatForage::get_digestibility() const {
  Digestibility result;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    result.set(*ft, operator[](*ft).get_digestibility());
  }
  return result;
}

ForageMass HabitatForage::get_mass() const {
  ForageMass result;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    result.set(*ft, operator[](*ft).get_mass());
  }
  return result;
}

ForageFraction HabitatForage::get_nitrogen_content() {
  ForageFraction n_content;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    const ForageBase& f = operator[](*ft);
    if (f.get_mass() != 0.0)
      n_content.set(*ft, f.get_nitrogen_mass() / f.get_mass());
  }
  return n_content;
}

ForageBase HabitatForage::get_total() const {
  // Return object
  ForageBase result;

  const ForageMass mass = get_mass();

  result.set_mass(mass.sum());

  // build weighted average only if total mass is greater zero
  if (result.get_mass() > 0.0) {
    // Create weighted average

    double dig_sum_weight = 0.0;
    // loop through each forage type
    for (ForageMass::const_iterator itr = mass.begin(); itr != mass.end();
         itr++) {
      const ForageType ft = itr->first;
      dig_sum_weight += itr->second * operator[](ft).get_digestibility();
    }
    result.set_digestibility(dig_sum_weight / mass.sum());
  } else
    result.set_digestibility(0.0);
  return result;
}

HabitatForage& HabitatForage::merge(const HabitatForage& other,
                                    const double this_weight,
                                    const double other_weight) {
  if (this == &other) return *this;

  grass.merge(other.grass, this_weight, other_weight);
  // ADD NEW FORAGE TYPES HERE
  return *this;
}

void HabitatForage::set_nitrogen_content(const ForageFraction& n_content) {
  // loop through each forage type
  for (ForageMass::const_iterator itr = n_content.begin();
       itr != n_content.end(); itr++) {
    const ForageType ft = itr->first;
    const double dry_matter = (*this)[ft].get_mass();
    if (n_content[ft] == 1.0)
      throw std::invalid_argument(
          "Fauna::HabitatForage::set_nitrogen_content() "
          "Nitrogen content for one forage type is 100%. That is not allowed.");
    (*this)[ft].set_nitrogen_mass(dry_matter * n_content[ft]);
  }
}
