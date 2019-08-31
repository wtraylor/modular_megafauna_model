#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include "catch.hpp"
#include "datapoint.h"
#include "dummy_hft.h"
#include "fileystem.h"
#include "text_table_writer.h"

using namespace Fauna;
using namespace Fauna::Output;

namespace {
/// Create a random output directory name.
std::string generate_output_dir() {
  std::srand(std::time(nullptr));  // set seed for random generator
  const int random = std::rand() / ((RAND_MAX + 1u) / 100);
  return "unittest_TextTableWriter_" + std::to_string(random);
}

/// Split a line into string elements by a delimiter (general template).
/** Source: https://stackoverflow.com/a/236803 */
template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

/// Split a line into string elements by a delimiter.
/** Source: https://stackoverflow.com/a/236803 */
std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}
}  // namespace

TEST_CASE("Fauna::Output::TextTableWriter", "") {
  Parameters::TextTableWriterOptions opt;
  opt.mass_density_per_hft = true;
  opt.output_directory = generate_output_dir();


  REQUIRE(!directory_exists(opt.output_directory));

  INFO((std::string) "Random output directory: " + opt.output_directory);

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
    const std::string mass_density_per_hft_path =
        opt.output_directory + '/' + "mass_density_per_hft" +
        TextTableWriter::FILE_EXTENSION;

    // Constructor
    TextTableWriter writer(OutputInterval::Annual, opt);

    REQUIRE(directory_exists(opt.output_directory));

    std::ifstream mass_density_per_hft(mass_density_per_hft_path);
    REQUIRE(mass_density_per_hft.good());

    SECTION("File already exists") {
      CHECK_THROWS(TextTableWriter(OutputInterval::Annual, opt));
    }

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

    // The first call should create captions.
    datapoint.data.datapoint_count = 1;
    REQUIRE(datapoint.data.datapoint_count > 0);
    writer.write_datapoint(datapoint);

    INFO((std::string) "Random output directory: " + opt.output_directory);
    // Check column captions
    {
      std::string line;
      REQUIRE(std::getline(mass_density_per_hft, line));
      std::vector<std::string> fields =
          split(line, TextTableWriter::FIELD_SEPARATOR);

      INFO((std::string) "line: " + line);
      INFO("fields array:");
      for (const auto &f : fields) INFO(f);
      REQUIRE(fields.size() == 5);

      CHECK(fields[0] == "year");
      CHECK(fields[1] == "agg_unit");
      CHECK(fields[2] == HFTS[0].name);
      CHECK(fields[3] == HFTS[1].name);
      CHECK(fields[4] == HFTS[2].name);
    }

    // Check tuple
    {
      std::string line;
      REQUIRE(std::getline(mass_density_per_hft, line));

      std::vector<std::string> fields =
          split(line, TextTableWriter::FIELD_SEPARATOR);

      INFO((std::string) "line: " + line);
      INFO("fields array:");
      for (const auto &f : fields) INFO(f);
      REQUIRE(fields.size() == 5);

      int year;
      std::string agg_unit;
      double hft1, hft2, hft3;
      REQUIRE_NOTHROW(year = std::stoi(fields[0]));
      agg_unit = fields[1];
      REQUIRE_NOTHROW(hft1 = std::stod(fields[2]));
      REQUIRE_NOTHROW(hft2 = std::stod(fields[3]));
      REQUIRE_NOTHROW(hft3 = std::stod(fields[4]));

      CHECK(year == YEAR);
      CHECK(agg_unit == AGG_UNIT);
      CHECK(hft1 == Approx(datapoint.data.hft_data[&HFTS[0]].massdens));
      CHECK(hft2 == Approx(datapoint.data.hft_data[&HFTS[1]].massdens));
      CHECK(hft3 == Approx(datapoint.data.hft_data[&HFTS[2]].massdens));
    }

    SECTION("Error on illegal aggregation unit name") {
      SECTION("Whitespace") {
        datapoint.aggregation_unit = "agg unit";
        REQUIRE(datapoint.data.datapoint_count > 0);
        CHECK_THROWS(writer.write_datapoint(datapoint));
      }
      SECTION("Delimiter") {
        datapoint.aggregation_unit =
            (std::string) "aggunit" + TextTableWriter::FIELD_SEPARATOR;
        REQUIRE(datapoint.data.datapoint_count > 0);
        CHECK_THROWS(writer.write_datapoint(datapoint));
      }
    }
  }

  // It is probably good enough to only check for the annual output. The other
  // output schemes are very similarly implemented.

  // Delete directory recursively.
  if (directory_exists(opt.output_directory))
      remove_directory(opt.output_directory);
}
