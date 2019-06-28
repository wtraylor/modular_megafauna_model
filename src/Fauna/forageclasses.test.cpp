#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "forageclasses.h"
using namespace Fauna;

TEST_CASE("Fauna::ForageValues", "") {
  // Not all functions are tested here, only the
  // exceptions for invalid values, the constructors, and
  // get, set, and sum.
  SECTION("positive and zero") {
    CHECK_THROWS(ForageValues<POSITIVE_AND_ZERO>(-1.0));
    CHECK_THROWS(ForageValues<POSITIVE_AND_ZERO>(NAN));
    CHECK_THROWS(ForageValues<POSITIVE_AND_ZERO>(INFINITY));

    // zero initialization
    ForageValues<POSITIVE_AND_ZERO> fv;
    CHECK(fv.sum() == Approx(0.0));
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(fv[*ft] == 0.0);

    // exceptions
    CHECK_THROWS(fv.get(FT_INEDIBLE));
    CHECK_THROWS(fv[FT_INEDIBLE]);
    CHECK_THROWS(fv.set(FT_GRASS, -1.0));
    CHECK_THROWS(fv.set(FT_GRASS, NAN));
    CHECK_THROWS(fv.set(FT_GRASS, INFINITY));
    CHECK_THROWS(fv / 0.0);
    CHECK_THROWS(fv /= 0.0);

    const double G = 2.0;
    fv.set(FT_GRASS, G);
    CHECK(fv.get(FT_GRASS) == G);
    CHECK(fv[FT_GRASS] == G);
    CHECK(fv.sum() == G);  // because only grass changed

    // assignment
    ForageValues<POSITIVE_AND_ZERO> fv2 = fv;
    CHECK(fv2 == fv);
    CHECK(fv2[FT_GRASS] == fv[FT_GRASS]);
    CHECK(fv2.sum() == fv.sum());

    // value initialization
    const double V = 3.0;
    ForageValues<POSITIVE_AND_ZERO> fv3(V);
    CHECK(fv3[FT_GRASS] == V);
    CHECK(fv3.sum() == FORAGE_TYPES.size() * V);

    // Sums
    CHECK((fv + fv).sum() == Approx(fv.sum() + fv.sum()));
    CHECK((fv2 + fv).sum() == Approx(fv2.sum() + fv.sum()));
    CHECK((fv3 + fv).sum() == Approx(fv3.sum() + fv.sum()));
  }

  SECTION("zero to one") {
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(-1.0));
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(1.1));
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(NAN));
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(INFINITY));
  }

  SECTION("Comparison") {
    ForageValues<POSITIVE_AND_ZERO> fv1(0.0);
    ForageValues<POSITIVE_AND_ZERO> fv2(1.0);
    ForageValues<POSITIVE_AND_ZERO> fv3(fv2);

    CHECK(fv1 < fv2);
    CHECK(fv1 <= fv2);
    CHECK(fv2 >= fv1);
    CHECK(fv2 > fv1);

    CHECK(fv2 == fv3);
    CHECK(fv2 <= fv3);
    CHECK(fv2 >= fv3);
  }

  SECTION("Merging: positive and zero") {
    const double V1 = 3.0;
    const double V2 = 19.0;
    ForageValues<POSITIVE_AND_ZERO> a(V1);
    const ForageValues<POSITIVE_AND_ZERO> b(V2);
    const double W1 = 12.0;
    const double W2 = 23.0;
    a.merge(b, W1, W2);
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(a[*ft] == Approx((V1 * W1 + V2 * W2) / (W2 + W1)));
  }

  SECTION("Merging: zero to one") {
    const double V1 = 0.1;
    const double V2 = 0.8;
    ForageValues<POSITIVE_AND_ZERO> a(V1);
    const ForageValues<POSITIVE_AND_ZERO> b(V2);
    const double W1 = 12.0;
    const double W2 = 23.0;
    a.merge(b, W1, W2);
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(a[*ft] == Approx((V1 * W1 + V2 * W2) / (W2 + W1)));
  }

  SECTION("Minimums") {
    ForageValues<POSITIVE_AND_ZERO> a(1.0);
    ForageValues<POSITIVE_AND_ZERO> b(2.0);
    CHECK(a.min(a) == a);
    CHECK(a.min(b) == b.min(a));
    CHECK(a.min(b) == a);
  }

  //------------------------------------------------------------------
  // FREE FUNCTIONS

  SECTION("operator*(ForageFraction, double)") {
    ForageFraction ff;
    double i = 1.0;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      ff.set(*ft, 1.0 / ++i);

    double d = 123.4;
    const ForageValues<POSITIVE_AND_ZERO> result = d * ff;

    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(result[*ft] == ff[*ft] * d);
  }

  SECTION("operator*(ForageFraction, ForageValues<POSITIVE_AND_ZERO>") {
    ForageFraction ff;
    double i = 1.0;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      ff.set(*ft, 1.0 / ++i);

    ForageValues<POSITIVE_AND_ZERO> fv;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      fv.set(*ft, ++i);

    const ForageValues<POSITIVE_AND_ZERO> result = ff * fv;

    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(result[*ft] == ff[*ft] * fv[*ft]);
  }

  SECTION("foragevalues_to_foragefractions()") {
    // `tolerance` mustn’t be negative
    CHECK_THROWS(foragevalues_to_foragefractions(
        ForageValues<POSITIVE_AND_ZERO>(), -.1));

    ForageValues<POSITIVE_AND_ZERO> fv;
    SECTION("All numbers below 1.0") {
      double i = 1;
      // create some numbers between 0 and 1
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        fv.set(*ft, 1.0 / (++i));
      const ForageFraction ff = foragevalues_to_foragefractions(fv, 0.0);

      for (ForageFraction::const_iterator i = ff.begin(); i != ff.end(); i++)
        CHECK(i->second == fv[i->first]);
    }

    SECTION("Numbers with tolerance") {
      const double TOLERANCE = .1;
      fv.set(FT_GRASS, 1.0 + TOLERANCE);
      const ForageFraction ff = foragevalues_to_foragefractions(fv, TOLERANCE);

      CHECK(ff[FT_GRASS] == 1.0);
    }

    SECTION("Exception exceeding tolerance") {
      const double TOLERANCE = .1;
      fv.set(FT_GRASS, 1.0 + TOLERANCE + .001);
      CHECK_THROWS(foragevalues_to_foragefractions(fv, TOLERANCE));
    }
  }

  SECTION("foragefractions_to_foragevalues()") {
    ForageFraction ff;
    double i = 1;
    // create some numbers between 0 and 1
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      ff.set(*ft, 1.0 / (++i));

    const ForageValues<POSITIVE_AND_ZERO> fv =
        foragefractions_to_foragevalues(ff);

    for (ForageValues<POSITIVE_AND_ZERO>::const_iterator i = fv.begin();
         i != fv.end(); i++)
      CHECK(i->second == ff[i->first]);
  }

  SECTION("convert_mj_to_kg_proportionally()") {
    // set some arbitrary energy content
    double i = 31.0;
    ForageEnergyContent energy_content;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      energy_content.set(*ft, i++);

    // set some arbitrary proportions
    ForageFraction prop_mj;  // energy proportions
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      prop_mj.set(*ft, 1.0 / (++i));

    // calculate mass proportions
    const ForageFraction prop_kg =
        convert_mj_to_kg_proportionally(energy_content, prop_mj);

    // CHECK RESULTS

    CHECK(prop_kg.sum() == Approx(prop_mj.sum()));

    // convert mass back to energy
    ForageMass mj = prop_kg * energy_content;
    const double mj_total = mj.sum();

    // The relation between each energy component towards the total
    // energy must stay the same.
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(mj[*ft] / mj.sum() == Approx(prop_mj[*ft] / prop_mj.sum()));
  }
}

