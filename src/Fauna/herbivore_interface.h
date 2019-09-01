#ifndef FAUNA_HERBIVORE_INTERFACE_H
#define FAUNA_HERBIVORE_INTERFACE_H

#include "forage_values.h"

namespace Fauna {

// Forward Declarations
namespace Output {
class HerbivoreData;
}
class HabitatForage;
class Hft;
class HabitatEnvironment;

/// Interface of a herbivore of a specific \ref Hft.
/**
 * Derived classes will define the model mechanics.
 * Each herbivore class has a corresponding implementation of
 * \ref PopulationInterface which creates and manages the
 * herbivores.
 * \see \ref sec_new_herbivore_class
 * \see \ref PopulationInterface
 * \see \ref sec_herbivoredesign
 */
struct HerbivoreInterface {
  /// Virtual Destructor
  /** Destructor must be virtual in an interface. */
  virtual ~HerbivoreInterface() {}

  /// Feed the herbivore dry matter forage.
  /**
   * \param kg_per_km2 Dry matter forage mass [kgDM/km²].
   * \param N_kg_per_km2 Nitrogen in the forage [kgN/km²].
   * \param digestibility Proportional forage digestibility.
   * \throw std::logic_error If `forage` exceeds
   * intake constraints of maximal foraging and digestion.
   * \throw std::logic_error If this herbivore is dead or
   * has no individuals.
   */
  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)) = 0;

  /// Body mass of one individual [kg/ind].
  virtual double get_bodymass() const = 0;

  /// Get the forage the herbivore would like to eat today.
  /**
   * Call this after \ref simulate_day().
   * \note This may be called multiple times a day in order to allow
   * switching to another forage type!
   *
   * \param available_forage Available forage in the habitat
   * [kgDM/km²].
   * \return Dry matter forage *per m²* that the herbivore
   * would eat without any food competition [kgDM/km²].
   */
  virtual ForageMass get_forage_demands(
      const HabitatForage& available_forage) = 0;

  /// Get the herbivore functional type
  virtual const Hft& get_hft() const = 0;

  /// Individuals per km²
  virtual double get_ind_per_km2() const = 0;

  /// Get herbivore biomass density [kg/km²]
  virtual double get_kg_per_km2() const = 0;

  /// Read current output.
  virtual const Output::HerbivoreData& get_todays_output() const = 0;

  /// Whether the herbivore object is dead.
  virtual bool is_dead() const = 0;

  /// Mark this herbivore as dead (see \ref is_dead()).
  /** This function is needed in order to take nitrogen back from “dead”
   * herbivores before removing them from memory. */
  virtual void kill() = 0;

  /// Simulate daily events.
  /**
   * Call this before \ref get_forage_demands().
   * \param[in] day Current day of year, 0=Jan. 1st.
   * \param[in] environment Current environmental conditions in the habitat.
   * \param[out] offspring Number of newborn today [ind/km²].
   * \throw std::invalid_argument If `day` not in [0,364].
   * \throw std::logic_error If this herbivore is dead.
   */
  virtual void simulate_day(const int day,
                            const HabitatEnvironment& environment,
                            double& offspring) = 0;

  /// Get how much nitrogen is excreted, and reset.
  /**
   * Through feeding, plant nitrogen is taken up. Any nitrogen that has
   * been excreted again can be queried with this function. This function
   * also resets the accumulated nitrogen to zero.
   * This way, the nitrogen cycle from plant to animal and back is
   * completely closed.
   *
   * If the herbivore is dead, *all* remaining nitrogen in the body
   * (including tissue) ought to be returned.
   *
   * Sidenote: The function name doesn’t start with `get_` because the
   * function changes the internal state of the object.
   * \return Excreted nitrogen [kgN/km²] (+ tissue nitrogen if dead).
   * \see \ref sec_nitrogen_excretion
   */
  virtual double take_nitrogen_excreta() = 0;
};

}  // namespace Fauna

#endif  // FAUNA_HERBIVORE_INTERFACE_H
