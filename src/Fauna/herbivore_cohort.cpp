#include "herbivore_cohort.h"
#include "energetics.h"
#include "hft.h"

using namespace Fauna;

HerbivoreCohort::HerbivoreCohort(const int age_days,
                                 const double body_condition, const Hft* hft,
                                 const Sex sex, const double ind_per_km2)
    : HerbivoreBase(age_days, body_condition, hft, sex),
      ind_per_km2(ind_per_km2) {
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreIndividual::HerbivoreIndividual() "
        "ind_per_km2 <0.0");
}

HerbivoreCohort::HerbivoreCohort(const Hft* hft, const Sex sex,
                                 const double ind_per_km2)
    : HerbivoreBase(hft, sex),  // parent establishment constructor
      ind_per_km2(ind_per_km2) {
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreIndividual::HerbivoreIndividual() "
        "ind_per_km2 <0.0");
}

HerbivoreCohort::HerbivoreCohort(const HerbivoreCohort& other)
    : HerbivoreBase(other), ind_per_km2(other.ind_per_km2) {}

HerbivoreCohort& HerbivoreCohort::operator=(const HerbivoreCohort& other) {
  HerbivoreBase::operator=(other);
  ind_per_km2 = other.ind_per_km2;
  return *this;
}

void HerbivoreCohort::apply_mortality(const double mortality) {
  if (mortality < 0.0 || mortality > 1.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::apply_mortality() "
        "Parameter \"mortality\" out of range.");
  // change of individual density [ind/km²]
  const double ind_change = -mortality * get_ind_per_km2();
  // apply the change and make sure that the density does not
  // drop below zero because of precision artefacts
  ind_per_km2 = std::max(0.0, ind_per_km2 + ind_change);
  assert(ind_per_km2 >= 0.0);
}

bool HerbivoreCohort::is_dead() const { return get_ind_per_km2() <= 0.0; }

void HerbivoreCohort::merge(HerbivoreCohort& other) {
  if (!is_same_age(other))
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::merge() "
        "The other cohort is not the same age.");
  if (this->get_sex() != other.get_sex())
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::merge() "
        "The other cohort is not the same sex.");
  if (this->get_hft() != other.get_hft())
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::merge() "
        "The other cohort is not the same HFT.");

  // Merge energy budget
  this->get_energy_budget().merge(other.get_energy_budget(),
                                  this->get_ind_per_km2(),
                                  other.get_ind_per_km2());

  // Merge nitrogen
  this->get_nitrogen().merge(other.get_nitrogen());

  // sum up density
  this->ind_per_km2 += other.ind_per_km2;
  // Change density in other object
  other.ind_per_km2 = 0.0;
}
