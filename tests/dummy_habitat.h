// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Habitat implementation for unit tests.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef TESTS_DUMMY_HABITAT_H
#define TESTS_DUMMY_HABITAT_H

#include "habitat.h"

namespace Fauna {
/// A dummy habitat that does nothing
class DummyHabitat : public Habitat {
 public:
  virtual HabitatForage get_available_forage() const { return HabitatForage(); }
  virtual const char* get_aggregation_unit() const { return "global"; }
  virtual HabitatEnvironment get_environment() const {
    return HabitatEnvironment();
  }
  int get_day_public() const { return get_day(); }
};

}

#endif // TESTS_DUMMY_HABITAT_H
