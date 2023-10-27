// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Minimal habitat implementation for demonstration purpose.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_DEMO_SIMPLE_HABITAT_H
#define FAUNA_DEMO_SIMPLE_HABITAT_H

#include <vector>

#include "logistic_grass.h"

using namespace Fauna;

namespace Fauna {
namespace Demo {

/// A herbivore habitat independent of the LPJ-GUESS framework for testing.
class SimpleHabitat : public Habitat {
 public:
  /// Simulation parameters for a \ref SimpleHabitat object.
  struct Parameters {
    /// Parameters for logistic grass growth.
    LogisticGrass::Parameters grass;

    /// Air temperature [°C] for each month.
    /** When the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year. */
    std::vector<double> air_temperature = {0.0};
  };

  /// Constructor with simulation settings.
  /**
   * \param settings Simulation settings for the vegetation model.
   * \param aggregation_unit An arbitrary name for grouping the habitat
   * for output.
   */
  SimpleHabitat(const SimpleHabitat::Parameters settings,
                const std::string aggregation_unit)
      : settings(settings),
        grass(settings.grass),
        aggregation_unit(aggregation_unit) {}

 public:  // ------ Fauna::Habitat implementations ----
  virtual void add_excreted_nitrogen(const double) {}  // disabled
  virtual const char* get_aggregation_unit() const {
    return aggregation_unit.c_str();
  }
  virtual HabitatForage get_available_forage() const {
    HabitatForage result;
    result.grass = grass.get_forage();
    return result;
  }
  virtual HabitatEnvironment get_environment() const;
  virtual void init_day(const int today);
  virtual void remove_eaten_forage(const ForageMass& eaten_forage);

 protected:
  /// Perform daily growth.
  /** \param day_of_year January 1st = 0 */
  virtual void grow_daily(const int day_of_year) {
    grass.grow_daily(day_of_year);
  }

 private:
  const std::string aggregation_unit;
  const SimpleHabitat::Parameters settings;

  /// Air temperature in °C, as read from
  /// \ref SimpleHabitat::Parameters::air_temperature.
  double air_temperature = 0.0;

  /// Grass in the habitat
  LogisticGrass grass;

  /// The current simulation month, starting with zero.
  /** We need this to address the current value in
   * \ref SimpleHabitat::Parameters::air_temperature. */
  int simulation_month = 0;
};

}  // namespace Demo
}  // namespace Fauna

#endif  // FAUNA_DEMO_SIMPLE_HABITAT_H
