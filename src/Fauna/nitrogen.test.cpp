/**
 * \file
 * \brief Unit test for nitrogen functionality.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "nitrogen.h"
using namespace Fauna;

TEST_CASE("Fauna::get_retention_time()") {
  CHECK_THROWS(get_retention_time(0));
  CHECK_THROWS(get_retention_time(-1));
  CHECK(get_retention_time(100) == Approx(45.276604));
}

TEST_CASE("Fauna::NitrogenInHerbivore") {
  NitrogenInHerbivore n;

  // Exceptions
  CHECK_THROWS(n.ingest(-.1));
  CHECK_THROWS(n.digest_today(-.1, 1.0));
  CHECK_THROWS(n.digest_today(0.0, 1.0));
  CHECK_THROWS(n.digest_today(1.0, -.1));

  // Initialization
  CHECK(n.get_excreta() == 0.0);
  CHECK(n.get_unavailable() == 0.0);

  SECTION("Ingestion-digestion cycle") {
    double ingested = 0.0;
    double soil = 0.0;   // nitrogen from excreta in the soil
    double total = 0.0;  // total nitrogen in ecoystem, as it should be
    const double RETENTION_TIME = 80;  // [hours]
    const double MASSDENS = 0;         // herbivore density [kg/km²]
    int hours = 0;                     // number of hours since first feeding
    for (int i = 0; i < 20; i++) {
      const double new_ingested = 1 + i % 2;  // just some positive numbers
      n.ingest(new_ingested);
      total += new_ingested;
      CHECK(total == soil + n.get_excreta() + n.get_unavailable());

      // every couple of feeding bouts, digest the nitrogen
      if (!(i % 3)) {
        hours += 24;
        n.digest_today(RETENTION_TIME, MASSDENS);
      }

      // put the excreta in the soil pool at some arbitrary interval
      if (!(i % 6)) {
        // Once the first feeding bout has passed the digestive tract,
        // there should be some excreta produced.
        INFO("hours = " << hours);
        if (hours > RETENTION_TIME) CHECK(n.get_excreta() > 0.0);

        soil += n.reset_excreta();
        CHECK(n.get_excreta() == 0);
      }
    }
  }

  SECTION("Tissue nitrogen") {
    const double MASSDENS = 10.0;       // [kg/km²]
    const double RETENTION_TIME = 1.0;  // hours
    const double INGESTED = MASSDENS;   // [kgN/km²]
    n.ingest(INGESTED);

    // Pass all nitrogen through the digestive tract.
    n.digest_today(RETENTION_TIME, MASSDENS);
    n.digest_today(RETENTION_TIME, MASSDENS);

    // Now, all nitrogen should be excreted, and only the tissue nitrogen
    // should be left in the unavailable pool.
    CHECK(n.get_unavailable() ==
          Approx(MASSDENS * NitrogenInHerbivore::N_CONTENT_IN_TISSUE));
    CHECK(n.get_unavailable() + n.get_excreta() == Approx(INGESTED));

    CHECK(n.reset_total() == Approx(INGESTED));
    CHECK(n.get_unavailable() == 0.0);
    CHECK(n.get_excreta() == 0.0);
  }
}

