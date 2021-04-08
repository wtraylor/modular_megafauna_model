// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief The forage class for grass.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "grass_forage.h"
#include "average.h"

using namespace Fauna;

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
