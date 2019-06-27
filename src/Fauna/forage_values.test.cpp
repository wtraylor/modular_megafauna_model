#include <catch2/catch.hpp>
#include "forage_values.h"

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
