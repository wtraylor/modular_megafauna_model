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

namespace Fauna {

class StaticReindeer : public HerbivoreInterface {
 public:
  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)){
    // Nothing is happening here, forage just disappears.
  }

  virtual double get_bodymass() const { return 75; }

  virtual ForageMass get_forage_demands(const HabitatForage& available_forage){
    ForageMass demand;
    // 4 km dry matter per day and animal
    demand[ForageType::Grass] = get_ind_per_km2() * 4.0;
    return demand;
  }

  virtual const Hft& get_hft() const {
    static const Hft reindeer_dummy;
    return reindeer_dummy;
  }

  virtual double get_ind_per_km2() const { return 2.5; }

  virtual double get_kg_per_km2() const {
    return get_ind_per_km2() * get_bodymass();
  }

  virtual const Output::HerbivoreData& get_todays_output() const {
    static const Output::HerbivoreData output_dummy;
    return output_dummy;
  };

  virtual bool is_dead() const { return false; }

  virtual void kill() {
    throw std::logic_error(
        "Fauna::StaticReindeer::kill() should never be called.");
  }

  virtual void simulate_day(const int day,
                            const HabitatEnvironment& environment,
                            double& offspring){
    // Nothing to do here yet.
  }

  virtual double take_nitrogen_excreta() { return 0; }
};

}  // namespace Fauna

#endif  // FAUNA_STATIC_REINDEER_H
