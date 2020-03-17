/**
 * \file
 * \brief A list of herbivore populations in a habitat.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_POPULATION_LIST_H
#define FAUNA_POPULATION_LIST_H

#include <memory>
#include <vector>

namespace Fauna {
// Forward Declarations
class PopulationInterface;

typedef std::vector<std::shared_ptr<PopulationInterface> > PopulationList;

}  // namespace Fauna

#endif  // FAUNA_POPULATION_LIST_H
