//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes for the spatial units where herbivores live.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
//////////////////////////////////////////////////////////////////////////
#ifndef HABITAT_H
#define HABITAT_H

#include <cassert>           // for assert()
#include <list>              // for HabitatList
#include <memory>            // for std::auto_ptr
#include "habitat_data.h"    // for HabitatData
#include "habitat_forage.h"  // for HabitatForage

namespace Fauna {
// Forward declaration of classes in the same namespace
class HabitatEnvironment;

/// Abstract class of a homogenous spatial unit populated by herbivores
/**
 * \note While this base class implements the basic output
 * functions, any derived class is responsible to add its
 * own output.
 * \see \ref Fauna::SimulationUnit
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
#endif  // HABITAT_H
