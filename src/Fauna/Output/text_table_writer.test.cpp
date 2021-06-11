// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Output::TextTableWriter.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "text_table_writer.h"
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

TEST_CASE("Fauna::Output::TextTableWriter ANNUAL", "") {
  TextTableWriterOptions opt;
  opt.eaten_forage_per_ind = true;
  opt.mass_density_per_hft = true;

  // We create 4 HFTs, but use only 3. The extra one is to check that an
  // exception gets thrown if the numbers don’t match up.
  static const HftList HFTS = create_hfts(4, Parameters());
  std::set<std::string> hft_names;
  for (int i = 0; i < 3; i++) hft_names.insert(HFTS[i]->name);

  // Constructor with new random output directory.
  opt.directory = generate_output_dir();
  REQUIRE(!directory_exists(opt.directory));
  TextTableWriter writer(OutputInterval::Annual, opt, hft_names);
  REQUIRE(directory_exists(opt.directory));
  INFO((std::string) "Random output directory: " + opt.directory);

  SECTION("File already exists") {
    CHECK_THROWS(TextTableWriter(OutputInterval::Annual, opt, hft_names));
  }

  static const int YEAR = 4;
  static const std::string AGG_UNIT = "unit1";

  // Since TextTableWriter sorts the HFT columns by name, we need to make sure
  // that also our test list is sorted the same way.
  for (int i = 1; i < HFTS.size(); i++)
    REQUIRE(HFTS[i - 1]->name < HFTS[i]->name);

  Datapoint datapoint;
  datapoint.aggregation_unit = AGG_UNIT;

  SECTION("Error on non-annual interval") {
    for (int day = 0; day < 366; day++)
      for (int year = YEAR; year < YEAR + 3; year++) {
        datapoint.interval = DateInterval(Date(0, YEAR), Date(day, year));
        if ((day != 364 && day != 365) || year != YEAR)
          CHECK_THROWS(writer.write_datapoint(datapoint));
      }
  }

  // Correct output interval.
  datapoint.interval = DateInterval(Date(0, YEAR), Date(364, YEAR));

  SECTION("Error on empty data") {
    datapoint.data.datapoint_count = 0;
    REQUIRE_THROWS(writer.write_datapoint(datapoint));
  }

  // Fill data with some arbitrary numbers.
  // -> Set more variables for tests for new output tables.
  datapoint.data.hft_data[HFTS[0].get()->name].massdens = 10.0;
  datapoint.data.hft_data[HFTS[1].get()->name].massdens = 16.0;
  datapoint.data.hft_data[HFTS[2].get()->name].massdens = 29.0;
  datapoint.data.hft_data[HFTS[0].get()->name].eaten_forage_per_ind = 10.0;
  datapoint.data.hft_data[HFTS[1].get()->name].eaten_forage_per_ind = 16.0;
  datapoint.data.hft_data[HFTS[2].get()->name].eaten_forage_per_ind = 29.0;
  datapoint.data.datapoint_count = 1;

  // The first call should create captions.
  REQUIRE(datapoint.data.datapoint_count > 0);
  REQUIRE_NOTHROW(writer.write_datapoint(datapoint));

  SECTION("Error on extra HFT") {
    // Try to write a second line with a datapoint where a new HFT suddenly
    // appeared.
    datapoint.data.hft_data[HFTS[3].get()->name].massdens = 12.0;
    CHECK_THROWS(writer.write_datapoint(datapoint));
  }

  SECTION("Illegal Aggregation Unit: Whitespace") {
    datapoint.aggregation_unit = "agg unit";
    REQUIRE(datapoint.data.datapoint_count > 0);
    CHECK_THROWS(writer.write_datapoint(datapoint));
  }

  SECTION("Illegal Aggregation Unit: Delimiter") {
    datapoint.aggregation_unit =
        (std::string) "aggunit" + TextTableWriter::FIELD_SEPARATOR;
    REQUIRE(datapoint.data.datapoint_count > 0);
    CHECK_THROWS(writer.write_datapoint(datapoint));
  }

  // mass_density_per_hft_path
  SECTION("mass_density_per_hft_path") {
    const std::string mass_density_per_hft_path =
        opt.directory + '/' + "mass_density_per_hft" +
        TextTableWriter::FILE_EXTENSION;

    std::ifstream mass_density_per_hft(mass_density_per_hft_path);
    REQUIRE(mass_density_per_hft.good());

    INFO((std::string) "Random output directory: " + opt.directory);
    // Check column captions
    {
      std::string line;
      REQUIRE(std::getline(mass_density_per_hft, line));
      std::vector<std::string> fields =
          split(line, TextTableWriter::FIELD_SEPARATOR);

      INFO((std::string) "line: " + line);
      REQUIRE(fields.size() == 5);

      CHECK(fields[0] == "year");
      CHECK(fields[1] == "agg_unit");
      CHECK(fields[2] == HFTS[0]->name);
      CHECK(fields[3] == HFTS[1]->name);
      CHECK(fields[4] == HFTS[2]->name);
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
      CHECK(fields[1] == AGG_UNIT);
      REQUIRE_NOTHROW(hft1 = std::stod(fields[2]));
      REQUIRE_NOTHROW(hft2 = std::stod(fields[3]));
      REQUIRE_NOTHROW(hft3 = std::stod(fields[4]));

      CHECK(year == YEAR);
      auto &hd = datapoint.data.hft_data;
      CHECK(hft1 == Approx(hd[HFTS[0].get()->name].massdens));
      CHECK(hft2 == Approx(hd[HFTS[1].get()->name].massdens));
      CHECK(hft3 == Approx(hd[HFTS[2].get()->name].massdens));
    }
  }

  // eaten_forage_per_ind
  SECTION("eaten_forage_per_ind") {
    const std::string eaten_forage_per_ind_path =
        opt.directory + '/' + "eaten_forage_per_ind" +
        TextTableWriter::FILE_EXTENSION;

    std::ifstream eaten_forage_per_ind(eaten_forage_per_ind_path);
    REQUIRE(eaten_forage_per_ind.good());
    datapoint.interval = DateInterval(Date(0, YEAR), Date(364, YEAR));

    // Check column captions
    {
      std::string line;
      REQUIRE(std::getline(eaten_forage_per_ind, line));
      std::vector<std::string> fields =
          split(line, TextTableWriter::FIELD_SEPARATOR);

      INFO((std::string) "line: " + line);
      REQUIRE(fields.size() == 6);

      CHECK(fields[0] == "year");
      CHECK(fields[1] == "agg_unit");
      CHECK(fields[2] == "forage_type");
      CHECK(fields[3] == HFTS[0]->name);
      CHECK(fields[4] == HFTS[1]->name);
      CHECK(fields[5] == HFTS[2]->name);
    }

    // Check tuple: one row for each forage type.
    for (const auto &ft : FORAGE_TYPES) {
      std::string line;
      REQUIRE(std::getline(eaten_forage_per_ind, line));

      std::vector<std::string> fields =
          split(line, TextTableWriter::FIELD_SEPARATOR);

      INFO((std::string) "line: " + line);
      REQUIRE(fields.size() == 6);

      int year;
      double hft1, hft2, hft3;
      REQUIRE_NOTHROW(year = std::stoi(fields[0]));
      CHECK(fields[1] == AGG_UNIT);
      REQUIRE(fields[2] == get_forage_type_name(ft));
      REQUIRE_NOTHROW(hft1 = std::stod(fields[3]));
      REQUIRE_NOTHROW(hft2 = std::stod(fields[4]));
      REQUIRE_NOTHROW(hft3 = std::stod(fields[5]));

      CHECK(year == YEAR);
      auto &hd = datapoint.data.hft_data;
      CHECK(hft1 == Approx(hd[HFTS[0].get()->name].eaten_forage_per_ind[ft]));
      CHECK(hft2 == Approx(hd[HFTS[1].get()->name].eaten_forage_per_ind[ft]));
      CHECK(hft3 == Approx(hd[HFTS[2].get()->name].eaten_forage_per_ind[ft]));
    }
  }

  // It is probably good enough to only check for the annual output. The other
  // output schemes are very similarly implemented.

  // Delete directory recursively.
  if (directory_exists(opt.directory)) remove_directory(opt.directory);
}
