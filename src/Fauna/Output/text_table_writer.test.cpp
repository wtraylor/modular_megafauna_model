#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include "catch.hpp"
#include "datapoint.h"
#include "text_table_writer.h"

using namespace Fauna;
using namespace Fauna::Output;

namespace {

/// Check if a directory exists.
bool dir_exists(const std::string& path) {
  struct stat stats;
  stat(path.c_str(), &stats);
  return S_ISDIR(stats.st_mode);
}

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

  SECTION("Annual") {
    TextTableWriter writer(OutputInterval::OI_ANNUAL, opt);

    Datapoint datapoint;
    datapoint.aggregation_unit = "unit1";
    datapoint.first_day = Date(0, 0);
    datapoint.last_day = Date(0, 1);

    // The first call should create directory & files.
    writer.write_datapoint(datapoint);

    REQUIRE(dir_exists(opt.output_directory));

    const std::string mass_density_per_hft_path =
        opt.output_directory + '/' + "mass_density_per_hft" +
        TextTableWriter::FILE_EXTENSION;

    std::ifstream mass_density_per_hft(mass_density_per_hft_path);
    REQUIRE(mass_density_per_hft.good());

    // TODO: Check if headers were created.
    // TODO: Check if tuples are created correctly.
  }

  // TODO: Write tests for other intervals.
}
