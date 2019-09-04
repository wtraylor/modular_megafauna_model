/**
 * \file
 * \brief Create objects for the \ref World class.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_WORLD_CONSTRUCTOR_H
#define FAUNA_WORLD_CONSTRUCTOR_H

#include <memory>

namespace Fauna {
// Forward declarations
class Parameters;
class Hft;
class HftList;
class PopulationInterface;
class PopulationList;
class DistributeForage;

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
  WorldConstructor(const Parameters& params, const HftList& hftlist);

  /// Create one (empty) herbivore population for one HFT.
  /**
   * \param phft Pointer to the Hft.
   * \throw std::logic_error if \ref Parameters::herbivore_type is not
   * implemented
   * \return Pointer to new object.
   */
  PopulationInterface* create_population(const Hft* phft) const;

  /// Instantiate populations for all HFTs in one \ref Habitat.
  /**
   * \throw std::logic_error if \ref Parameters::herbivore_type is not
   * implemented
   * \return Pointer to new object
   */
  PopulationList* create_populations() const;

  /// Instantiate a population of only one \ref Hft for one \ref Habitat.
  /**
   * \param phft Pointer to the one \ref Hft.
   * \throw std::logic_error if \ref Parameters::herbivore_type is not
   * implemented.
   * \return Pointer to new object.
   */
  PopulationList* create_populations(const Hft* phft) const;

  /// Create new \ref DistributeForage object according to parameters.
  std::auto_ptr<DistributeForage> create_distribute_forage() const;

  /// Get herbivore functional types.
  const HftList& get_hftlist() const { return hftlist; }

  /// Get global parameters.
  const Parameters& get_params() const { return params; }

 private:
  const Parameters& params;
  const HftList& hftlist;
};
}  // namespace Fauna
#endif  // FAUNA_WORLD_CONSTRUCTOR_H
