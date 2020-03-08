/**
 * \file
 * \brief A list of populations per HFT in a habitat.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_POPULATION_LIST_H
#define FAUNA_POPULATION_LIST_H

#include <list>
#include <memory>
#include <vector>
#include "herbivore_vector.h"

namespace Fauna {
// Forward Declarations
class Hft;
class PopulationInterface;

/// List of \ref PopulationInterface objects per \ref Hft.
/**
 * There is one \ref PopulationInterface object per \ref Hft.
 *
 * \ref PopulationInterface object instances passed to this class are
 * owned by this class and will be deleted in the destructor.
 */
class PopulationList {
 public:
  /// Add a new \ref PopulationInterface object for a HFT
  /**
   * \param new_pop Pointer to a newly created object. The
   * object will be owned by this \ref PopulationList
   * and deleted in the destructor.
   * \throw std::invalid_argument If a population of that HFT already
   * exists.
   * \throw std::invalid_argument If `new_pop==NULL`.
   */
  void add(PopulationInterface* new_pop);

  /// Check whether a population for given HFT is already in the list.
  bool exists(const Hft&) const;

  /// Get reference to the population of one HFT.
  /**
   * \throw std::invalid_argument If HFT not in the list yet.
   */
  PopulationInterface& get(const Hft&);

  /// Get pointers to all alive herbivores of all populations.
  /**
   * \return Pointers to all living herbivores in all
   * populations. Guaranteed no `NULL` pointers.
   */
  HerbivoreVector get_all_herbivores();

  /// Get read-only pointers to all alive herbivores of all populations.
  /**
   * \return Pointers to all living herbivores in all
   * populations. Guaranteed no `NULL` pointers.
   */
  ConstHerbivoreVector get_all_herbivores() const;

  /// Kill populations whose density is below minimum threshold.
  /**
   * If a population has a total density of less than
   * \ref Hft::mortality_minimum_density_threshold, all of the
   * herbivores are killed (\ref HerbivoreInterface::kill()).
   */
  void kill_nonviable();

  /// Delete all dead herbivores.
  /** Iterate over all populations and call
   * \ref PopulationInterface::purge_of_dead(). */
  void purge_of_dead();

  /// Create new herbivores if they got extinct.
  void reestablish();

 private:
  std::list<std::unique_ptr<PopulationInterface>> list;
};
}  // namespace Fauna

#endif  // FAUNA_POPULATION_LIST_H
