// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
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

  // Groups can be for instance HFTs.
  typedef std::map<const std::string, HerbivoreData> GroupMap;

  // First, create empty HerbivoreData objects for all HFTs/groups from the
  // *other* object that are not yet present in *this* object.
  for (const auto& itr : other.hft_data) {
    // Create new, empty object if the group is new to this object.
    if (!hft_data.count(itr.first)) hft_data[itr.first] = HerbivoreData();
  }

  // Now we have all groups in this->hft_data, and we can merge for each
  // group/HFT.
  for (auto& itr : hft_data) {
    // If an HFT/group in this->hft_data is not in other.hft_data, we create a
    // temporary empty HerbivoreData object.
    const auto& found = other.hft_data.find(itr.first);
    HerbivoreData other_herbi_data;  // Temporary empty object for now.
    // If we did find the HFT/group in the other object, we use that for
    // merging.
    if (found != other.hft_data.end()) other_herbi_data = found->second;

    // Let the class HerbivoreData do the actual merge.
    itr.second.merge(other_herbi_data, this->datapoint_count,
                     other.datapoint_count);
  }

  // increment datapoint counter
  this->datapoint_count += other.datapoint_count;

  return *this;
}
