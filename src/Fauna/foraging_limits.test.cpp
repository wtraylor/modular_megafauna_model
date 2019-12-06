/**
 * \file
 * \brief Unit test for models limiting daily forage intake.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "foraging_limits.h"

using namespace Fauna;

TEST_CASE("Fauna::HalfMaxIntake") {
  CHECK_THROWS(HalfMaxIntake(-1, 1));
  CHECK_THROWS(HalfMaxIntake(0, 1));
  CHECK_THROWS(HalfMaxIntake(1, 0));
  CHECK_THROWS(HalfMaxIntake(1, -1));

  const double V_HALF = 10.0;   // half-saturation density
  const double MAX_RATE = 200;  // maximum intake rate
  const HalfMaxIntake h(V_HALF, MAX_RATE);

  CHECK_THROWS(h.get_intake_rate(-1));
  CHECK(h.get_intake_rate(0.0) == 0.0);
  CHECK(h.get_intake_rate(10.0) == Approx(MAX_RATE * 10.0 / (V_HALF + 10.0)));
}

TEST_CASE("Fauna::get_digestive_limit_illius_gordon_1992()", "") {
  static const double DIG = 0.5;
  static const std::array<double, 3> IJK_HIND = {0.108, 3.284, 0.080};
  static const std::array<double, 3> IJK_RUM = {0.034, 3.565, 0.077};

  CHECK_THROWS(
      get_digestive_limit_illius_gordon_1992(-1.0, -1.0, DIG, IJK_RUM));
  CHECK_THROWS(get_digestive_limit_illius_gordon_1992(0.0, 0.0, DIG, IJK_RUM));

  SECTION("exceptions") {
    static const double AD = 100.0;
    CHECK_THROWS(
        get_digestive_limit_illius_gordon_1992(AD, AD + 1, DIG, IJK_RUM));
    CHECK_THROWS(get_digestive_limit_illius_gordon_1992(AD, 0.0, DIG, IJK_RUM));
    CHECK_THROWS(
        get_digestive_limit_illius_gordon_1992(AD, -1.0, DIG, IJK_RUM));
  }

  SECTION("check some example numbers") {
    static const double AD = 40.0;       // adult weight, [kg]
    static const double CURRENT = 20.0;  // current weight [kg]

    SECTION("...for grass") {
      static const double d = DIG;
      CHECK(get_digestive_limit_illius_gordon_1992(AD, CURRENT, d, IJK_RUM) ==
            Approx(0.034 * exp(3.565 * d) * pow(AD, 0.077 * exp(d) + 0.73) *
                   pow(CURRENT / AD, 0.75)));

      CHECK(get_digestive_limit_illius_gordon_1992(AD, CURRENT, d, IJK_HIND) ==
            Approx(0.108 * exp(3.284 * d) * pow(AD, 0.080 * exp(d) + 0.73) *
                   pow(CURRENT / AD, 0.75)));
    }
  }

  SECTION("pre-adult has less capacity") {
    static const double AD = 100.0;
    CHECK(get_digestive_limit_illius_gordon_1992(AD, AD / 2, DIG, IJK_RUM) <
          get_digestive_limit_illius_gordon_1992(AD, AD, DIG, IJK_RUM));
    CHECK(get_digestive_limit_illius_gordon_1992(AD, AD / 2, DIG, IJK_HIND) <
          get_digestive_limit_illius_gordon_1992(AD, AD, DIG, IJK_HIND));
  }

  SECTION("bigger animals have more capacity") {
    static const double AD1 = 100.0;
    static const double AD2 = AD1 * 1.4;
    CHECK(get_digestive_limit_illius_gordon_1992(AD1, AD1, DIG, IJK_HIND) <
          get_digestive_limit_illius_gordon_1992(AD2, AD2, DIG, IJK_HIND));
    CHECK(get_digestive_limit_illius_gordon_1992(AD1, AD1, DIG, IJK_RUM) <
          get_digestive_limit_illius_gordon_1992(AD2, AD2, DIG, IJK_RUM));
  }

  SECTION("higher digestibility brings higher capacity") {
    const double AD = 100.0;
    const double DIG1(.8);
    const double DIG2(.9);
    {  // RUMINANT
      INFO("Ruminant, digestibility=" << DIG1);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(AD, AD, DIG1,
                                                               IJK_RUM));

      INFO("Ruminant, digestibility=" << DIG2);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(AD, AD, DIG2,
                                                               IJK_RUM));

      CHECK(get_digestive_limit_illius_gordon_1992(AD, AD, DIG1, IJK_RUM) <
            get_digestive_limit_illius_gordon_1992(AD, AD, DIG2, IJK_RUM));
    }
    {  // HINDGUT
      INFO("Hindgut, digestibility=" << DIG1);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(AD, AD, DIG1,
                                                               IJK_HIND));

      INFO("Hindgut, digestibility=" << DIG2);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(AD, AD, DIG2,
                                                               IJK_HIND));

      CHECK(get_digestive_limit_illius_gordon_1992(AD, AD, DIG1, IJK_HIND) <
            get_digestive_limit_illius_gordon_1992(AD, AD, DIG2, IJK_HIND));
    }
  }

  SECTION("zero digestibility => zero energy") {
    const double ADULT = 100.0;
    const double ZERO(0.0);
    CHECK(get_digestive_limit_illius_gordon_1992(ADULT, ADULT, ZERO,
                                                 IJK_HIND) == 0.0);
    CHECK(get_digestive_limit_illius_gordon_1992(ADULT, ADULT, ZERO,
                                                 IJK_HIND) == 0.0);
  }
}
