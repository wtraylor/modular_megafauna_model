// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::ForageValues.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "forage_values.h"
#include "catch.hpp"

using namespace Fauna;

TEST_CASE("Fauna::ForageValues", "") {
  // Not all functions are tested here, only the
  // exceptions for invalid values, the constructors, and
  // get, set, and sum.
  SECTION("positive and zero") {
    CHECK_THROWS(ForageValues<ForageValueTag::PositiveAndZero>(-1.0));
    CHECK_THROWS(ForageValues<ForageValueTag::PositiveAndZero>(NAN));
    CHECK_THROWS(ForageValues<ForageValueTag::PositiveAndZero>(INFINITY));

    // Allow for imprecision
    const double barely_zero =
        -ForageValues<ForageValueTag::ZeroToOne>::IMPRECISION_TOLERANCE;
    REQUIRE(barely_zero < 0.0);
    CHECK_NOTHROW(ForageValues<ForageValueTag::PositiveAndZero>(barely_zero));
    CHECK_THROWS(
        ForageValues<ForageValueTag::PositiveAndZero>(barely_zero * 2.0));

    // zero initialization
    ForageValues<ForageValueTag::PositiveAndZero> fv;
    CHECK(fv.sum() == Approx(0.0));
    for (const auto ft : FORAGE_TYPES) CHECK(fv[ft] == 0.0);

    // exceptions
    CHECK_THROWS(fv.get(ForageType::Inedible));
    CHECK_THROWS(fv[ForageType::Inedible]);
    CHECK_THROWS(fv.set(ForageType::Grass, -1.0));
    CHECK_THROWS(fv.set(ForageType::Grass, NAN));
    CHECK_THROWS(fv.set(ForageType::Grass, INFINITY));
    CHECK_THROWS(fv / 0.0);
    CHECK_THROWS(fv /= 0.0);

    const double G = 2.0;
    fv.set(ForageType::Grass, G);
    CHECK(fv.get(ForageType::Grass) == G);
    CHECK(fv[ForageType::Grass] == G);
    CHECK(fv.sum() == G);  // because only grass changed

    // assignment
    ForageValues<ForageValueTag::PositiveAndZero> fv2 = fv;
    CHECK(fv2 == fv);
    CHECK(fv2[ForageType::Grass] == fv[ForageType::Grass]);
    CHECK(fv2.sum() == fv.sum());

    // value initialization
    const double V = 3.0;
    ForageValues<ForageValueTag::PositiveAndZero> fv3(V);
    CHECK(fv3[ForageType::Grass] == V);
    CHECK(fv3.sum() == FORAGE_TYPES.size() * V);

    // Sums
    CHECK((fv + fv).sum() == Approx(fv.sum() + fv.sum()));
    CHECK((fv2 + fv).sum() == Approx(fv2.sum() + fv.sum()));
    CHECK((fv3 + fv).sum() == Approx(fv3.sum() + fv.sum()));
  }

  SECTION("zero to one") {
    // Bad values
    CHECK_THROWS(ForageValues<ForageValueTag::ZeroToOne>(-1.0));
    CHECK_THROWS(ForageValues<ForageValueTag::ZeroToOne>(1.1));
    CHECK_THROWS(ForageValues<ForageValueTag::ZeroToOne>(NAN));
    CHECK_THROWS(ForageValues<ForageValueTag::ZeroToOne>(INFINITY));

    // Good values
    CHECK_NOTHROW(ForageValues<ForageValueTag::ZeroToOne>(0.1));
    CHECK_NOTHROW(ForageValues<ForageValueTag::ZeroToOne>(0.9));
    CHECK_NOTHROW(ForageValues<ForageValueTag::ZeroToOne>(0.0));
    CHECK_NOTHROW(ForageValues<ForageValueTag::ZeroToOne>(1.0));

    // Allow for imprecision below zero.
    const double barely_zero =
        -ForageValues<ForageValueTag::ZeroToOne>::IMPRECISION_TOLERANCE;
    REQUIRE(barely_zero < 0.0);
    CHECK_NOTHROW(ForageValues<ForageValueTag::ZeroToOne>(barely_zero));
    CHECK_THROWS(ForageValues<ForageValueTag::ZeroToOne>(barely_zero * 2.0));

    // Allow for imprecision above one.
    const double barely_one =
        1.0 + ForageValues<ForageValueTag::ZeroToOne>::IMPRECISION_TOLERANCE;
    REQUIRE(barely_one > 1.0);
    CHECK_NOTHROW(ForageValues<ForageValueTag::ZeroToOne>(barely_one));
    CHECK_THROWS(ForageValues<ForageValueTag::ZeroToOne>(barely_one * 2.0));
  }

  SECTION("Comparison") {
    ForageValues<ForageValueTag::PositiveAndZero> fv1(0.0);
    ForageValues<ForageValueTag::PositiveAndZero> fv2(1.0);
    ForageValues<ForageValueTag::PositiveAndZero> fv3(fv2);

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
    ForageValues<ForageValueTag::PositiveAndZero> a(V1);
    const ForageValues<ForageValueTag::PositiveAndZero> b(V2);
    const double W1 = 12.0;
    const double W2 = 23.0;
    a.merge(b, W1, W2);
    for (const auto ft : FORAGE_TYPES)
      CHECK(a[ft] == Approx((V1 * W1 + V2 * W2) / (W2 + W1)));
  }

  SECTION("Merging: zero to one") {
    const double V1 = 0.1;
    const double V2 = 0.8;
    ForageValues<ForageValueTag::PositiveAndZero> a(V1);
    const ForageValues<ForageValueTag::PositiveAndZero> b(V2);
    const double W1 = 12.0;
    const double W2 = 23.0;
    a.merge(b, W1, W2);
    for (const auto ft : FORAGE_TYPES)
      CHECK(a[ft] == Approx((V1 * W1 + V2 * W2) / (W2 + W1)));
  }

  SECTION("Minimums") {
    ForageValues<ForageValueTag::PositiveAndZero> a(1.0);
    ForageValues<ForageValueTag::PositiveAndZero> b(2.0);
    CHECK(a.min(a) == a);
    CHECK(a.min(b) == b.min(a));
    CHECK(a.min(b) == a);
  }

  //------------------------------------------------------------------
  // FREE FUNCTIONS

  SECTION("operator*(ForageFraction, double)") {
    ForageFraction ff;
    double i = 1.0;
    for (const auto ft : FORAGE_TYPES) ff.set(ft, 1.0 / ++i);

    double d = 123.4;
    const ForageValues<ForageValueTag::PositiveAndZero> result = d * ff;

    for (const auto ft : FORAGE_TYPES) CHECK(result[ft] == ff[ft] * d);
  }

  SECTION(
      "operator*(ForageFraction, "
      "ForageValues<ForageValueTag::PositiveAndZero>") {
    ForageFraction ff;
    double i = 1.0;
    for (const auto ft : FORAGE_TYPES) ff.set(ft, 1.0 / ++i);

    ForageValues<ForageValueTag::PositiveAndZero> fv;
    for (const auto ft : FORAGE_TYPES) fv.set(ft, ++i);

    const ForageValues<ForageValueTag::PositiveAndZero> result = ff * fv;

    for (const auto ft : FORAGE_TYPES) CHECK(result[ft] == ff[ft] * fv[ft]);
  }

  SECTION("foragevalues_to_foragefractions()") {
    // `tolerance` mustn’t be negative
    CHECK_THROWS(foragevalues_to_foragefractions(
        ForageValues<ForageValueTag::PositiveAndZero>(), -.1));

    ForageValues<ForageValueTag::PositiveAndZero> fv;
    SECTION("All numbers below 1.0") {
      double i = 1;
      // create some numbers between 0 and 1
      for (const auto ft : FORAGE_TYPES) fv.set(ft, 1.0 / (++i));
      const ForageFraction ff = foragevalues_to_foragefractions(fv, 0.0);

      for (const auto ft : FORAGE_TYPES) CHECK(ff[ft] == fv[ft]);
    }

    SECTION("Numbers with tolerance") {
      const double TOLERANCE = .1;
      fv.set(ForageType::Grass, 1.0 + TOLERANCE);
      const ForageFraction ff = foragevalues_to_foragefractions(fv, TOLERANCE);

      CHECK(ff[ForageType::Grass] == 1.0);
    }

    SECTION("Exception exceeding tolerance") {
      const double TOLERANCE = .1;
      fv.set(ForageType::Grass, 1.0 + TOLERANCE + .001);
      CHECK_THROWS(foragevalues_to_foragefractions(fv, TOLERANCE));
    }
  }

  SECTION("foragefractions_to_foragevalues()") {
    ForageFraction ff;
    double i = 1;
    // create some numbers between 0 and 1
    for (const auto ft : FORAGE_TYPES) ff.set(ft, 1.0 / (++i));

    const ForageValues<ForageValueTag::PositiveAndZero> fv =
        foragefractions_to_foragevalues(ff);

    for (const auto ft : FORAGE_TYPES) CHECK(fv[ft] == ff[ft]);
  }

  SECTION("convert_mj_to_kg_proportionally()") {
    // set some arbitrary energy content
    double i = 31.0;
    ForageEnergyContent energy_content;
    for (const auto ft : FORAGE_TYPES) energy_content.set(ft, i++);

    // set some arbitrary proportions
    ForageFraction prop_mj;  // energy proportions
    for (const auto ft : FORAGE_TYPES) prop_mj.set(ft, 1.0 / (++i));

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
    for (const auto ft : FORAGE_TYPES)
      CHECK(mj[ft] / mj.sum() == Approx(prop_mj[ft] / prop_mj.sum()));
  }
}
