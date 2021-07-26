// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Combined herbivore + habitat output data.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_COMBINED_DATA_H
#define FAUNA_OUTPUT_COMBINED_DATA_H

#include "habitat_data.h"
#include "herbivore_data.h"

namespace Fauna {
namespace Output {
/// Output data for herbivores and habitat(s).
/**
 * This can be data for one \ref Fauna::SimulationUnit (possibly aggregated
 * over a period of time) or for a set of spatial units (aggregated over
 * time and space).
 * \see \ref sec_design_output_classes
 */
struct CombinedData {
  /// How many data points are merged in this object.
  unsigned int datapoint_count = 0;

  /// Habitat output data.
  HabitatData habitat_data;

  /// Herbivore output data aggregated by output group.
  /**
   * An “output group” is typically an HFT.
   * \see \ref HerbivoreInterface::get_output_group()
   */
  std::map<const std::string, HerbivoreData> hft_data;

  /// Merge other data into this object.
  /**
   * Use this to aggregate (=build averages) over space and time.
   * \ref datapoint_count is used to weigh the values in
   * average-building.
   *
   * For herbivore data (\ref hft_data), the merge routine creates an empty
   * \ref HerbivoreData object as a ‘stub’ if the HFT is found in one of the
   * merge partners, but not in the other one. This way, the averages are built
   * correctly across habitats even if in one habitat, there are no herbivores
   * of one type.
   *
   * This does no calculations if the partners are the same object, or
   * \ref datapoint_count is zero in one of the two objects.
   * \return This object after merging.
   *
   * \see \ref HerbivoreData::merge()
   * \see \ref HabitatData::merge()
   */
  CombinedData& merge(const CombinedData&);

  /// Retrieve aggregated data and reset object.
  CombinedData reset() {
    // copy old object
    CombinedData result = *this;
    // reset with copy assignment operator
    this->operator=(CombinedData());
    return result;
  }
};
}  // namespace Output
}  // namespace Fauna

#endif  // FAUNA_OUTPUT_COMBINED_DATA_H
