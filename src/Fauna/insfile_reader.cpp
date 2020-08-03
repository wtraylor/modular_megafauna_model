/**
 * \file
 * \brief Read the instruction file with HFTs and global parameters.
 * \copyright ...
 * \date 2019
 */
#include "insfile_reader.h"
#include <algorithm>
#include <string>
#include "forage_types.h"
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
  read_table_forage();
  read_table_output();
  if (params.output_format == OutputFormat::TextTables)
    read_table_output_text_tables();
  read_table_simulation();

  {
    std::string err_msg;
    if (!params.is_valid(err_msg))
      throw std::runtime_error("Parameters are not valid:\n" + err_msg);
  }

  if (params.herbivore_type == HerbivoreType::Cohort) {
    auto hft_table_array = ins->get_table_array("hft");
    if (hft_table_array) {
      for (const auto& hft_table : *hft_table_array) {
        std::shared_ptr<const Hft> new_hft(new Hft(read_hft(hft_table)));
        std::string err_msg;
        if (!new_hft->is_valid(params, err_msg))
          throw std::runtime_error("HFT \"" + new_hft->name +
                                   "\" is not valid:\n" + err_msg);
        // Add HFT to list, but check if HFT with that name already exists.
        for (const auto& hft : hfts) {
          assert(hft.get());
          if (hft->name == new_hft->name)
            throw std::runtime_error("HFT with name \"" + hft->name +
                                     "\" is defined twice.");
        }
        hfts.push_back(new_hft);
      }
    } else
      // If there are no HFTs, then the groups are just ignored and not checked
      // for invalid keys.
      ins->erase("group");

    // Erase all valid HFT keys in the groups so that only invalid/unknown keys
    // remain.
    auto group_table_array = ins->get_table_array("group");
    if (group_table_array)
      for (auto group_table : *group_table_array)
        for (const std::string& key : hft_keys_parsed) {
          if (group_table->contains_qualified(key))
            remove_qualified_key(group_table, key);
        }
  } else {
    // Don’t parse HFTs and HFT groups.
    ins->erase("hft");
    ins->erase("group");
  }

  // Whenever a parameter was parsed, it got deleted from the TOML table `ins`.
  // Now that all known parameters have been parsed, `ins` should be empty. For
  // any remaining parameter, an exception will be thrown.
  auto unknown_elements = get_all_keys(ins);
  if (!unknown_elements.empty()) throw unknown_parameters(unknown_elements);
}

template <class Expected>
void InsfileReader::check_wrong_type(const std::string& key) const {
  // User-readable label for the expected type.
  std::string expected;
  if (std::is_same<Expected, bool>::value)
    expected = "boolean";
  else if (std::is_same<Expected, double>::value)
    expected = "floating point";
  else if (std::is_same<Expected, std::vector<double>>::value)
    expected = "array of floating point";
  else if (std::is_same<Expected, int>::value)
    expected = "integer";
  else if (std::is_same<Expected, std::string>::value)
    expected = "string";
  else if (std::is_same<Expected, std::vector<std::string>>::value)
    expected = "array of string";
  // Add more types here if you want to support them.
  assert(expected != "");

  // The wrong datatype found in the TOML file.
  std::string found;

  // Check for each possible type if we can find the parameter with that name
  // in the TOML file.
  if (!std::is_same<Expected, bool>::value && ins->get_qualified_as<bool>(key))
    found = "boolean";

  if (!std::is_same<Expected, double>::value &&
      ins->get_qualified_as<double>(key))
    found = "floating point";

  if (!std::is_same<Expected, int>::value && ins->get_qualified_as<int>(key))
    found = "integer";

  if (!std::is_same<Expected, std::string>::value) {
    if (ins->get_qualified_as<std::string>(key)) found = "string";
    if (ins->get_qualified_array_of<std::string>(key))
      found = "array of string";
  }
  // Add more types here if you want to support them.

  if (found != "") throw wrong_param_type(key, expected, found);
}
// Explicit instantiations of the template function for all supported datatypes:
template void InsfileReader::check_wrong_type<bool>(const std::string&) const;
template void InsfileReader::check_wrong_type<double>(const std::string&) const;
template void InsfileReader::check_wrong_type<int>(const std::string&) const;
template void InsfileReader::check_wrong_type<std::string>(
    const std::string&) const;

