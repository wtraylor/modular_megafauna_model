// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief The spatial unit where herbivores live in.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_HABITAT_H
#define FAUNA_HABITAT_H

#include <cassert>
#include <list>
#include <memory>
#include "Fauna/Output/habitat_data.h"

namespace Fauna {
// Forward Declarations
struct HabitatEnvironment;

/// Abstract class of a homogenous spatial unit populated by herbivores
/**
 * \note While this base class implements the basic output
 * functions, any derived class is responsible to add its
 * own output.
 */
class Habitat {
 public:
  /// Virtual Destructor
  /** Destructor must be virtual in an interface. */
  virtual ~Habitat() {}

  /// Account for nitrogen cycling back to soil (faeces + carcasses).
  /**
   * \param kgN_per_km2 Nitrogen deposited in habitat [kgN/km²].
   * \throw std::invalid_argument If `kgN_per_km2 < 0.0`.
   * \throw std::logic_error If this object is dead.
   */
  virtual void add_excreted_nitrogen(const double kgN_per_km2) = 0;

  /// A string identifier for the group of habitats whose output is aggregated.
  /**
   * Suppose the vegetation model works in longitude/latitude grid cells and
   * has three habitats in each grid cell. Output shall be aggregated per grid
   * cell. Then all habitats in each set of three have the same (unique!)
   * aggregation unit sting identifier. This could be for instance "10.0/54.0"
   * for a grid cell at 10° E and 54° N. It’s completely up to the vegetation
   * model to define a convention for the aggregation unit identifiers.
   *
   * You should avoid leading or trailing whitespaces and take care that you
   * don’t include a character that is used as a field separator in the output
   * table. Also, the string should not be empty.
   *
   * The output of this function should not change within the lifetime of one
   * class instance: One Habitat object shall not change into another
   * aggregation unit.
   */
  virtual const char* get_aggregation_unit() const = 0;

  /// Get dry-matter biomass [kgDM/km²] that is available to herbivores to eat.
  /**
   * \throw std::logic_error If this object is dead.
   */
  virtual HabitatForage get_available_forage() const = 0;

  /// Get today’s abiotic environmental variables in the habitat.
  /**
   * \throw std::logic_error If this object is dead.
   */
  virtual HabitatEnvironment get_environment() const = 0;

  /// Update at the start of the day.
  /**
   * Call this once every day from the framework.
   * When overwriting this in derived classes, make sure to
   * call this parent function first.
   * \param today Day of the year (0 ≙ Jan 1st).
   * \throw std::invalid_argument If not `0<=today<=364`.
   * \throw std::logic_error If this object is dead.
   */
  virtual void init_day(const int today);

  /// Whether \ref kill() has been called on this object.
  bool is_dead() const { return killed; }

  /// Mark the object as dead and to be deleted.
  /**
   * Call this when the corresponding vegetation unit is invalid.
   */
  void kill() { killed = true; }

  /// Remove forage eaten by herbivores.
  /**
   * The base class implements only adding the eaten forage
   * to the output. Any derived class should call this (the parent‘s)
   * function and do forage removal afterwards.
   * \param eaten_forage Dry matter leaf forage [kgDM/km²],
   * must not exceed available forage.
   * \throw std::logic_error If `eaten_forage` exceeds
   * available forage (**to be implemented in derived classes**).
   * \throw std::logic_error If this object is dead.
   */
  virtual void remove_eaten_forage(const ForageMass& eaten_forage) {
    get_todays_output().eaten_forage += eaten_forage;
  }

  /// The current day as set by \ref init_day().
  /** \see Date::day */
  int get_day() const { return day_of_year; }

  /// The current output data (read-only).
  const Output::HabitatData& get_todays_output() const {
    return current_output;
  }

 protected:
  /// Class-internal read/write access to current output data.
  Output::HabitatData& get_todays_output() { return current_output; }

 private:
  Output::HabitatData current_output;
  int day_of_year;
  bool killed = false;
};

/// A list of \ref Habitat pointers.
typedef std::list<const Habitat*> HabitatList;
}  // namespace Fauna
#endif  // FAUNA_HABITAT_H
