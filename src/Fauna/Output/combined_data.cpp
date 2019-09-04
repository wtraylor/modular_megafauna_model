/**
 * \file
 * \brief Combined herbivore + habitat output data.
 * \copyright ...
 * \date 2019
 */
#include "combined_data.h"
#include "grass_forage.h"
#include "hft.h"

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
  // Merge herbivore data for each HFT.

  typedef std::map<const Fauna::Hft*, HerbivoreData> HftMap;

  // First, create empty HerbivoreData objects for all HFTs that are not
  // yet present in this object.
  for (HftMap::const_iterator itr = other.hft_data.begin();
       itr != other.hft_data.end(); itr++) {
    // create new object if HFT not found.
    if (!hft_data.count(itr->first)) hft_data[itr->first] = HerbivoreData();
  }

  // Second, merge all herbivore data.
  for (HftMap::iterator itr = hft_data.begin(); itr != hft_data.end(); itr++) {
    // try to find this HFT in the other object
    HftMap::const_iterator found = other.hft_data.find(itr->first);

    // If the other object doesn’t contain this HFT, we use an empty
    // object.
    HerbivoreData other_herbi_data;
    if (found != other.hft_data.end()) other_herbi_data = found->second;

    // Let the class HerbivoreData do the actual merge.
    itr->second.merge(other_herbi_data, this->datapoint_count,
                      other.datapoint_count);
  }

  // increment datapoint counter
  this->datapoint_count += other.datapoint_count;

  return *this;
}
