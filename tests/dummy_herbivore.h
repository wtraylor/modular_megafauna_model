#ifndef DUMMY_HERBIVORE_H
#define DUMMY_HERBIVORE_H

#include "forageclasses.h"
#include "herbivore.h"
#include "herbivore_data.h"

namespace Fauna {
/// A dummy herbivore that does nothing
class DummyHerbivore : public HerbivoreInterface {
 public:
  DummyHerbivore(const Hft* hft, const double ind_per_km2,
                 const double bodymass = 30.0)
      : hft(hft), ind_per_km2(ind_per_km2), bodymass(bodymass), killed(false) {}

  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)) {
    eaten += kg_per_km2;
    // Make sure we don’t drop the demand below zero.
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      actual_demand.set(*ft, std::max(0.0, actual_demand[*ft] - eaten[*ft]));
  }

  virtual double get_bodymass() const { return bodymass; }

  virtual ForageMass get_forage_demands(const HabitatForage& available_forage) {
    return actual_demand;
  }

  virtual const Hft& get_hft() const { return *hft; }

  virtual double get_ind_per_km2() const { return ind_per_km2; }
  virtual double get_kg_per_km2() const { return bodymass * ind_per_km2; }

  virtual const Output::HerbivoreData& get_todays_output() const {
    static Output::HerbivoreData dummy_output;
    return dummy_output;
  }

  virtual bool is_dead() const { return killed; }
  virtual void kill() {
    killed = true;
    ind_per_km2 = 0.0;
  }

  virtual void simulate_day(const int day, const HabitatEnvironment&,
                            double& offspring) {
    offspring = 0.0;
  }

  virtual double take_nitrogen_excreta() { return 0; }

 public:
  const ForageMass& get_original_demand() const { return original_demand; }
  void set_demand(const ForageMass& d) { original_demand = actual_demand = d; }
  const ForageMass& get_eaten() const { return eaten; }
  double ind_per_km2;

 private:
  const Hft* hft;
  const double bodymass;
  ForageMass original_demand, actual_demand, eaten;
  int killed;
};
}  // namespace Fauna

#endif  // DUMMY_HERBIVORE_H
