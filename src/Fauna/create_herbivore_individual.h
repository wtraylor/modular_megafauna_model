/**
 * \file
 * \brief Construct new herbivore individual objects.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_CREATE_HERBIVORE_INDIVIDUAL_H
#define FAUNA_CREATE_HERBIVORE_INDIVIDUAL_H

#include "create_herbivore_common.h"

namespace Fauna {
class HerbivoreIndividual;

/// Function class constructing \ref HerbivoreIndividual objects.
class CreateHerbivoreIndividual : public CreateHerbivoreCommon {
 public:
  /// Constructor
  /** \copydoc CreateHerbivoreCommon::CreateHerbivoreCommon() */
  CreateHerbivoreIndividual(const Hft* hft, const Parameters* parameters)
      : CreateHerbivoreCommon(hft, parameters) {}

  /// Habitat area size [km²].
  double get_area_km2() const;

  /// Create a new object instance.
  /**
   * \param age_days Age in days.
   * \param sex Gender of the herbivore.
   * \return New object instance.
   * \throw std::invalid_argument If `age_days<0`
   */
  virtual HerbivoreIndividual operator()(const int age_days,
                                         const Sex sex) const;
};
}  // namespace Fauna

#endif  // FAUNA_CREATE_HERBIVORE_INDIVIDUAL_H
