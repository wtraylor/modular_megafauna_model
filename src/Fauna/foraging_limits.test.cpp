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

TEST_CASE("Fauna::GetDigestiveLimitIlliusGordon1992", "") {
  CHECK_THROWS(
      GetDigestiveLimitIlliusGordon1992(-1.0, DigestionType::Ruminant));
  CHECK_THROWS(GetDigestiveLimitIlliusGordon1992(0.0, DigestionType::Ruminant));

  const Digestibility digestibility(0.5);

  SECTION("exceptions") {
    const Digestibility digestibility(0.5);
    const double AD = 100.0;
    GetDigestiveLimitIlliusGordon1992 rum(AD, DigestionType::Ruminant);
    CHECK_THROWS(rum(AD + 1, digestibility));
    CHECK_THROWS(rum(0.0, digestibility));
    CHECK_THROWS(rum(-1.0, digestibility));
  }

  SECTION("check some example numbers") {
    const double ADULT = 40.0;    // adult weight, [kg]
    const double CURRENT = 20.0;  // current weight [kg]
    GetDigestiveLimitIlliusGordon1992 rum(ADULT, DigestionType::Ruminant);
    GetDigestiveLimitIlliusGordon1992 hind(ADULT, DigestionType::Hindgut);

    SECTION("...for grass") {
      const double d = digestibility[FT_GRASS];
      CHECK(rum(CURRENT, d)[FT_GRASS] ==
            Approx(0.034 * exp(3.565 * d) * pow(ADULT, 0.077 * exp(d) + 0.73) *
                   pow(CURRENT / ADULT, 0.75)));

      CHECK(hind(CURRENT, d)[FT_GRASS] ==
            Approx(0.108 * exp(3.284 * d) * pow(ADULT, 0.080 * exp(d) + 0.73) *
                   pow(CURRENT / ADULT, 0.75)));
    }
  }

  SECTION("pre-adult has less capacity") {
    const double ADULT = 100.0;
    GetDigestiveLimitIlliusGordon1992 rum(ADULT, DigestionType::Ruminant);
    CHECK(rum(ADULT / 2, digestibility) < rum(ADULT, digestibility));
    GetDigestiveLimitIlliusGordon1992 hind(ADULT, DigestionType::Hindgut);
    CHECK(hind(ADULT / 2, digestibility) < hind(ADULT, digestibility));
  }

  SECTION("bigger animals have more capacity") {
    const double AD1 = 100.0;
    const double AD2 = AD1 * 1.4;
    const Digestibility DIG(.5);
    CHECK(GetDigestiveLimitIlliusGordon1992(AD1, DigestionType::Hindgut)(AD1,
                                                                         DIG) <
          GetDigestiveLimitIlliusGordon1992(AD2, DigestionType::Hindgut)(AD2,
                                                                         DIG));
    CHECK(GetDigestiveLimitIlliusGordon1992(AD1, DigestionType::Ruminant)(AD1,
                                                                          DIG) <
          GetDigestiveLimitIlliusGordon1992(AD2, DigestionType::Ruminant)(AD2,
                                                                          DIG));
  }

  SECTION("higher digestibility brings higher capacity") {
    const double ADULT = 100.0;
    const Digestibility DIG1(.8);
    const Digestibility DIG2(.9);
    {  // RUMINANT
      const GetDigestiveLimitIlliusGordon1992 rumi(ADULT,
                                                   DigestionType::Ruminant);

      INFO("Ruminant, digestibility=" << DIG1[FT_GRASS]);
      INFO("grass: " << rumi(ADULT, DIG1)[FT_GRASS]);

      INFO("Ruminant, digestibility=" << DIG2[FT_GRASS]);
      INFO("grass: " << rumi(ADULT, DIG2)[FT_GRASS]);

      CHECK(rumi(ADULT, DIG1) < rumi(ADULT, DIG2));
    }
    {  // HINDGUT
      const GetDigestiveLimitIlliusGordon1992 hind(ADULT,
                                                   DigestionType::Hindgut);

      INFO("Hindgut, digestibility=" << DIG1[FT_GRASS]);
      INFO("grass: " << hind(ADULT, DIG1)[FT_GRASS]);

      INFO("Hindgut, digestibility=" << DIG2[FT_GRASS]);
      INFO("grass: " << hind(ADULT, DIG2)[FT_GRASS]);

      CHECK(hind(ADULT, DIG1) < hind(ADULT, DIG2));
    }
  }

  SECTION("zero digestibility => zero energy") {
    const double ADULT = 100.0;
    const Digestibility ZERO(0.0);
    CHECK(GetDigestiveLimitIlliusGordon1992(ADULT, DigestionType::Hindgut)(
              ADULT, ZERO) == 0.0);
    CHECK(GetDigestiveLimitIlliusGordon1992(ADULT, DigestionType::Ruminant)(
              ADULT, ZERO) == 0.0);
  }
}

