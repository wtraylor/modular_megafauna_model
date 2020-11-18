// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Different models how to distribute available forage among herbivores.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_FORAGE_DISTRIBUTION_ALGORITHMS_H
#define FAUNA_FORAGE_DISTRIBUTION_ALGORITHMS_H

#include "forage_values.h"

namespace Fauna {
// Forward Declarations
class HabitatForage;

/// Interface for a forage distribution algorithm
/** \see \ref sec_strategy */
struct DistributeForage {
  /// Distribute forage equally among herbivores
  /**
   * No direct competition.
   * Under forage scarcity, each herbivore gets its share in
   * proportion to its demanded forage.
   * \param[in]  available Available forage in the habitat.
   * \param[in,out] forage_distribution As input: Demanded
   * forage of each herbivore
   * (see \ref HerbivoreInterface::get_forage_demands()).
   * As output: Forage portion for each herbivore.
   * Unit is kgDM/km².
   * The sum of all portions must not exceed the available
   * forage!
   */
  virtual void operator()(const HabitatForage& available,
                          ForageDistribution& forage_distribution) const = 0;

  /// Virtual destructor.
  /** The virtual destructor is necessary so that the object is properly
   * released from memory.*/
  virtual ~DistributeForage() {}
};

/// Equal forage distribution algorithm
/**
 * No direct competition.
 * Under forage scarcity, each herbivore gets its share in
 * proportion to its demanded forage.
 *
 * Under food scarcity, the following equation holds:
 * \f[
 * \frac{P_{ind}}{A} = \frac{D_{ind}}{D_{total}}
 * \f]
 * - \f$P_{ind}\f$: individual portion of one herbivore
 * - \f$D_{ind}\f$: forage demanded by that herbivore
 * - A: total available forage
 * - \f$D_{total}\f$: sum of all forage demands
 *
 * \note There could occur precision errors leading to the sum of
 * forage being greater than what is available.
 * To counteract this, only a fraction of 99.9\% of the available
 * forage gets actually distributed.
 */
struct DistributeForageEqually : public DistributeForage {
  virtual void operator()(const HabitatForage& available,
                          ForageDistribution& forage_distribution) const;
};

}  // namespace Fauna
#endif  // FAUNA_FORAGE_DISTRIBUTION_ALGORITHMS_H
