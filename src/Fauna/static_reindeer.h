/**
 * \file
 * \brief Constant reindeer population
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_STATIC_REINDEER_H
#define FAUNA_STATIC_REINDEER_H

#include "herbivore_data.h"
#include "herbivore_interface.h"
#include "population_interface.h"

namespace Fauna {

class StaticReindeerPopulation;

class StaticReindeer : public HerbivoreInterface {
 public:
  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)) {
    for (const auto& ft : FORAGE_TYPES)
      if (N_kg_per_km2[ft] > 0)
        throw std::logic_error(
            "Fauna::StaticReindeer::eat() "
            "Received a non-zero amount of nitrogen for forage type '" +
            get_forage_type_name(ft) +
            "'. The StaticReindeer herbivore class does not have nitrogen "
            "excretion. Don’t feed nitrogen to it because it would not be "
            "returned.");
    todays_output.eaten_forage_per_ind += kg_per_km2 / get_ind_per_km2();
    todays_output.eaten_nitrogen_per_ind +=
        N_kg_per_km2.sum() / get_ind_per_km2();
    eaten_nitrogen += N_kg_per_km2.sum();
  }

  virtual double get_bodymass() const { return 75; }

  virtual ForageMass get_forage_demands(const HabitatForage& available_forage) {
    ForageMass demand;
    // 4 km dry matter per day and animal
    demand[ForageType::Grass] = get_ind_per_km2() * 4.0;
    return demand;
  }

  virtual double get_ind_per_km2() const { return 2.5; }

  virtual double get_kg_per_km2() const {
    return get_ind_per_km2() * get_bodymass();
  }

  virtual std::string get_output_group() const { return "Reindeer"; }

  virtual const Output::HerbivoreData& get_todays_output() const {
    return todays_output;
  };

  virtual bool is_dead() const { return false; }

  virtual void kill() {
    throw std::logic_error(
        "Fauna::StaticReindeer::kill() should never be called.");
  }

  virtual void simulate_day(const int day,
                            const HabitatEnvironment& environment,
                            double& offspring) {
    // Nothing to do here, but set the output.
    todays_output.reset();
    todays_output.inddens = get_ind_per_km2();
    todays_output.massdens = get_kg_per_km2();
  }

  virtual double take_nitrogen_excreta() { return eaten_nitrogen; }

 private:
  Output::HerbivoreData todays_output;

  /// Nitrogen pool within the reindeer [kgN/km²].
  /**
   * This is filled by \ref eat() and emptied again by
   * \ref take_nitrogen_excreta(). So there is no retention of nitrogen. It
   * cycles directly back into the vegetation model.
   */
  bool eaten_nitrogen = 0.0;
};

class StaticReindeerPopulation : public PopulationInterface {
 public:
  StaticReindeerPopulation() {}
  virtual void create_offspring(const double ind_per_km2) {}
  virtual void establish() {}

  virtual ConstHerbivoreVector get_list() const;
  virtual HerbivoreVector get_list() { return list; }

  virtual void kill_nonviable() {}

  virtual void purge_of_dead() {}

 private:
  HerbivoreVector list = {new StaticReindeer};
};

}  // namespace Fauna

#endif  // FAUNA_STATIC_REINDEER_H
