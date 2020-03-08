/**
 * \file
 * \brief Create objects for the \ref Fauna::World class.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_WORLD_CONSTRUCTOR_H
#define FAUNA_WORLD_CONSTRUCTOR_H

#include <cassert>
#include <memory>
#include <vector>
#include "population_list.h"

namespace Fauna {
// Forward declarations
class Parameters;
class Hft;
class PopulationInterface;
class DistributeForage;

// Repeat typedef from hft.h
typedef std::vector<std::shared_ptr<const Hft> > HftList;

/// Helper class of World to create various megafauna components.
/**
 * We separate \ref World and \ref WorldConstructor into two classes because
 * the framework class \ref World should be as slim as possible because it is
 * exposed in the library interface. Moreover, \ref World cannot be easily
 * unit-tested since it is constructed with an external instruction file.
 */
class WorldConstructor {
 public:
  /// Constructor: only set member variables.
  WorldConstructor(const std::shared_ptr<const Parameters> params,
                   const HftList& hftlist);

  /// Instantiate populations for all HFTs in one \ref Habitat.
  /**
   * \throw std::logic_error if \ref Parameters::herbivore_type is not
   * implemented
   * \return Pointer to new object
   */
  PopulationList* create_populations() const;

  /// Create new \ref DistributeForage object according to parameters.
  DistributeForage* create_distribute_forage() const;

  /// Get herbivore functional types.
  const HftList& get_hftlist() const { return hftlist; }

  /// Get global parameters.
  const Parameters& get_params() const {
    assert(params.get());
    return *params;
  }

 private:
  const std::shared_ptr<const Parameters> params;
  const HftList& hftlist;
};
}  // namespace Fauna
#endif  // FAUNA_WORLD_CONSTRUCTOR_H
