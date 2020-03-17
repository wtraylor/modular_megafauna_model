/**
 * \file
 * \brief Abstract class for any population of herbivores.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_POPULATION_INTERFACE_H
#define FAUNA_POPULATION_INTERFACE_H

#include "herbivore_vector.h"

namespace Fauna{
/// A container of herbivore objects.
/**
 * Manages a set of \ref HerbivoreInterface instances. What makes a
 * "population" may be defined by the derived class. For instance, herbivore
 * cohorts have one population per HFT in each simulation unit.
 *
 * A population also instantiates all new objects of herbivore classes
 * (derived from \ref HerbivoreInterface) in a simulation.
 * \note This is strictly speaking no “interface” anymore since not all
 * of its functions are pure abstract. It is just unnecessary effort to
 * change the name.
 */
struct PopulationInterface {
  /// Virtual destructor
  /** Destructor must be virtual in an interface. */
  virtual ~PopulationInterface() {}

  /// Give birth to new herbivores
  /**
   * The new herbivores are owned by this population object.
   * \param ind_per_km2 Offspring amount [ind/km²].
   * \throw std::invalid_argument If `offspring<0.0`.
   */
  virtual void create_offspring(const double ind_per_km2) = 0;

  /// Create a set of new herbivores to establish a population.
  /** \throw std::logic_error If this population is not empty. */
  virtual void establish() = 0;

  /// Get individual density of all herbivores together [ind/km²].
  virtual const double get_ind_per_km2() const;

  /// Get mass density of all herbivores together [kg/km²].
  virtual const double get_kg_per_km2() const;

  /// Get pointers to the herbivores (including dead ones).
  /**
   * \warning The pointers are not guaranteed to stay valid
   * on changing the population in \ref create_offspring() or
   * \ref establish().
   * \return Pointers to all living herbivores in the population.
   * Guaranteed no NULL pointers.
   */
  virtual ConstHerbivoreVector get_list() const = 0;

  /** \copydoc get_list()const */
  virtual HerbivoreVector get_list() = 0;

  /// Mark all herbivores as dead (see \ref HerbivoreInterface::kill()).
  virtual void kill_all();

  /// Mark those herbivores as dead that are not viable.
  /**
   * The first step is to mark non-viable herbivores as dead (see
   * \ref HerbivoreInterface::kill()). In the second step, dead herbivores are
   * removed. It is necessary to split this up so that the simulation framework
   * can do something with dead herbivores before they are deleted in \ref
   * purge_of_dead(). For example this may be returning nitrogen.
   */
  virtual void kill_nonviable() = 0;

  /// Delete all dead herbivores.
  /** \see \ref HerbivoreInterface::is_dead() */
  virtual void purge_of_dead() = 0;
};

}

#endif // FAUNA_POPULATION_INTERFACE_H
