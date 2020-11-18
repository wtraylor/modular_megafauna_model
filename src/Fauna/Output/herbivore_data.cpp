// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Herbivore output data.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "herbivore_data.h"
using namespace Fauna::Output;

namespace {
/// Mortality–value map.
typedef std::map<Fauna::MortalityFactor, double> MortMap;
}  // namespace

HerbivoreData& HerbivoreData::merge(const HerbivoreData& other,
                                    const double this_weight,
                                    const double other_weight) {
  if (!(this_weight >= 0.0))
    throw std::invalid_argument(
        "Fauna::Output::HerbivoreData::merge() "
        "Parameter `this_weight` is not a >=0.0");
  if (!(other_weight >= 0.0))
    throw std::invalid_argument(
        "Fauna::Output::HerbivoreData::merge() "
        "Parameter `other_weight` is not a >=0.0");
  if (this_weight == 0.0 && other_weight == 0.0)
    throw std::invalid_argument(
        "Fauna::Output::HerbivoreData::merge() "
        "Both objects have zero weight");

  // If objects are identical, do nothing.
  if (&other == this) return *this;

  // Don’t do any calculations if one partner is weighed with zero.
  if (other_weight == 0.0) return *this;
  if (this_weight == 0.0) {
    *this = other;  // copy all values
    return *this;
  }

  // ------------------------------------------------------------------
  // PER INDIVIDUAL
  // Here, we weigh additionally with individual density.
  if (other.inddens > 0.0) {
    const double this_weight_ind = this_weight * inddens;
    const double other_weight_ind = other_weight * other.inddens;

    age_years =
        average(age_years, other.age_years, this_weight_ind, other_weight_ind);
    bodyfat =
        average(bodyfat, other.bodyfat, this_weight_ind, other_weight_ind);
    eaten_nitrogen_per_ind =
        average(eaten_nitrogen_per_ind, other.eaten_nitrogen_per_ind,
                this_weight, other_weight);
    expenditure = average(expenditure, other.expenditure, this_weight_ind,
                          other_weight_ind);
  }

  // ------------------------------------------------------------------
  // PER HABITAT VARIABLES
  // Here, we weigh just with the given weights.

  // Only use those mortality factors that are included in
  // *both* maps.
  MortMap mort_intersect;
  for (MortMap::const_iterator itr = other.mortality.begin();
       itr != other.mortality.end(); itr++) {
    const MortalityFactor& mf = itr->first;
    // find the mortality factor of the other object in this object.
    MortMap::iterator found = this->mortality.find(mf);
    // If we find it, we build the average.
    if (found != this->mortality.end())
      mort_intersect[mf] =
          average(found->second, itr->second, this_weight, other_weight);
  }
  this->mortality = mort_intersect;

  bound_nitrogen =
      average(bound_nitrogen, other.bound_nitrogen, this_weight, this_weight);
  inddens = average(inddens, other.inddens, this_weight, other_weight);
  massdens = average(massdens, other.massdens, this_weight, other_weight);
  offspring = average(offspring, other.offspring, this_weight, other_weight);

  // Delegate average building to class ForageValues.
  eaten_forage_per_ind.merge(other.eaten_forage_per_ind, this_weight,
                             other_weight);
  eaten_forage_per_mass.merge(other.eaten_forage_per_mass, this_weight,
                              other_weight);
  energy_intake_per_ind.merge(other.energy_intake_per_ind, this_weight,
                              other_weight);
  energy_intake_per_mass.merge(other.energy_intake_per_mass, this_weight,
                               other_weight);

  merge_energy_content(this->energy_content, other.energy_content, this_weight,
                       other_weight);

  return *this;
}

void HerbivoreData::merge_energy_content(Fauna::ForageEnergyContent& obj1,
                                         const Fauna::ForageEnergyContent& obj2,
                                         const double weight1,
                                         const double weight2) {
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    if (obj2.get(*ft) != 0.0)
      obj1.set(*ft, average(obj1[*ft], obj2[*ft], weight1, weight2));
  }
}

HerbivoreData HerbivoreData::create_datapoint(
    const std::vector<HerbivoreData> vec) {
  if (vec.empty())
    throw std::invalid_argument(
        "Fauna::Output::HerbivoreData::create_datapoint() "
        "Received empty vector as argument.");

  HerbivoreData result;

  // Iterate through all herbivore data items and add them to `result`.
  for (std::vector<HerbivoreData>::const_iterator itr = vec.begin();
       itr != vec.end(); itr++) {
    const HerbivoreData& other = *itr;

    if (result.inddens > 0.0 || other.inddens > 0.0) {
      // AVERAGE building for per-individual variables
      // All numbers are weighed by the individual density.
      result.age_years = average(result.age_years, other.age_years,
                                 result.inddens, other.inddens);
      result.bodyfat =
          average(result.bodyfat, other.bodyfat, result.inddens, other.inddens);
      result.eaten_nitrogen_per_ind =
          average(result.eaten_nitrogen_per_ind, other.eaten_nitrogen_per_ind,
                  result.inddens, other.inddens);
      result.expenditure = average(result.expenditure, other.expenditure,
                                   result.inddens, other.inddens);

      result.eaten_forage_per_ind.merge(other.eaten_forage_per_ind);
      result.eaten_forage_per_mass.merge(other.eaten_forage_per_mass);
      merge_energy_content(result.energy_content, other.energy_content);
      result.energy_intake_per_ind.merge(other.energy_intake_per_ind);
      result.energy_intake_per_mass.merge(other.energy_intake_per_mass);

      // Include *all* mortality factors.
      for (MortMap::const_iterator itr = other.mortality.begin();
           itr != other.mortality.end(); itr++) {
        result.mortality[itr->first] =
            average(result.mortality[itr->first], itr->second, result.inddens,
                    other.inddens);
      }
    }

    // SUM building for per-area and per-habitat variables
    result.bound_nitrogen += other.bound_nitrogen;
    result.inddens += other.inddens;
    result.massdens += other.massdens;
    result.offspring += other.offspring;
  }

  return result;
}