TEST_CASE("Fauna::GrassForage", "") {
  SECTION("Initialization") {
    CHECK(GrassForage().get_mass() == 0.0);
    CHECK(GrassForage().get_digestibility() == 0.0);
    CHECK(GrassForage().get_fpc() == 0.0);
  }

  SECTION("Exceptions") {
    CHECK_THROWS(GrassForage().set_fpc(1.2));
    CHECK_THROWS(GrassForage().set_fpc(-0.2));
    CHECK_THROWS(GrassForage().set_mass(-0.2));
    CHECK_THROWS(GrassForage().set_digestibility(-0.2));
    CHECK_THROWS(GrassForage().set_digestibility(1.2));

    GrassForage g;
    CHECK_THROWS(g.set_fpc(0.5));  // mass must be >0.0
    g.set_mass(1.0);
    CHECK_THROWS(g.get_fpc());     // illogical state
    CHECK_THROWS(g.set_fpc(0.0));  // fpc must be >0.0
  }

  SECTION("sward density") {
    CHECK(GrassForage().get_sward_density() == 0.0);

    GrassForage g;
    const double FPC = .234;
    const double MASS = 1256;
    g.set_mass(MASS);
    g.set_fpc(FPC);
    CHECK(g.get_sward_density() == Approx(MASS / FPC));
  }

  SECTION("merge") {
    // merge some arbitrary numbers
    GrassForage g1, g2;
    const double W1 = 956;
    const double W2 = 123;
    const double M1 = 23;
    const double M2 = 54;
    const double D1 = 0.342;
    const double D2 = 0.56;
    const double F1 = 0.76;
    const double F2 = 0.123;
    g1.set_mass(M1);
    g2.set_mass(M2);
    g1.set_digestibility(D1);
    g2.set_digestibility(D2);
    g1.set_fpc(F1);
    g2.set_fpc(F2);

    g1.merge(g2, W1, W2);
    CHECK(g1.get_mass() == Approx(average(M1, M2, W1, W2)));
    CHECK(g1.get_digestibility() == Approx(average(D1, D2, W1, W2)));
    CHECK(g1.get_fpc() == Approx(average(F1, F2, W1, W2)));
  }
}

