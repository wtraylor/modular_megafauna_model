#include "feed_herbivores.h"
#include "forage_distribution_algorithms.h"
#include "habitat_forage.h"
#include "herbivore_interface.h"

using namespace Fauna;

FeedHerbivores::FeedHerbivores(
    std::auto_ptr<DistributeForage> _distribute_forage)
    : distribute_forage(_distribute_forage) {
  if (distribute_forage.get() == NULL)
    throw std::invalid_argument(
        "Fauna::FeedHerbivores::FeedHerbivores() "
        "Parameter `distribute_forage` is NULL.");
}

void FeedHerbivores::operator()(HabitatForage& available,
                                const HerbivoreVector& herbivores) const {
  // loop as many times as there are forage types
  // to allow prey switching:
  // If one forage type gets “empty” in the first loop, the
  // herbivores can then demand from another forage type, and so
  // on until it’s all empty or they are all satisfied or cannot
  // switch to another forage type.
  for (int i = 0; i < FORAGE_TYPES.size(); i++) {
    // If there is no forage available (anymore), abort!
    if (available.get_mass() <= 0.00001) break;

    //------------------------------------------------------------
    // GET FORAGE DEMANDS
    ForageDistribution forage_demand;
    for (HerbivoreVector::const_iterator itr = herbivores.begin();
         itr != herbivores.end(); itr++) {
      HerbivoreInterface& herbivore = **itr;

      // Skip dead herbivores.
      if (herbivore.is_dead()) continue;

      // calculate forage demand for this herbivore
      const ForageMass ind_demand = herbivore.get_forage_demands(available);

      // only add those herbivores that do want to eat
      if (!(ind_demand == 0.0)) forage_demand[&herbivore] = ind_demand;
    }

    // abort if all herbivores are satisfied
    if (forage_demand.empty()) break;

    // get the forage distribution
    assert(distribute_forage.get() != NULL);
    (*distribute_forage)(available, forage_demand);

    // rename variable to make clear it’s not the demands anymore
    // but the portions to feed the herbivores
    ForageDistribution& forage_portions = forage_demand;

    //------------------------------------------------------------
    // LET THE HERBIVORES EAT

    const Digestibility digestibility = available.get_digestibility();
    const ForageFraction nitrogen_content = available.get_nitrogen_content();

    // Loop through all portions and feed it to the respective
    // herbivore
    for (ForageDistribution::iterator iter = forage_portions.begin();
         iter != forage_portions.end(); iter++) {
      const ForageMass& portion = iter->second;  // [kgDM/km²]
      HerbivoreInterface& herbivore = *(iter->first);

      const ForageMass& nitrogen = portion * nitrogen_content;

      if (herbivore.get_ind_per_km2() > 0.0) {
        // feed this herbivore
        herbivore.eat(portion, digestibility, nitrogen);

        // reduce the available forage
        for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
             ft != FORAGE_TYPES.end(); ft++) {
          available[*ft].set_nitrogen_mass(available[*ft].get_nitrogen_mass() -
                                           nitrogen[*ft]);
          available[*ft].set_mass(available[*ft].get_mass() - portion[*ft]);
        }
      }
    }
  }
}
