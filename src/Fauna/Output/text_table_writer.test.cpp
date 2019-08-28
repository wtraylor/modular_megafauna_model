#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "catch.hpp"
#include "datapoint.h"
#include "dummy_hft.h"
#include "text_table_writer.h"
#include "fileystem.h"

using namespace Fauna;
using namespace Fauna::Output;

namespace {
/// Create a random output directory name.
std::string generate_output_dir() {
  std::srand(std::time(nullptr));  // set seed for random generator
  const int random = std::rand() / ((RAND_MAX + 1u) / 100);
  return "unittest_TextTableWriter_" + std::to_string(random);
}

}  // namespace

TEST_CASE("Fauna::Output::TextTableWriter", "") {
  Parameters::TextTableWriterOptions opt;
  opt.mass_density_per_hft = true;
  opt.output_directory = generate_output_dir();

  static const int YEAR = 4;
  static const std::string AGG_UNIT = "unit1";
  static const HftList HFTS = create_hfts(3, Parameters());

  Datapoint datapoint;
  datapoint.aggregation_unit = AGG_UNIT;

  // Fill data with some arbitrary numbers.
  // -> Set more variables for tests for new output tables.
  datapoint.data.hft_data[&HFTS[0]].massdens = 10.0;
  datapoint.data.hft_data[&HFTS[1]].massdens = 16.0;
  datapoint.data.hft_data[&HFTS[2]].massdens = 29.0;
  datapoint.data.datapoint_count = 1;

  SECTION("Annual") {
    TextTableWriter writer(OutputInterval::Annual, opt);

    SECTION("Error on non-annual interval") {
      for (int day = 0; day < 366; day++)
        for (int year = YEAR; year < YEAR + 3; year++) {
          datapoint.interval = DateInterval(Date(0, YEAR), Date(day, year));
          if ((day != 364 && day != 365) || year != YEAR)
            CHECK_THROWS(writer.write_datapoint(datapoint));
        }
    }

    datapoint.interval = DateInterval(Date(0, YEAR), Date(364, YEAR));

    SECTION("Error on empty data") {
      datapoint.data.datapoint_count = 0;
      REQUIRE_THROWS(writer.write_datapoint(datapoint));
    }

    // The first call should create directory & files.
    REQUIRE(datapoint.data.datapoint_count > 0);
    writer.write_datapoint(datapoint);

    INFO((std::string) "Random output directory: " + opt.output_directory);

    REQUIRE(directory_exists(opt.output_directory));

    const std::string mass_density_per_hft_path =
        opt.output_directory + '/' + "mass_density_per_hft" +
        TextTableWriter::FILE_EXTENSION;

    std::ifstream mass_density_per_hft(mass_density_per_hft_path);
    REQUIRE(mass_density_per_hft.good());

    // Check header
    std::string hd_year, hd_agg_unit, hd_hft1, hd_hft2, hd_hft3;
    mass_density_per_hft >> hd_year >> hd_agg_unit >> hd_hft1 >> hd_hft2 >>
        hd_hft3;

    CHECK(hd_year == "year");
    CHECK(hd_agg_unit == "agg_unit");
    CHECK(hd_hft1 == HFTS[0].name);
    CHECK(hd_hft2 == HFTS[1].name);
    CHECK(hd_hft3 == HFTS[2].name);

    // Check tuple
    int year;
    std::string agg_unit;
    double hft1, hft2, hft3;
    mass_density_per_hft >> year >> agg_unit >> hft1 >> hft2 >> hft3;

    CHECK(year == YEAR);
    CHECK(agg_unit == AGG_UNIT);
    CHECK(hft1 == Approx(datapoint.data.hft_data[&HFTS[0]].massdens));
    CHECK(hft2 == Approx(datapoint.data.hft_data[&HFTS[1]].massdens));
    CHECK(hft3 == Approx(datapoint.data.hft_data[&HFTS[2]].massdens));
  }

  // TODO: Write tests for other intervals.
}
