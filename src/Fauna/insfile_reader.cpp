/**
 * \file
 * \brief Read the instruction file with HFTs and global parameters.
 * \copyright ...
 * \date 2019
 */
#include "insfile_reader.h"
#include <algorithm>
#include <string>
#include "hft.h"

using namespace Fauna;

namespace {
/// Convert given string to lowercase.
std::string lowercase(const std::string& s) {
  std::string result;
  std::transform(s.begin(), s.end(), std::back_inserter(result), ::tolower);
  return result;
}

}  // namespace

//============================================================
// invalid_option
//============================================================

std::string invalid_option::construct_message(
    const std::string& key, const std::string& value,
    const std::set<std::string>& valid_options) {
  if (valid_options.empty())
    throw std::logic_error(
        "Fauna::invalid_option No set of valid options provided.");
  std::stringstream msg;
  msg << "Invalid option for parameter \"" << key << "\"\n";
  msg << "This was in the instruction file: \"" << value << "\"\n";
  msg << "These are the valid options:";
  for (const auto& s : valid_options) msg << " \"" << s << "\"";
  return msg.str();
}

//============================================================
// InsfileReader
//============================================================

InsfileReader::InsfileReader(const std::string filename)
    : ins(cpptoml::parse_file(filename)) {
  // Read global parameters
  read_table_output();
  if (params.output_format == OutputFormat::TextTables)
    read_table_output_text_tables();
  read_table_simulation();
}

void InsfileReader::read_table_output() {
  {
    const auto key = "output.format";
    auto value = ins->get_qualified_as<std::string>(key);
    if (value) {
      if (lowercase(*value) == lowercase("TextTables"))
        params.output_format = OutputFormat::TextTables;
      // -> Add new output formats here.
      else
        throw invalid_option(key, *value, {"TextTables"});
    } else
      throw missing_parameter(key);
  }
  {
    const auto key = "output.interval";
    auto value = ins->get_qualified_as<std::string>(key);
    if (value) {
      if (lowercase(*value) == lowercase("Daily"))
        params.output_interval = OutputInterval::Daily;
      else if (lowercase(*value) == lowercase("Monthly"))
        params.output_interval = OutputInterval::Monthly;
      else if (lowercase(*value) == lowercase("Annual"))
        params.output_interval = OutputInterval::Annual;
      else if (lowercase(*value) == lowercase("Decadal"))
        params.output_interval = OutputInterval::Decadal;
      else
        throw invalid_option(key, *value,
                             {"Daily", "Monthly", "Decadal", "Annual"});
    } else
      throw missing_parameter(key);
  }
}

void InsfileReader::read_table_output_text_tables() {
  {
    const auto key = "output.text_tables.directory";
    auto value = ins->get_qualified_as<std::string>(key);
    if (value) {
      params.text_table_output.output_directory = *value;
    } else
      throw missing_parameter(key);
  }
  {
    const auto key = "output.text_tables.precision";
    auto value = ins->get_qualified_as<int>(key);
    if (value) {
      params.text_table_output.precision = *value;
    }
  }
  {
    const auto key = "output.text_tables.tables";
    auto value = ins->get_qualified_array_of<std::string>(key);
    if (value) {
      for (const auto& s : *value)
        if (lowercase(s) == "mass_density_per_hft")
          params.text_table_output.mass_density_per_hft = true;
        // -> Add new output tables here.
        else
          throw invalid_option(key, s, {"mass_density_per_hft"});
    }
  }
}

void InsfileReader::read_table_simulation() {
  {
    const auto key = "simulation.forage_distribution";
    auto value = ins->get_qualified_as<std::string>(key);
    if (value) {
      if (lowercase(*value) == lowercase("Equally"))
        params.forage_distribution = ForageDistributionAlgorithm::Equally;
      // -> Add new forage distribution algorithm here.
      else
        throw invalid_option(key, *value, {"Equally"});
    }
  }
  {
    const auto key = "simulation.habitat_area_km2";
    auto value = ins->get_qualified_as<double>(key);
    if (value) {
      params.habitat_area_km2 = *value;
    }
  }
  {
    const auto key = "simulation.establishment_interval";
    auto value = ins->get_qualified_as<int>(key);
    if (value) {
      params.herbivore_establish_interval = *value;
    }
  }
  {
    const auto key = "simulation.herbivore_type";
    auto value = ins->get_qualified_as<std::string>(key);
    if (value) {
      if (lowercase(*value) == lowercase("Cohort"))
        params.herbivore_type = HerbivoreType::Cohort;
      else if (lowercase(*value) == lowercase("Individual"))
        params.herbivore_type = HerbivoreType::Individual;
      else
        throw invalid_option(key, *value, {"Cohort", "Individual"});
    } else
      throw missing_parameter(key);
  }
  {
    const auto key = "simulation.one_hft_per_patch";
    auto value = ins->get_qualified_as<bool>(key);
    if (value) {
      params.one_hft_per_patch = *value;
    }
  }
}
