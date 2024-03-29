// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Determine erbivore breeding season.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_BREEDING_SEASON_H
#define FAUNA_BREEDING_SEASON_H

namespace Fauna {

/// Helper class for reproduction within a breeding season.
class BreedingSeason {
 public:
  /// Constructor.
  /**
   * \param breeding_season_start The day of the year (0=Jan 1st)
   * at which the breeding season starts.
   * \param breeding_season_length Length of the breeding season
   * in days [1--365].
   */
  BreedingSeason(const int breeding_season_start,
                 const int breeding_season_length);

  /// Whether given day (0=Jan 1st) is in the breeding season.
  /**
   * \throw std::invalid_argument If `day` is not in [0,364].
   */
  bool is_in_season(const int day) const;

  /// Convert an annual reproduction rate to a daily one in season.
  double annual_to_daily_rate(const double annual) const;

  /// Whether all member variables are the same in the other object.
  bool operator==(const BreedingSeason& other) const {
    return start == other.start && length == other.length;
  }

 private:
  int start;   // day of year (0=Jan 1st)
  int length;  // number of days
};

}  // namespace Fauna

#endif  // FAUNA_BREEDING_SEASON_H
