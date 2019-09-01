#include "catch.hpp"
#include "dummy_hft.h"
#include "get_forage_demands.h"
using namespace Fauna;
#include "parameters.h"

TEST_CASE("Fauna::GetForageDemands") {
  // constructor exceptions
  CHECK_THROWS(GetForageDemands(NULL, SEX_MALE));

  const Parameters params;
  Hft hft = create_hfts(1, params)[0];
  hft.foraging_limits.clear();
  hft.digestive_limit = DL_NONE;

  const int DAY = 0;
  HabitatForage avail;                            // available forage
  const ForageEnergyContent ENERGY_CONTENT(1.0);  // [MJ/kgDM]
  const double BODYMASS = hft.bodymass_female;    // [kg/ind]

  SECTION("Check some exceptions.") {
    // Create the object.
    GetForageDemands gfd(&hft, SEX_FEMALE);

    // Exception because not initialized.
    CHECK_THROWS(gfd(1.0));

    // exceptions during initialization
    CHECK_THROWS(gfd.init_today(DAY, avail, ENERGY_CONTENT,
                                -1));  // body mass
    CHECK_THROWS(gfd.init_today(-1,    // day
                                avail, ENERGY_CONTENT, BODYMASS));

    // initialize
    CHECK(!gfd.is_day_initialized(DAY));
    gfd.init_today(DAY, avail, ENERGY_CONTENT, BODYMASS);
    CHECK(gfd.is_day_initialized(DAY));

    // negative energy needs
    CHECK_THROWS(gfd(-1.0));

    // Negative eaten forage
    CHECK_THROWS(gfd.add_eaten(-1.0));

    // Eat more than possible.
    CHECK_THROWS(gfd.add_eaten(999999));
  }

  SECTION("Grazer with Fixed Fraction") {
    hft.diet_composer = DC_PURE_GRAZER;
    hft.digestive_limit = DL_FIXED_FRACTION;
    GetForageDemands gfd(&hft, SEX_FEMALE);  // create object
    const double DIG_FRAC = 0.03;  // max. intake as fraction of body mass
    hft.digestive_limit_fixed = DIG_FRAC;
    avail.grass.set_mass(999999);  // Lots of live grass (but nothing else).

    // We prescribe *lots* of hunger so that digestion *must* be the
    // limiting factor.
    const double ENERGY_DEMAND = 99999;

    // initialize
    gfd.init_today(DAY, avail, ENERGY_CONTENT, BODYMASS);

    // Lots of energy needs, but intake is limited by digestion.
    // Only grass shall be demanded.
    // The result must match the given fraction of body mass.
    const ForageMass init_demand = gfd(ENERGY_DEMAND);
    CHECK(init_demand[FT_GRASS] == Approx(BODYMASS * DIG_FRAC));
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++) {
      if (*ft != FT_GRASS) CHECK(init_demand[*ft] == 0.0);
    }
    // The demand may not change if we call it again.
    CHECK(gfd(ENERGY_DEMAND) == init_demand);

    // Now give something to eat, and the demand should become less.
    ForageMass EATEN;
    EATEN.set(FT_GRASS, 1.0);
    gfd.add_eaten(EATEN);
    CHECK(gfd(ENERGY_DEMAND) == init_demand - EATEN);
    // … and of course the result should stay the same.
    CHECK(gfd(ENERGY_DEMAND) == init_demand - EATEN);
  }
}

TEST_CASE("Fauna::GetForageDemands::get_max_intake_as_total_mass()") {
  double i = 1.0;
  ForageEnergyContent energy_content;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    energy_content.set(*ft, i++);

  // set some arbitrary proportions
  ForageFraction prop_mj;  // energy proportions
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    prop_mj.set(*ft, 1.0 / (++i));

  // exception
  CHECK_THROWS(GetForageDemands::get_max_intake_as_total_mass(
      prop_mj, energy_content, -1));

  const double KG_TOTAL = 10.0;

  const ForageMass result = GetForageDemands::get_max_intake_as_total_mass(
      prop_mj, energy_content, KG_TOTAL);

  CHECK(result.sum() == Approx(KG_TOTAL));

  // convert mass back to energy
  ForageMass mj = result * energy_content;
  const double mj_total = mj.sum();

  // The relation between each energy component towards the total
  // energy must stay the same.
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    CHECK(mj[*ft] / mj.sum() == Approx(prop_mj[*ft] / prop_mj.sum()));
}