std::shared_ptr<cpptoml::table> InsfileReader::get_group_table(
    const std::string& group_name) const {
  auto group_table_array = ins->get_table_array("group");
  if (group_table_array)
    for (const auto& group_table : *group_table_array) {
      const auto name = group_table->get_as<std::string>("name");
      if (!name) throw missing_parameter("group.name");
      if (*name == group_name) return group_table;
    }
  return std::shared_ptr<cpptoml::table>(NULL);  // nothing found
}

template <class T>
std::shared_ptr<T> InsfileReader::get_value(
    const std::shared_ptr<cpptoml::table>& table, const std::string& key,
    const GetValueOpt opt) const {
  if (!table)
    throw std::invalid_argument(
        "Fauna::InsfileReader::get_value() "
        "Parameter 'table' is NULL.");
  auto value = table->get_qualified_as<T>(key);
  if (value) {
    if (opt == GetValueOpt::RemoveKey) remove_qualified_key(table, key);
    return std::shared_ptr<T>(new T(*value));
  } else {
    check_wrong_type<T>(key);
    return NULL;  // Nothing found.
  }
}
// Explicit instantiations of the template function for all supported datatypes:
template std::shared_ptr<bool> InsfileReader::get_value<>(
    const std::shared_ptr<cpptoml::table>&, const std::string& key,
    const GetValueOpt) const;
template std::shared_ptr<double> InsfileReader::get_value<>(
    const std::shared_ptr<cpptoml::table>&, const std::string& key,
    const GetValueOpt) const;
template std::shared_ptr<int> InsfileReader::get_value<>(
    const std::shared_ptr<cpptoml::table>&, const std::string& key,
    const GetValueOpt) const;
template std::shared_ptr<std::string> InsfileReader::get_value<>(
    const std::shared_ptr<cpptoml::table>&, const std::string& key,
    const GetValueOpt) const;

template <class T>
std::shared_ptr<std::vector<T>> InsfileReader::get_value_array(
    const std::shared_ptr<cpptoml::table>& table, const std::string& key,
    const GetValueOpt opt) const {
  if (!table)
    throw std::invalid_argument(
        "Fauna::InsfileReader::get_value() "
        "Parameter 'table' is NULL.");
  auto value = table->get_qualified_array_of<T>(key);
  if (value) {
    if (opt == GetValueOpt::RemoveKey) remove_qualified_key(table, key);
    return std::shared_ptr<std::vector<T>>(new std::vector<T>(*value));
  } else {
    // Look for a single value and treat it as a one-element array.
    auto single = get_value<T>(table, key, opt);
    if (single) {
      return std::shared_ptr<std::vector<T>>(new std::vector<T>({*single}));
    } else {
      check_wrong_type<std::vector<T>>(key);
      return NULL;  // Nothing found.
    }
  }
}
// Explicit instantiations of the template function for all supported datatypes:
template std::shared_ptr<std::vector<std::string>>
InsfileReader::get_value_array<>(const std::shared_ptr<cpptoml::table>&,
                                 const std::string&, const GetValueOpt) const;

template <class T>
std::shared_ptr<T> InsfileReader::find_hft_parameter(
    const std::shared_ptr<cpptoml::table>& hft_table, const std::string& key,
    const bool mandatory) {
  assert(hft_table->get_as<std::string>("name"));  // Name must be defined.
  const std::string name = *hft_table->get_as<std::string>("name");

  // Mark this key as a valid key.
  hft_keys_parsed.insert(key);

  {
    const auto value = get_value<T>(hft_table, key);
    if (value) return value;
  }

  const auto groups =
      get_value_array<std::string>(hft_table, "groups", GetValueOpt::KeepKey);
  if (groups)
    for (const auto& g : *groups) {
      const auto group_table = get_group_table(g);
      if (!group_table) throw missing_group(name, g);
      {
        const auto value = get_value<T>(group_table, key, GetValueOpt::KeepKey);
        if (value) return value;
      }
    }

  // If we reach this point, nothing has been found.
  if (mandatory)
    throw missing_hft_parameter(name, key);
  else
    return NULL;
}

