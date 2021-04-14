// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief A list of herbivore populations in a habitat.
 * \copyright LGPL-3.0-or-later
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
