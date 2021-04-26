// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Construct new herbivore cohort objects.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef CREATE_HERBIVORE_COHORT
#define CREATE_HERBIVORE_COHORT

#include "create_herbivore_common.h"
#include "herbivore_cohort.h"

namespace Fauna {

/// Function class constructing \ref HerbivoreCohort objects.
class CreateHerbivoreCohort : public CreateHerbivoreCommon {
 public:
  /// Constructor
  /** \copydoc CreateHerbivoreCommon::CreateHerbivoreCommon() */
  CreateHerbivoreCohort(const std::shared_ptr<const Hft> hft,
                        const std::shared_ptr<const Parameters> parameters)
      : CreateHerbivoreCommon(hft, parameters) {}

  /// Create a new object instance
  /**
   * \param ind_per_km2 Individual density of the new cohort.
   * \param age_years Age in years.
   * \param sex Gender of the herbivore.
   * \return New object instance.
   * \throw std::invalid_argument if `ind_per_km2<=0.0` or
   * `age_years<0`
   */
  virtual HerbivoreCohort operator()(const double ind_per_km2,
                                     const int age_years, Sex sex) const;
};
}  // namespace Fauna

#endif  // CREATE_HERBIVORE_COHORT
