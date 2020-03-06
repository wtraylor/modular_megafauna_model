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
#include "hft.h"
#include "population_interface.h"

namespace Fauna {

class StaticReindeerPopulation;

class StaticReindeer : public HerbivoreInterface {
 public:
  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)) {
    // Nothing is happening here, forage just disappears.
  }

  virtual double get_bodymass() const { return 75; }

  virtual ForageMass get_forage_demands(const HabitatForage& available_forage) {
    ForageMass demand;
    // 4 km dry matter per day and animal
    demand[ForageType::Grass] = get_ind_per_km2() * 4.0;
    return demand;
  }

  virtual const Hft& get_hft() const;

  virtual double get_ind_per_km2() const { return 2.5; }

  virtual double get_kg_per_km2() const {
    return get_ind_per_km2() * get_bodymass();
  }

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
    todays_output.inddens = get_ind_per_km2();
    todays_output.massdens = get_kg_per_km2();
  }

  virtual double take_nitrogen_excreta() { return 0; }

 private:
  Output::HerbivoreData todays_output;
};

class StaticReindeerPopulation : public PopulationInterface {
 public:
   StaticReindeerPopulation(){
     reindeer_dummy.name = "Reindeer";
   }
  virtual void create_offspring(const double ind_per_km2) {}
  virtual void establish() {}
  virtual const Hft& get_hft() const { return reindeer_dummy; }

  static Hft reindeer_dummy;
  virtual ConstHerbivoreVector get_list() const;
  virtual HerbivoreVector get_list() { return list; }

  virtual void purge_of_dead() {}

 private:
  HerbivoreVector list = {new StaticReindeer};
};

}  // namespace Fauna

#endif  // FAUNA_STATIC_REINDEER_H
