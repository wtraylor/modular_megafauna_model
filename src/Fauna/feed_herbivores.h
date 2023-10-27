// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Function object to feed herbivores.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_FEED_HERBIVORES_H
#define FAUNA_FEED_HERBIVORES_H

#include <memory>

#include "herbivore_vector.h"

namespace Fauna {
// Forward Declarations
class DistributeForage;
class HabitatForage;

/// Function object to feed herbivores.
class FeedHerbivores {
 public:
  /// Constructor.
  /**
   * \param distribute_forage Strategy object for calculating the forage
   * portions. This must be a newly created object. It will be owned by the
   * FeedHerbivores object.
   * \throw std::invalid_argument If `distribute_forage==NULL`. */
  FeedHerbivores(DistributeForage* distribute_forage);

  /// Delete copy constructor because of pointer ownership.
  FeedHerbivores(FeedHerbivores const&) = delete;

  /// Delete copy assignment because of pointer ownership.
  void operator=(FeedHerbivores const&) = delete;

  /// Destructor.
  ~FeedHerbivores();

  /// Feed the herbivores.
  /**
   * \param[in,out] available Available forage mass in the
   * habitat. This will be reduced by the amount of eaten
   * forage.
   * \param[in,out] herbivores Herbivore objects that are
   * being fed by calling \ref HerbivoreInterface::eat().
   */
  void operator()(HabitatForage& available,
                  const HerbivoreVector& herbivores) const;

 private:
  std::unique_ptr<DistributeForage> distribute_forage;
};

}  // namespace Fauna

#endif  // FAUNA_FEED_HERBIVORES_H
