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
  if (params.output.format == OutputFormat::TextTables)
    read_table_output_text_tables();
  read_table_simulation();

  auto hft_table_array = ins->get_table_array("hft");
  if (hft_table_array)
    for (const auto& hft_table : *hft_table_array)
      hfts.insert(read_hft(hft_table));
  else
    throw std::runtime_error("No HFTs provided.");
}

Hft InsfileReader::read_hft(const std::shared_ptr<cpptoml::table>& table) {
  Hft hft;
  {
    const auto key = "name";
    auto value = table->get_as<std::string>("name");
    if (value)
      hft.name = *value;
    else
      throw missing_parameter("hft.name");
  }

  // Section "body_fat"
  {
    const auto key = "body_fat.birth";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_fat.birth = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "body_fat.deviation";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_fat.deviation = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "body_fat.maximum";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_fat.maximum = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "body_fat.maximum_daily_gain";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_fat.maximum_daily_gain = *value;
    else
      throw missing_parameter(hft, key);
  }
  // Section "body_mass"
  {
    const auto key = "body_mass.birth";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_mass.birth = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "body_mass.female";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_mass.female = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "body_mass.male";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.body_mass.male = *value;
    else
      throw missing_parameter(hft, key);
  }
  // Section "breeding_season"
  {
    const auto key = "breeding_season.length";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.breeding_season.length = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "breeding_season.start";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.breeding_season.start = *value;
    else
      throw missing_parameter(hft, key);
  }
  // Section "digestion"
  {
    const auto key = "digestion.limit";
    const auto value = table->get_qualified_as<std::string>(key);
    if (value)
      if (lowercase(*value) == lowercase("None"))
        hft.digestion.limit = DigestiveLimit::None;
      else if (lowercase(*value) == lowercase("Allometric"))
        hft.digestion.limit = DigestiveLimit::Allometric;
      else if (lowercase(*value) == lowercase("FixedFraction"))
        hft.digestion.limit = DigestiveLimit::FixedFraction;
      else if (lowercase(*value) == lowercase("IlliusGordon1992"))
        hft.digestion.limit = DigestiveLimit::IlliusGordon1992;
      else
        invalid_option(
            hft, key, *value,
            {"None", "Allometric", "FixedFraction", "IlliusGordon1992"});
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "digestion.type";
    const auto value = table->get_qualified_as<std::string>(key);
    if (value)
      if (lowercase(*value) == lowercase("Hindgut"))
        hft.digestion.type = DigestionType::Hindgut;
      else if (lowercase(*value) == lowercase("Ruminant"))
        hft.digestion.type = DigestionType::Ruminant;
      else
        invalid_option(hft, key, *value, {"Hindgut", "Ruminant"});
    else
      throw missing_parameter(hft, key);
  }
  if (hft.digestion.limit == DigestiveLimit::FixedFraction) {
    const auto key = "digestion.fixed_fraction";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.digestion.fixed_fraction = *value;
    else
      throw missing_parameter(hft, key);
  }
  if (hft.digestion.limit == DigestiveLimit::Allometric) {
    const auto key = "digestion.allometric.coefficient";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.digestion.allometric.coefficient = *value;
    else
      throw missing_parameter(hft, key);
  }
  if (hft.digestion.limit == DigestiveLimit::Allometric) {
    const auto key = "digestion.allometric.exponent";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.digestion.allometric.exponent = *value;
    else
      throw missing_parameter(hft, key);
  }
  // Section "establishment"
  {
    const auto key = "establishment.age_range.first";
    const auto value = table->get_qualified_as<int>(key);
    if (value)
      hft.establishment.age_range.first = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "establishment.age_range.last";
    const auto value = table->get_qualified_as<int>(key);
    if (value)
      hft.establishment.age_range.second = *value;
    else
      throw missing_parameter(hft, key);
  }
  {
    const auto key = "establishment.density";
    const auto value = table->get_qualified_as<double>(key);
    if (value)
      hft.establishment.density = *value;
    else
      throw missing_parameter(hft, key);
  }
  // Section "expenditure"
  // Section "foraging"
  // Section "life_history"
  // Section "mortality"
  // Section "reproduction"
  // Section "thermoregulation"
  return hft;
}
void InsfileReader::read_table_output() {
  {
    const auto key = "output.format";
    auto value = ins->get_qualified_as<std::string>(key);
    if (value) {
      if (lowercase(*value) == lowercase("TextTables"))
        params.output.format = OutputFormat::TextTables;
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
        params.output.interval = OutputInterval::Daily;
      else if (lowercase(*value) == lowercase("Monthly"))
        params.output.interval = OutputInterval::Monthly;
      else if (lowercase(*value) == lowercase("Annual"))
        params.output.interval = OutputInterval::Annual;
      else if (lowercase(*value) == lowercase("Decadal"))
        params.output.interval = OutputInterval::Decadal;
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
      params.text_table_output.directory = *value;
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