TEST_CASE("Fauna::HabitatForage", "") {
  HabitatForage hf1 = HabitatForage();

  // Initialization
  REQUIRE(hf1.get_total().get_mass() == Approx(0.0));
  REQUIRE(hf1.get_total().get_digestibility() == Approx(0.0));

  SECTION("adding forage") {
    const double GRASSMASS = 10.0;        // dry matter [kgDM/km²]
    const double NMASS = GRASSMASS * .1;  // nitrogen [kgN/km²]
    hf1.grass.set_mass(GRASSMASS);
    hf1.grass.set_digestibility(0.5);
    hf1.grass.set_fpc(0.3);

    // Nitrogen
    CHECK_THROWS(hf1.grass.set_nitrogen_mass(GRASSMASS * 1.1));
    hf1.grass.set_nitrogen_mass(NMASS);
    CHECK(hf1.grass.get_nitrogen_mass() == NMASS);
    CHECK(hf1.get_nitrogen_content()[FT_GRASS] == Approx(NMASS / GRASSMASS));
    CHECK_THROWS(hf1.grass.set_mass(NMASS * .9));

    // Check value access
    REQUIRE(hf1.grass.get_mass() == GRASSMASS);
    CHECK(hf1.grass.get_mass() == hf1.get_mass()[FT_GRASS]);
    REQUIRE(hf1.get_total().get_mass() == GRASSMASS);
    CHECK(hf1.get_total().get_mass() == Approx(hf1.get_mass().sum()));
    REQUIRE(hf1.get_total().get_digestibility() == 0.5);
  }

  // The member function `merge()` is not tested here
  // because it is a nothing more than simple wrapper around
  // the merge functions of ForageBase and its child classes.
}

