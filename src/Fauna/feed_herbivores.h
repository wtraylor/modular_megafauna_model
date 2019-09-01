#ifndef FEED_HERBIVORES_H
#define FEED_HERBIVORES_H

#include <memory>
#include "herbivore_vector.h"

namespace Fauna {
// Forward Declarations
class DistributeForage;
class HabitatForage;

/// Function object to feed herbivores.
class FeedHerbivores {
 public:
  /// Constructor.
  /**
   * \param distribute_forage Strategy object for
   * calculating the forage portions.
   * \throw std::invalid_argument If `distribute_forage==NULL`. */
  FeedHerbivores(std::auto_ptr<DistributeForage> distribute_forage);

  /// Feed the herbivores.
  /**
   * \param[in,out] available Available forage mass in the
   * habitat. This will be reduced by the amount of eaten
   * forage.
   * \param[in,out] herbivores Herbivore objects that are
   * being fed by calling \ref HerbivoreInterface::eat().
   */
  void operator()(HabitatForage& available,
                  const HerbivoreVector& herbivores) const;

 private:
  std::auto_ptr<DistributeForage> distribute_forage;
};

}  // namespace Fauna

#endif  // FEED_HERBIVORES_H
