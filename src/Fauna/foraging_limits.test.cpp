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
  static const Digestibility DIG(0.5);

  CHECK_THROWS(get_digestive_limit_illius_gordon_1992(
      -1.0, DigestionType::Ruminant, -1.0, DIG));
  CHECK_THROWS(get_digestive_limit_illius_gordon_1992(
      0.0, DigestionType::Ruminant, 0.0, DIG));

  SECTION("exceptions") {
    static const double AD = 100.0;
    CHECK_THROWS(get_digestive_limit_illius_gordon_1992(
        AD, DigestionType::Ruminant, AD + 1, DIG));
    CHECK_THROWS(get_digestive_limit_illius_gordon_1992(
        AD, DigestionType::Ruminant, 0.0, DIG));
    CHECK_THROWS(get_digestive_limit_illius_gordon_1992(
        AD, DigestionType::Ruminant, -1.0, DIG));
  }

  SECTION("check some example numbers") {
    static const double AD = 40.0;       // adult weight, [kg]
    static const double CURRENT = 20.0;  // current weight [kg]

    SECTION("...for grass") {
      static const double d = DIG[ForageType::Grass];
      CHECK(get_digestive_limit_illius_gordon_1992(
                AD, DigestionType::Ruminant, CURRENT, d)[ForageType::Grass] ==
            Approx(0.034 * exp(3.565 * d) * pow(AD, 0.077 * exp(d) + 0.73) *
                   pow(CURRENT / AD, 0.75)));

      CHECK(get_digestive_limit_illius_gordon_1992(
                AD, DigestionType::Hindgut, CURRENT, d)[ForageType::Grass] ==
            Approx(0.108 * exp(3.284 * d) * pow(AD, 0.080 * exp(d) + 0.73) *
                   pow(CURRENT / AD, 0.75)));
    }
  }

  SECTION("pre-adult has less capacity") {
    static const double AD = 100.0;
    CHECK(get_digestive_limit_illius_gordon_1992(AD, DigestionType::Ruminant,
                                                 AD / 2, DIG) <
          get_digestive_limit_illius_gordon_1992(AD, DigestionType::Ruminant,
                                                 AD, DIG));
    CHECK(get_digestive_limit_illius_gordon_1992(AD, DigestionType::Hindgut,
                                                 AD / 2, DIG) <
          get_digestive_limit_illius_gordon_1992(AD, DigestionType::Hindgut, AD,
                                                 DIG));
  }

  SECTION("bigger animals have more capacity") {
    static const double AD1 = 100.0;
    static const double AD2 = AD1 * 1.4;
    CHECK(get_digestive_limit_illius_gordon_1992(AD1, DigestionType::Hindgut,
                                                 AD1, DIG) <
          get_digestive_limit_illius_gordon_1992(AD2, DigestionType::Hindgut,
                                                 AD2, DIG));
    CHECK(get_digestive_limit_illius_gordon_1992(AD1, DigestionType::Ruminant,
                                                 AD1, DIG) <
          get_digestive_limit_illius_gordon_1992(AD2, DigestionType::Ruminant,
                                                 AD2, DIG));
  }

  SECTION("higher digestibility brings higher capacity") {
    const double AD = 100.0;
    const Digestibility DIG1(.8);
    const Digestibility DIG2(.9);
    {  // RUMINANT
      INFO("Ruminant, digestibility=" << DIG1[ForageType::Grass]);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(
               AD, DigestionType::Ruminant, AD, DIG1)[ForageType::Grass]);

      INFO("Ruminant, digestibility=" << DIG2[ForageType::Grass]);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(
               AD, DigestionType::Ruminant, AD, DIG2)[ForageType::Grass]);

      CHECK(get_digestive_limit_illius_gordon_1992(AD, DigestionType::Ruminant,
                                                   AD, DIG1) <
            get_digestive_limit_illius_gordon_1992(AD, DigestionType::Ruminant,
                                                   AD, DIG2));
    }
    {  // HINDGUT
      INFO("Hindgut, digestibility=" << DIG1[ForageType::Grass]);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(
               AD, DigestionType::Hindgut, AD, DIG1)[ForageType::Grass]);

      INFO("Hindgut, digestibility=" << DIG2[ForageType::Grass]);
      INFO("grass: " << get_digestive_limit_illius_gordon_1992(
               AD, DigestionType::Hindgut, AD, DIG2)[ForageType::Grass]);

      CHECK(get_digestive_limit_illius_gordon_1992(AD, DigestionType::Hindgut,
                                                   AD, DIG1) <
            get_digestive_limit_illius_gordon_1992(AD, DigestionType::Hindgut,
                                                   AD, DIG2));
    }
  }

  SECTION("zero digestibility => zero energy") {
    const double ADULT = 100.0;
    const Digestibility ZERO(0.0);
    CHECK(get_digestive_limit_illius_gordon_1992(ADULT, DigestionType::Hindgut,
                                                 ADULT, ZERO) == 0.0);
    CHECK(get_digestive_limit_illius_gordon_1992(ADULT, DigestionType::Ruminant,
                                                 ADULT, ZERO) == 0.0);
  }
}