template <class T>
std::shared_ptr<std::vector<T>> InsfileReader::find_hft_array_parameter(
    const std::shared_ptr<cpptoml::table>& hft_table, const std::string& key,
    const bool mandatory) {
  assert(hft_table->get_as<std::string>("name"));  // Name must be defined.
  const std::string name = *hft_table->get_as<std::string>("name");

  // Mark this key as a valid key.
  hft_keys_parsed.insert(key);

  {
    // Look for an array of values in the "hft" section.
    const auto array = get_value_array<T>(hft_table, key);
    if (array) return array;
  }

  const auto groups =
      get_value_array<std::string>(hft_table, "groups", GetValueOpt::KeepKey);
  if (groups)
    for (const auto& g : *groups) {
      const auto group_table = get_group_table(g);
      if (!group_table) throw missing_group(name, g);
      {
        // Look for an array of values in the "groups" section.
        const auto array =
            get_value_array<T>(group_table, key, GetValueOpt::KeepKey);
        if (array) return array;
      }
    }

  // If we reach this point, nothing has been found.
  if (mandatory)
    throw missing_hft_parameter(name, key);
  else {
    return NULL;
  }
}

std::list<std::string> InsfileReader::get_all_keys(
    std::shared_ptr<cpptoml::table> table) {
  std::list<std::string> l;
  if (!table) return l;
  for (const auto i : *table) {
    const std::string key = i.first;
    if (i.second->is_value() || i.second->is_array())
      l.push_back(key);
    else if (i.second->is_table()) {
      // Add all elements from subtree by prefixing it with the name of the
      // table.
      for (auto j : get_all_keys(i.second->as_table()))
        l.push_back(i.first + '.' + j);
    } else if (i.second->is_table_array()) {
      for (auto t : *i.second->as_table_array())
        for (auto j : get_all_keys(t)) l.push_back(i.first + '.' + j);
    }
  }
  return l;
}

