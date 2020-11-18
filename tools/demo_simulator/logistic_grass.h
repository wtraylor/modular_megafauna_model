// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Simplistic grass model for demonstration purpose.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_DEMO_LOGISTIC_GRASS_H
#define FAUNA_DEMO_LOGISTIC_GRASS_H
#include <string>
#include <vector>
#include "megafauna.h"

namespace Fauna {
namespace Demo {

/// Helper class for performing simple grass growth to test herbivore
/// functionality
class LogisticGrass {
 public:
  /// Settings for grass growth
  /**
   * The initialization values are just arbitrary.
   */
  struct Parameters {
    /// Proportional daily rates of grass decay [day^-1]
    /**
     * This is a vector of *daily* decay rates for each month. When
     * the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year.
     */
    std::vector<double> decay_monthly = {0.0};

    /// Proportional digestibility of the grass [frac].
    /**
     * This is a vector of digestibility values for each month. When
     * the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year.
     */
    std::vector<double> digestibility = {0.5};

    /// Percentage of habitat covered with grass (Foliar Perc. Cover) [frac]
    double fpc = 0.1;

    /// Proportional daily grass growth rates [day^-1]
    /**
     * This is a vector of *daily* growth rates for each month. When
     * the end of the vector is reached, the values are recycled.
     * A vector of length 12 creates the same behaviour every year.
     */
    std::vector<double> growth_monthly = {0.0};

    /// Initial available forage [kgDM/km²]
    /** This should be smaller than \ref saturation */
    double init_mass = 0.0;

    /// Ungrazable grass biomass reserve, inaccessable to herbivores [kgDM/km²]
    /** Owen-Smith (2002) gives value of 20 g/m²*/
    double reserve = 0.1;

    /// Saturation grass biomass [kgDM/m²]
    /** Owen-Smith (2002): 200 g/m²*/
    double saturation = 1.0;

    /// Check if parameters are valid
    /**
     * \param[out] msg Possible warnings and error messages.
     * \return true if values are valid, false if not.
     */
    bool is_valid(std::string& msg) const;
  };

  /// Constructor
  /**
   * \throw std::invalid_argument If `settings` are not valid.
   */
  LogisticGrass(const LogisticGrass::Parameters& settings);

  /// Perform grass growth and decay for one day.
  /**
   * \param day_of_year January 1st = 0
   * \throw std::invalid_argument if not `0<=day_of_year<=364`
   */
  void grow_daily(const int day_of_year);

  /// Get current grass forage
  const GrassForage& get_forage() const { return forage; }

  /// Set the grass forage
  void set_forage(const GrassForage& f) { forage = f; }

 private:
  /// Current forage
  /** Excluding the reserve \ref LogisticGrass::Parameters::reserve. */
  GrassForage forage;

  LogisticGrass::Parameters settings;

  /// The current simulation month, starting with zero.
  /** We need this to address the current value in
   * \ref Parameters::growth_monthly and
   * \ref Parameters::decay_monthly.
   */
  int simulation_month;
};

}  // namespace Demo
}  // namespace Fauna

#endif  // FAUNA_DEMO_LOGISTIC_GRASS_H
