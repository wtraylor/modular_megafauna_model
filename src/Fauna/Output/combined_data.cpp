// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Combined herbivore + habitat output data.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "combined_data.h"
#include "grass_forage.h"

using namespace Fauna::Output;

CombinedData& CombinedData::merge(const CombinedData& other) {
  // If objects are identical, do nothing.
  if (&other == this) return *this;

  // Don’t do any calculations if one partner is weighed with zero.
  if (other.datapoint_count == 0) return *this;
  if (this->datapoint_count == 0) {
    *this = other;  // copy all values
    return *this;
  }

  // ------------------------------------------------------------------
  // HABITAT DATA
  habitat_data.merge(other.habitat_data, this->datapoint_count,
                     other.datapoint_count);

  // ------------------------------------------------------------------
  // HERBIVORE DATA
  // Merge herbivore data for each output group.

  typedef std::map<const std::string, HerbivoreData> GroupMap;

  // First, create empty HerbivoreData objects for all groups that are not
  // yet present in this object.
  for (const auto& itr : hft_data) {
    // create new object if group not found.
    if (!hft_data.count(itr.first)) hft_data[itr.first] = HerbivoreData();
  }

  // Second, merge all herbivore data.
  for (auto& itr : hft_data) {
    // try to find this group in the other object
    GroupMap::const_iterator found = other.hft_data.find(itr.first);

    // If the other object doesn’t contain this HFT, we use an empty
    // object.
    HerbivoreData other_herbi_data;
    if (found != other.hft_data.end()) other_herbi_data = found->second;

    // Let the class HerbivoreData do the actual merge.
    itr.second.merge(other_herbi_data, this->datapoint_count,
                     other.datapoint_count);
  }

  // increment datapoint counter
  this->datapoint_count += other.datapoint_count;

  return *this;
}