Hft InsfileReader::read_hft(const std::shared_ptr<cpptoml::table>& table) {
  if (!table)
    throw std::invalid_argument(
        "Fauna::InsfileReader::read_hft() "
        "Parameter 'table' is NULL.");
  Hft hft;
  {
    const auto key = "name";
    auto value = table->get_as<std::string>("name");
    if (value)
      hft.name = *value;
    else
      throw missing_parameter("hft.name");
    // Don’t erase "name" from TOML table yet because we need it for potential
    // error messages.
  }

  // ======= MANDATORY PARAMETERS =======
  {
    const auto value = find_hft_parameter<double>(
        table, "body_fat.catabolism_efficiency", true);
    assert(value);
    hft.body_fat_catabolism_efficiency = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "body_fat.gross_energy", true);
    assert(value);
    hft.body_fat_gross_energy = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "body_fat.maximum", true);
    assert(value);
    hft.body_fat_maximum = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "body_fat.maximum_daily_gain", true);
    assert(value);
    hft.body_fat_maximum_daily_gain = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "body_mass.empty", true);
    assert(value);
    hft.body_mass_empty = *value;
  }
  {
    const auto value = find_hft_parameter<int>(table, "body_mass.female", true);
    assert(value);
    hft.body_mass_female = *value;
  }
  {
    const auto value = find_hft_parameter<int>(table, "body_mass.male", true);
    assert(value);
    hft.body_mass_male = *value;
  }
  {
    const auto value =
        find_hft_parameter<std::string>(table, "digestion.limit", true);
    assert(value);
    if (lowercase(*value) == lowercase("None"))
      hft.digestion_limit = DigestiveLimit::None;
    else if (lowercase(*value) == lowercase("Allometric"))
      hft.digestion_limit = DigestiveLimit::Allometric;
    else if (lowercase(*value) == lowercase("FixedFraction"))
      hft.digestion_limit = DigestiveLimit::FixedFraction;
    else if (lowercase(*value) == lowercase("IlliusGordon1992"))
      hft.digestion_limit = DigestiveLimit::IlliusGordon1992;
    else
      throw invalid_option(
          hft, "digestion.limit", *value,
          {"None", "Allometric", "FixedFraction", "IlliusGordon1992"});
  }
  {
    const auto value =
        find_hft_parameter<int>(table, "establishment.age_range.first", true);
    assert(value);
    hft.establishment_age_range.first = *value;
  }
  {
    const auto value =
        find_hft_parameter<int>(table, "establishment.age_range.last", true);
    assert(value);
    hft.establishment_age_range.second = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "establishment.density", true);
    assert(value);
    hft.establishment_density = *value;
  }
  {
    const auto array = find_hft_array_parameter<std::string>(
        table, "expenditure.components", true);
    assert(array);
    hft.expenditure_components = {};
    for (const auto& i : *array)
      if (lowercase(i) == lowercase("BasalMetabolicRate"))
        hft.expenditure_components.insert(
            ExpenditureComponent::BasalMetabolicRate);
      else if (lowercase(i) == lowercase("FieldMetabolicRate"))
        hft.expenditure_components.insert(
            ExpenditureComponent::FieldMetabolicRate);
      else if (lowercase(i) == lowercase("Taylor1981"))
        hft.expenditure_components.insert(ExpenditureComponent::Taylor1981);
      else if (lowercase(i) == lowercase("Thermoregulation"))
        hft.expenditure_components.insert(
            ExpenditureComponent::Thermoregulation);
      else if (lowercase(i) == lowercase("Zhu2018"))
        hft.expenditure_components.insert(ExpenditureComponent::Zhu2018);
      else
        throw invalid_option(hft, "expenditure.components", i,
                             {"BasalMetabolicRate", "FieldMetabolicRate",
                              "Taylor1981", "Thermoregulation", "Zhu2018"});
    if (table->get_table("expenditure"))
      table->get_table("expenditure")->erase("components");
  }
  {
    const auto value =
        find_hft_parameter<std::string>(table, "foraging.diet_composer", true);
    assert(value);
    if (lowercase(*value) == lowercase("PureGrazer"))
      hft.foraging_diet_composer = DietComposer::PureGrazer;
    else
      throw invalid_option(hft, "foraging.diet_composer", *value,
                           {"PureGrazer"});
  }
  {
    const auto value = find_hft_parameter<double>(
        table, "digestion.digestibility_multiplier", false);
    if (value) hft.digestion_digestibility_multiplier = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "digestion.k_fat", true);
    assert(value);
    hft.digestion_k_fat = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "digestion.k_maintenance", true);
    assert(value);
    hft.digestion_k_maintenance = *value;
  }
  {
    const auto value =
        find_hft_parameter<double>(table, "digestion.me_coefficient", true);
    assert(value);
    hft.digestion_me_coefficient = *value;
  }
  {
    const auto value = find_hft_parameter<std::string>(
        table, "digestion.net_energy_model", true);
    assert(value);
    if (lowercase(*value) == lowercase("GrossEnergyFraction"))
      hft.digestion_net_energy_model = NetEnergyModel::GrossEnergyFraction;
    else
      throw invalid_option(hft, "digestion.net_energy_model", *value,
                           {"GrossEnergyFraction"});
  }
  {
    const auto array = find_hft_array_parameter<std::string>(
        table, "mortality.factors", false);
    hft.mortality_factors = {};
    if (array) {
      for (const auto& i : *array)
        if (lowercase(i) == lowercase("Background"))
          hft.mortality_factors.insert(MortalityFactor::Background);
        else if (lowercase(i) == lowercase("Lifespan"))
          hft.mortality_factors.insert(MortalityFactor::Lifespan);
        else if (lowercase(i) == lowercase("StarvationIlliusOConnor2000"))
          hft.mortality_factors.insert(
              MortalityFactor::StarvationIlliusOConnor2000);
        else if (lowercase(i) == lowercase("StarvationThreshold"))
          hft.mortality_factors.insert(MortalityFactor::StarvationThreshold);
        else
          throw invalid_option(
              hft, "mortality.factors", i,
              {"Background", "Lifespan", "StarvationIlliusOConnor2000",
               "StarvationThreshold"});
    }
  }
  {
    const auto value =
        find_hft_parameter<std::string>(table, "reproduction.model", true);
    assert(value);
    if (lowercase(*value) == lowercase("None")) {
      hft.reproduction_model = ReproductionModel::None;
      hft.life_history_physical_maturity_female = 1;
      hft.life_history_physical_maturity_male = 1;
    } else if (lowercase(*value) == lowercase("ConstantMaximum"))
      hft.reproduction_model = ReproductionModel::ConstantMaximum;
    else if (lowercase(*value) == lowercase("Logistic"))
      hft.reproduction_model = ReproductionModel::Logistic;
    else if (lowercase(*value) == lowercase("Linear"))
      hft.reproduction_model = ReproductionModel::Linear;
    else
      throw invalid_option(hft, "reproduction.model", *value,
                           {"None", "ConstantMaximum", "Logistic", "Linear"});
  }

  // ======== NON-MANDATORY PARAMETERS =======
  {
    const auto array =
        find_hft_array_parameter<std::string>(table, "foraging.limits", false);
    hft.foraging_limits = {};
    if (array) {
      for (const auto& i : *array)
        if (lowercase(i) == lowercase("GeneralFunctionalResponse"))
          hft.foraging_limits.insert(ForagingLimit::GeneralFunctionalResponse);
        else if (lowercase(i) == lowercase("IlliusOConnor2000"))
          hft.foraging_limits.insert(ForagingLimit::IlliusOConnor2000);
        else
          throw invalid_option(
              hft, "foraging.limits", i,
              {"GeneralFunctionalResponse", "IlliusOConnor2000"});
    }
  }

  // ======== DEPENDENT PARAMETERS =======
  //
  // A local variable `mandatory` determines whether the parameter is required.
  // The function find_hft_parameter() is called regardless in order to ensure
  // that the TOML element gets erased from the instruction file in any case.
  // Erasing the element marks it as parsed. If it isn’t erased, an exception
  // for an unknown parameter will be thrown.

  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value =
        find_hft_parameter<double>(table, "body_fat.birth", mandatory);
    if (value) hft.body_fat_birth = *value;
  }
  {
    const bool mandatory = (hft.mortality_factors.count(
        MortalityFactor::StarvationIlliusOConnor2000));
    const auto value =
        find_hft_parameter<double>(table, "body_fat.deviation", mandatory);
    if (value) hft.body_fat_deviation = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value =
        find_hft_parameter<int>(table, "body_mass.birth", mandatory);
    if (value) hft.body_mass_birth = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value =
        find_hft_parameter<int>(table, "breeding_season.length", mandatory);
    if (value) hft.breeding_season_length = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value =
        find_hft_parameter<int>(table, "breeding_season.start", mandatory);
    if (value) hft.breeding_season_start = *value;
  }
  {
    const bool mandatory =
        (hft.digestion_limit == DigestiveLimit::FixedFraction);
    const auto value = find_hft_parameter<double>(
        table, "digestion.fixed_fraction", mandatory);
    if (value) hft.digestion_fixed_fraction = *value;
  }
  {
    const bool mandatory = hft.digestion_limit == DigestiveLimit::Allometric;
    const auto value = find_hft_parameter<double>(
        table, "digestion.allometric.fraction_male_adult", mandatory);
    if (value) hft.digestion_allometric.value_male_adult = *value;
  }
  {
    const bool mandatory = (hft.digestion_limit == DigestiveLimit::Allometric);
    const auto value = find_hft_parameter<double>(
        table, "digestion.allometric.exponent", mandatory);
    if (value) hft.digestion_allometric.exponent = *value;
  }
  {
    const bool mandatory =
        (hft.digestion_limit == DigestiveLimit::IlliusGordon1992);
    const auto value = find_hft_array_parameter<double>(
        table, "digestion.i_g_1992_ijk", mandatory);
    if (value) {
      if (value->size() != 3)
        throw bad_array_size("hft.digestion.i_g_1992_ijk", value->size(), "3");
      for (int i = 0; i < 3; i++) hft.digestion_i_g_1992_ijk[i] = (*value)[i];
    }
  }
  {
    const bool mandatory = (hft.expenditure_components.count(
                                ExpenditureComponent::BasalMetabolicRate) ||
                            hft.expenditure_components.count(
                                ExpenditureComponent::FieldMetabolicRate));
    const auto value1 = find_hft_parameter<double>(
        table, "expenditure.basal_rate.mj_per_day_male_adult", mandatory);
    if (value1) hft.expenditure_basal_rate.value_male_adult = *value1;
    const auto value2 = find_hft_parameter<double>(
        table, "expenditure.basal_rate.exponent", mandatory);
    if (value2) hft.expenditure_basal_rate.exponent = *value2;
  }
  {
    const bool mandatory = hft.expenditure_components.count(
        ExpenditureComponent::FieldMetabolicRate);
    const auto value = find_hft_parameter<double>(
        table, "expenditure.fmr_multiplier", mandatory);
    if (value) hft.expenditure_fmr_multiplier = *value;
  }
  {
    const bool mandatory =
        (hft.foraging_limits.count(ForagingLimit::GeneralFunctionalResponse) ||
         hft.foraging_limits.count(ForagingLimit::IlliusOConnor2000));
    const auto value = find_hft_parameter<double>(
        table, "foraging.half_max_intake_density", mandatory);
    if (value) hft.foraging_half_max_intake_density = *value;
  }
  {
    const bool mandatory =
        (hft.mortality_factors.count(MortalityFactor::Lifespan));
    const auto value =
        find_hft_parameter<int>(table, "life_history.lifespan", mandatory);
    if (value) hft.life_history_lifespan = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value = find_hft_parameter<int>(
        table, "life_history.physical_maturity_female", mandatory);
    if (value) hft.life_history_physical_maturity_female = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value = find_hft_parameter<int>(
        table, "life_history.physical_maturity_male", mandatory);
    if (value) hft.life_history_physical_maturity_male = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value = find_hft_parameter<int>(
        table, "life_history.sexual_maturity", mandatory);
    if (value) hft.life_history_sexual_maturity = *value;
  }
  {
    const bool mandatory =
        (hft.mortality_factors.count(MortalityFactor::Background));
    const auto value =
        find_hft_parameter<double>(table, "mortality.adult_rate", mandatory);
    if (value) hft.mortality_adult_rate = *value;
  }
  {
    const bool mandatory =
        (hft.mortality_factors.count(MortalityFactor::Background));
    const auto value =
        find_hft_parameter<double>(table, "mortality.juvenile_rate", mandatory);
    if (value) hft.mortality_juvenile_rate = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value = find_hft_parameter<double>(
        table, "mortality.minimum_density_threshold", mandatory);
    if (value) hft.mortality_minimum_density_threshold = *value;
  }
  {
    const bool mandatory =
        (hft.mortality_factors.count(MortalityFactor::StarvationThreshold) ||
         hft.mortality_factors.count(
             MortalityFactor::StarvationIlliusOConnor2000));
    const auto value = find_hft_parameter<bool>(
        table, "mortality.shift_body_condition_for_starvation", mandatory);
    if (value) hft.mortality_shift_body_condition_for_starvation = *value;
  }
  {
    const bool mandatory =
        (hft.reproduction_model == ReproductionModel::ConstantMaximum ||
         hft.reproduction_model == ReproductionModel::Logistic ||
         hft.reproduction_model == ReproductionModel::Linear);
    const auto value = find_hft_parameter<double>(
        table, "reproduction.annual_maximum", mandatory);
    if (value) hft.reproduction_annual_maximum = *value;
  }
  {
    const bool mandatory = (hft.reproduction_model != ReproductionModel::None);
    const auto value = find_hft_parameter<int>(
        table, "reproduction.gestation_length", mandatory);
    if (value) hft.reproduction_gestation_length = *value;
  }
  {
    const bool mandatory =
        (hft.reproduction_model == ReproductionModel::Logistic);
    // growth_rate
    const auto growth_rate = find_hft_parameter<double>(
        table, "reproduction.logistic.growth_rate", mandatory);
    if (growth_rate) hft.reproduction_logistic[0] = *growth_rate;
    // midpoint
    const auto midpoint = find_hft_parameter<double>(
        table, "reproduction.logistic.midpoint", mandatory);
    if (midpoint) hft.reproduction_logistic[1] = *midpoint;
  }
  {
    const bool mandatory = (hft.expenditure_components.count(
        ExpenditureComponent::Thermoregulation));
    const auto value = find_hft_parameter<std::string>(
        table, "thermoregulation.conductance", mandatory);
    if (value) {
      if (lowercase(*value) == lowercase("BradleyDeavers1980"))
        hft.thermoregulation_conductance = ConductanceModel::BradleyDeavers1980;
      else if (lowercase(*value) == lowercase("CuylerOeritsland2004"))
        hft.thermoregulation_conductance =
            ConductanceModel::CuylerOeritsland2004;
      else
        throw invalid_option(hft, "thermoregulation.conductance", *value,
                             {"BradleyDeavers1980", "CuylerOeritsland2004"});
    }
  }
  {
    const bool mandatory = (hft.expenditure_components.count(
        ExpenditureComponent::Thermoregulation));
    const auto value = find_hft_parameter<double>(
        table, "thermoregulation.core_temperature", mandatory);
    if (value) hft.thermoregulation_core_temperature = *value;
  }

  // The "name" TOML element is erased last because it is used before for
  // potential error messages.
  table->erase("name");
  hft_keys_parsed.insert("name");
  // Now the "groups" TOML element can be erased because it is no longer needed.
  table->erase("groups");

  return hft;
}

