/**
 * \file
 * \brief Base class for creating herbivore cohorts & individuals.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_CREATE_HERBIVORE_COMMON_H
#define FAUNA_CREATE_HERBIVORE_COMMON_H

#include <memory>

namespace Fauna {
class Hft;
class Parameters;
enum class Sex;

/// Parent class for \ref CreateHerbivoreIndividual and
/// \ref  CreateHerbivoreCohort
/**
 * This parent class simply provides some common functionality common to both
 * child classes.
 */
class CreateHerbivoreCommon {
 public:
  /// The herbivore functional type.
  const Hft& get_hft() const;

  /// Global simulation parameters.
  const Parameters& get_params() const;

 protected:
  /// Protected constructor.
  /** \throw std::invalid_argument If `hft==NULL` or `parameters==NULL`*/
  CreateHerbivoreCommon(const std::shared_ptr<const Hft> hft,
                        const std::shared_ptr<const Parameters> parameters);

  /// Fat mass per maximum fat mass.
  double get_body_condition(const int age_days) const;

  const std::shared_ptr<const Hft> hft;
  const std::shared_ptr<const Parameters> parameters;
};

}  // namespace Fauna

#endif  // FAUNA_CREATE_HERBIVORE_COMMON_H
