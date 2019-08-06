#ifndef WORLD_CONSTRUCTOR_H
#define WORLD_CONSTRUCTOR_H

namespace Fauna {
// Forward declarations
class Parameters;
class HftList;
class PopulationInterface;
class HftPopulationsMap;
class DistributeForage;

class WorldConstructor {
 public:
  /// Constructor: only set member variables.
  WorldConstructor(const Parameters& params, const HftList& hftlist);

  /// Create one (empty) herbivore population for one HFT.
  /**
   * \param phft Pointer to the Hft.
   * \throw std::logic_error if \ref Parameters::herbivore_type
   * is not implemented
   * \return Pointer to new object.
   */
  std::auto_ptr<PopulationInterface> create_population(const Hft* phft) const;

  /// Instantiate populations for all HFTs in one \ref Habitat.
  /**
   * \throw std::logic_error if \ref Parameters::herbivore_type is not
   * implemented
   * \return Pointer to new object
   */
  std::auto_ptr<HftPopulationsMap> create_populations() const;

  /// Instantiate a populaton of only one \ref Hft for one \ref Habitat.
  /**
   * \param phft Pointer to the one \ref Hft.
   * \throw std::logic_error if \ref Parameters::herbivore_type is not
   * implemented.
   * \return Pointer to new object.
   */
  std::auto_ptr<HftPopulationsMap> create_populations(const Hft* phft) const;

  /// Create new \ref DistributeForage object according to parameters.
  std::auto_ptr<DistributeForage> create_distribute_forage();

  /// Get herbivore functional types.
  const HftList& get_hftlist() { return hftlist; }

  /// Get global parameters.
  const Parameters& get_params() { return params; }

 private:
  const Parameters& params;
  const HftList& hftlist;
};
}  // namespace Fauna
#endif  // WORLD_CONSTRUCTOR_H
