/**
 * \file
 * \brief Herbivore Functional Type (HFT) class and its dependents.
 * \copyright ...
 * \date 2019
 */
#include "hft.h"
#include <sstream>
#include "parameters.h"

using namespace Fauna;

bool Hft::is_valid(const Parameters& params, std::string& msg) const {
  bool is_valid = true;

  // The message text is written into an output string stream
  std::ostringstream stream;

  if (name == "") {
    stream << "name is empty." << std::endl;
    is_valid = false;
  }
  if (name.find(' ') != std::string::npos ||
      name.find(',') != std::string::npos ||
      name.find('_') != std::string::npos) {
    stream << "name contains a forbidden character: ' ' ',' '_'" << std::endl;
    is_valid = false;
  }

  //------------------------------------------------------------
  if (params.herbivore_type == HerbivoreType::Cohort ||
      params.herbivore_type == HerbivoreType::Individual) {
    if (body_fat_birth <= 0.0) {
      stream << "body_fat.birth must be >0.0 (" << body_fat_birth << ")"
             << std::endl;
      is_valid = false;
    }

    if (body_fat_birth > body_fat_maximum) {
      stream << "body_fat.birth must not exceed body_fat.maximum ("
             << body_fat_birth << ")" << std::endl;
      is_valid = false;
    }

    if (body_fat_deviation < 0.0 || body_fat_deviation > 1.0) {
      stream << "body_fat.deviation is out of bounds. (Current value: "
             << body_fat_deviation << ")" << std::endl;
      is_valid = false;
    }

    if (body_fat_maximum <= 0.0 || body_fat_maximum >= 1.0) {
      stream << "body_fat.maximum must be between 0.0 and 1.0"
             << body_fat_maximum << ")" << std::endl;
      is_valid = false;
    }

    if (body_fat_maximum_daily_gain < 0) {
      stream << "`body_fat.maximum_daily_gain` must be >= 0"
             << " (" << body_fat_maximum_daily_gain << ")" << std::endl;
      is_valid = false;
    }

    if (body_fat_maximum_daily_gain > body_fat_maximum) {
      stream << "`body_fat.maximum_daily_gain` cannot be greater than "
                "`body_fat.maximum`."
             << "Note that a value of zero indicates no limits. "
             << " (current value: " << body_fat_maximum_daily_gain << ")"
             << std::endl;
      is_valid = false;
    }

    if (body_mass_birth <= 0.0) {
      stream << "body_mass.birth must be > 0.0 (" << body_fat_birth << ")"
             << std::endl;
      is_valid = false;
    }

    if (body_mass_birth > body_mass_male ||
        body_mass_birth > body_mass_female) {
      stream << "body_mass.birth must not be greater than either "
             << "body_mass.male or body_mass.female (" << body_mass_birth << ")"
             << std::endl;
      is_valid = false;
    }

    if (body_mass_female < 1) {
      stream << "body_mass.female must be >=1 (" << body_mass_female << ")"
             << std::endl;
      is_valid = false;
    }

    if (body_mass_male < 1) {
      stream << "body_mass.male must be >=1 (" << body_mass_male << ")"
             << std::endl;
      is_valid = false;
    }

    if (thermoregulation_core_temperature <= 0.0) {
      stream << "thermoregulation.core_temperature must be >0 ("
             << thermoregulation_core_temperature << ")" << std::endl;
      is_valid = false;
    }

    if (mortality_minimum_density_threshold <= 0.0 ||
        mortality_minimum_density_threshold >= 1.0) {
      stream << "mortality.minimum_density_threshold not between 0 and 1"
             << " (current value: " << mortality_minimum_density_threshold
             << ")";
      is_valid = false;
    }

    if (digestion_limit == DigestiveLimit::None) {
      stream << "No digestive limit defined." << std::endl;
      // the HFT is still valid (e.g. for testing purpose)
    }

    if (digestion_net_energy_model == NetEnergyModel::Default &&
        (digestion_efficiency <= 0.0 || digestion_efficiency > 1.0)) {
      stream << "digestion.efficiency must be in the interval (0,1]."
             << std::endl;
      is_valid = false;
    }

    if (establishment_age_range.first < 0 ||
        establishment_age_range.second < 0) {
      stream << "establishment.age_range must be 2 positive numbers ("
             << establishment_age_range.first << ", "
             << establishment_age_range.second << ")" << std::endl;
      is_valid = false;
    }

    if (establishment_age_range.first > establishment_age_range.second) {
      stream << "First number of `establishment.age_range` must be smaller "
             << " the second number (" << establishment_age_range.first << ", "
             << establishment_age_range.second << ")" << std::endl;
      is_valid = false;
    }

    if (establishment_density <= 0.0) {
      stream << "establishment.density must be >=0.0 (" << establishment_density
             << ")" << std::endl;
      is_valid = false;
    }

    if (params.herbivore_type == HerbivoreType::Individual &&
        establishment_density <= 2.0 / params.habitat_area_km2) {
      stream << "establishment.density (" << establishment_density
             << " ind/km²) "
             << "must not be smaller than two individuals in a habitat"
             << " (habitat_area_km2 = " << params.habitat_area_km2 << " km²)."
             << std::endl;
      is_valid = false;
    }

    if (expenditure_components.empty()) {
      stream << "No energy expenditure components defined." << std::endl;
      is_valid = false;
    }

    if (expenditure_components.count(ExpenditureComponent::Thermoregulation) &&
        expenditure_components.size() == 1) {
      stream << "Thermoregulation is the only expenditure component. "
                "That means that there is no basal metabolism."
             << std::endl;
    }

    if (expenditure_components.count(ExpenditureComponent::Thermoregulation) &&
        expenditure_components.count(ExpenditureComponent::Zhu2018)) {
      stream
          << "Both \"thermoregulation\" and \"zhu_2018\" are chosen as "
             "expenditure components, but the model of Zhu et al. (2018) has "
             "thermoregulation already included."
          << std::endl;
      is_valid = false;
    }

    if (expenditure_components.count(ExpenditureComponent::Allometric) &&
        expenditure_allometric.coefficient < 0.0) {
      stream << "Coefficient for allometric expenditure must not be "
                "negative. That would result in negative expenditure values. "
                "Current value: expenditure_allometric_coefficient = "
             << expenditure_allometric.coefficient << std::endl;
      is_valid = false;
    }

    if (foraging_limits.count(ForagingLimit::IlliusOConnor2000) &&
        foraging_diet_composer != DietComposer::PureGrazer) {
      stream << "`ILLIUS_OCONNOR_2000` is set as a foraging limit and"
                "requires a pure grass diet."
             << std::endl;
      is_valid = false;
    }

    if ((foraging_limits.count(ForagingLimit::IlliusOConnor2000) ||
         foraging_limits.count(ForagingLimit::GeneralFunctionalResponse)) &&
        !(foraging_half_max_intake_density > 0.0)) {
      stream << "foraging.half_max_intake_density must be >0 "
                "if 'IlliusOConnor2000' or 'GeneralFunctionalResponse' "
                "is set in `foraging.limit`."
             << " (current value: " << foraging_half_max_intake_density << ")"
             << std::endl;
      is_valid = false;
    }

    if (foraging_limits.count(ForagingLimit::IlliusOConnor2000) &&
        foraging_limits.count(ForagingLimit::GeneralFunctionalResponse)) {
      stream << "The foraging limits 'IlliusOConnor2000' and "
                "'GeneralFunctionalResponse' are mutually exclusive because "
                "they are functionally equivalent. The former applies a "
                "functional response to maximum energy intake. The latter "
                "applies it to mass intake."
             << std::endl;
      is_valid = false;
    }

    if (reproduction_gestation_length <= 0) {
      stream << "`reproduction.gestation_length` must be a positive number."
             << " (current value: " << reproduction_gestation_length << ")"
             << std::endl;
      is_valid = false;
    }

    if (digestion_anabolism_coefficient <= 0.0) {
      stream << "`digestion.anabolism_coefficient` must be a positive number."
             << " (current value: " << digestion_anabolism_coefficient << ")"
             << std::endl;
      is_valid = false;
    }

    if (digestion_catabolism_coefficient <= 0.0) {
      stream << "`digestion.catabolism_coefficient` must be a positive number."
             << " (current value: " << digestion_catabolism_coefficient << ")"
             << std::endl;
      is_valid = false;
    }

    if (digestion_limit == DigestiveLimit::Allometric &&
        digestion_allometric.coefficient < 0.0) {
      stream << "Coefficient in `digestion.allometric` must not be negative "
                "if 'Allometric' is set as a digestive limit."
             << " (current value: " << digestion_allometric.coefficient << ")"
             << std::endl;
      is_valid = false;
    }

    if (digestion_limit == DigestiveLimit::FixedFraction &&
        (digestion_fixed_fraction <= 0.0 || digestion_fixed_fraction >= 1.0)) {
      stream
          << "Body mass fraction `digestion.fixed_fraction` must be in "
             "interval (0,1) if 'FixedFraction' is set as the digestive limit."
          << " (current value: " << digestion_fixed_fraction << ")"
          << std::endl;
      is_valid = false;
    }

    if (life_history_physical_maturity_female < 1) {
      stream << "life_history.physical_maturity_female must be >=1"
             << " (current value: " << life_history_physical_maturity_female
             << ")" << std::endl;
      is_valid = false;
    }

    if (life_history_physical_maturity_male < 1) {
      stream << "life_history.physical_maturity_male must be >=1"
             << " (" << life_history_physical_maturity_male << ")" << std::endl;
      is_valid = false;
    }

    if (life_history_sexual_maturity < 1) {
      stream << "life_history.sexual_maturity must be >=1"
             << " (" << life_history_sexual_maturity << ")" << std::endl;
      is_valid = false;
    }

    if (mortality_factors.empty()) {
      stream << "No mortality factors defined." << std::endl;
      // it is still valid (mainly for testing purposes)
    }

    if (mortality_factors.count(MortalityFactor::Background)) {
      if (mortality_adult_rate < 0.0 || mortality_adult_rate >= 1.0) {
        stream << "mortality.adult_rate must be between >=0.0 and <1.0 "
                  "("
               << mortality_adult_rate << ")" << std::endl;
        is_valid = false;
      }

      if (mortality_juvenile_rate < 0.0 || mortality_juvenile_rate >= 1.0) {
        stream << "mortality.juvenile_rate must be between >=0.0 and <1.0 "
                  "("
               << mortality_juvenile_rate << ")" << std::endl;
        is_valid = false;
      }
    }

    if (mortality_factors.count(MortalityFactor::Lifespan)) {
      if (establishment_age_range.first >= life_history_lifespan ||
          establishment_age_range.second >= life_history_lifespan) {
        stream << "establishment.age_range must be smaller than "
                  "`life_history.lifespan` ("
               << establishment_age_range.first << ", "
               << establishment_age_range.second << ")" << std::endl;
        is_valid = false;
      }

      if (life_history_lifespan < 1) {
        stream << "life_history.lifespan must be >=1 (" << life_history_lifespan
               << ")" << std::endl;
        is_valid = false;
      }

      if (life_history_physical_maturity_female >= life_history_lifespan) {
        stream << "life_history.physical_maturity_female must not exceed "
                  "life_history.lifespan"
               << " (" << life_history_physical_maturity_female << ")"
               << std::endl;
        is_valid = false;
      }

      if (life_history_physical_maturity_male >= life_history_lifespan) {
        stream << "life_history.physical_maturity_male must not exceed "
                  "life_history.lifespan"
               << " (" << life_history_physical_maturity_male << ")"
               << std::endl;
        is_valid = false;
      }

      if (life_history_sexual_maturity >= life_history_lifespan) {
        stream << "life_history.sexual_maturity must not exceed "
                  "life_history.lifespan"
               << " (" << life_history_sexual_maturity << ")" << std::endl;
        is_valid = false;
      }
    }

    if (reproduction_model == ReproductionModel::IlliusOConnor2000 ||
        reproduction_model == ReproductionModel::ConstantMaximum ||
        reproduction_model == ReproductionModel::Linear) {
      if (reproduction_annual_maximum <= 0.0) {
        stream << "reproduction.annual_maximum must be >0.0 ("
               << reproduction_annual_maximum << ")" << std::endl;
        is_valid = false;
      }

      if (breeding_season_length < 0 || breeding_season_length > 365) {
        stream << "breeding_season.length must be in [0,365]"
               << " (" << breeding_season_length << ")" << std::endl;
        is_valid = false;
      }

      if (breeding_season_start < 0 || breeding_season_start >= 365) {
        stream << "breeding_season.start must be in [0,364]"
               << " (" << breeding_season_start << ")" << std::endl;
        is_valid = false;
      }
    }
    // add more checks in alphabetical order
  }

  if (params.herbivore_type == HerbivoreType::Individual) {
    if (mortality_factors.count(MortalityFactor::StarvationIlliusOConnor2000)) {
      stream << "Mortality factor `StarvationIlliusOConnor2000` "
                "is not meant for individual mode."
             << std::endl;
      is_valid = false;
    }
  }

  // convert stream to string
  msg = stream.str();

  return is_valid;
}

bool Hft::is_valid(const Parameters& params) const {
  // Just call the other overload function, but dump the messages.
  std::string dump;
  return is_valid(params, dump);
}