void InsfileReader::read_table_forage() {
  auto table = ins->get_table("forage");
  for (const auto& ft : Fauna::FORAGE_TYPES) {
    const auto key = "forage.gross_energy." + get_forage_type_name(ft);
    auto value = get_value<double>(ins, key);
    if (!value) throw missing_parameter(key);
    if (*value < 0)
      throw param_out_of_range(key, std::to_string(*value), "[0,∞)");
    params.forage_gross_energy[ft] = *value;
    if (table && table->get_table("gross_energy"))
      table->get_table("gross_energy")->erase(get_forage_type_name(ft));
  }
  // Remove the table "forage" in order to indicate that it’s been parsed.
  if (table && table->empty()) ins->erase("forage");
}

void InsfileReader::read_table_output() {
  {
    const auto key = "output.format";
    auto value = get_value<std::string>(ins, key);
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
    auto value = get_value<std::string>(ins, key);
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
  // Remove the table "output" in order to indicate that it’s been parsed.
  auto table = ins->get_table("output");
  if (table && table->empty()) ins->erase("output");
}

void InsfileReader::read_table_output_text_tables() {
  {
    const auto key = "output.text_tables.directory";
    auto value = get_value<std::string>(ins, key);
    if (value)
      params.output_text_tables.directory = *value;
    else
      throw missing_parameter(key);
  }
  {
    const auto key = "output.text_tables.precision";
    auto value = get_value<int>(ins, key);
    if (value) params.output_text_tables.precision = *value;
  }
  {
    const auto key = "output.text_tables.tables";
    auto value = get_value_array<std::string>(ins, key);
    if (value) {
      for (const auto& s : *value)
        if (lowercase(s) == "available_forage")
          params.output_text_tables.available_forage = true;
        else if (lowercase(s) == "digestibility")
          params.output_text_tables.digestibility = true;
        else if (lowercase(s) == "eaten_forage_per_ind")
          params.output_text_tables.eaten_forage_per_ind = true;
        else if (lowercase(s) == "eaten_nitrogen_per_ind")
          params.output_text_tables.eaten_nitrogen_per_ind = true;
        else if (lowercase(s) == "mass_density_per_hft")
          params.output_text_tables.mass_density_per_hft = true;
        // -> Add new output tables here (alphabetical order).
        else
          throw invalid_option(
              key, s,
              {"available_forage", "digestibility", "eaten_forage_per_ind",
               "eaten_nitrogen_per_ind", "mass_density_per_hft"});
    }
  }
  // Remove the table "output" in order to indicate that it’s been parsed.
  auto table = ins->get_table_qualified("output.text_tables");
  if (table && table->empty()) ins->erase("output");
}

void InsfileReader::read_table_simulation() {
  {
    const auto key = "simulation.forage_distribution";
    auto value = get_value<std::string>(ins, key);
    if (value) {
      if (lowercase(*value) == lowercase("Equally"))
        params.forage_distribution = ForageDistributionAlgorithm::Equally;
      // -> Add new forage distribution algorithm here.
      else
        throw invalid_option(key, *value, {"Equally"});
    }
  }
  {
    const auto key = "simulation.establishment_interval";
    auto value = get_value<int>(ins, key);
    if (value) params.herbivore_establish_interval = *value;
  }
  {
    const auto key = "simulation.herbivore_type";
    auto value = get_value<std::string>(ins, key);
    if (value) {
      if (lowercase(*value) == lowercase("Cohort"))
        params.herbivore_type = HerbivoreType::Cohort;
      else
        throw invalid_option(key, *value, {"Cohort"});
    } else
      throw missing_parameter(key);
  }
  {
    const auto key = "simulation.one_hft_per_habitat";
    auto value = get_value<bool>(ins, key);
    if (value) params.one_hft_per_habitat = *value;
  }
  // Remove the table "simulation" in order to indicate that it’s been parsed.
  auto table = ins->get_table("simulation");
  if (table && table->empty()) ins->erase("simulation");
}

void InsfileReader::remove_qualified_key(std::shared_ptr<cpptoml::table> table,
                                         const std::string key) const {
  if (!table)
    throw std::invalid_argument(
        "Fauna::InsfileReader::remove_qualified_key() "
        "Parameter 'table' is NULL.");
  if (!table->contains_qualified(key))
    throw std::out_of_range(
        "Fauna::InsfileReader::remove_qualified_key() "
        "TOML key '" +
        key + "' could not be found.");

  // Get the parent TOML table of the parameter. For instance "body_fat.max"
  // has the parent_key "body_fat" and the leaf_key "max".
  auto const pos = key.find_last_of('.');

  if (pos != key.npos) {
    // There is a parent and child element.
    const std::string parent_key = key.substr(0, pos);
    const std::string leaf_key = key.substr(pos + 1);
    assert(leaf_key != "");
    assert(parent_key != "");

    // Remove the key from this HFT table in order to indicate that it has
    // been parsed.
    auto parent = table->get_table_qualified(parent_key);
    if (!parent)
      throw std::out_of_range(
          "Fauna::InsfileReader::remove_qualified_key() "
          "Parent element '" +
          parent_key + "' could not be found in table." + " (key = '" + key +
          "')");
    parent->erase(leaf_key);
  } else
    table->erase(key);
}
