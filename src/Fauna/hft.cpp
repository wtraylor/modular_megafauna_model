// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Herbivore Functional Type (HFT) class and its dependents.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "hft.h"
#include <iomanip>
#include <sstream>
#include "net_energy_models.h"
#include "parameters.h"

using namespace Fauna;

double GivenPointAllometry::extrapolate(const double bodymass_male_adult,
                                        const double bodymass) const {
  if (bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::GivenPointAllometry::extrapolate() `bodymass` is negative or "
        "zero.");
  if (bodymass_male_adult <= 0.0)
    throw std::invalid_argument(
        "Fauna::GivenPointAllometry::extrapolate() `bodymass_male_adult` is "
        "negative or zero.");
  const double c = value_male_adult * pow(bodymass_male_adult, -exponent);
  return c * pow(bodymass, exponent);
}

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
  if (params.herbivore_type == HerbivoreType::Cohort) {
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

    if (body_fat_catabolism_efficiency <= 0.0 ||
        body_fat_catabolism_efficiency > 1.0) {
      stream << "body_fat.catabolism_efficiency is out of bound."
             << " (Current value: " << body_fat_catabolism_efficiency << ")"
             << std::endl;
      is_valid = false;
    }

    if (body_fat_deviation < 0.0 || body_fat_deviation > 1.0) {
      stream << "body_fat.deviation is out of bounds. (Current value: "
             << body_fat_deviation << ")" << std::endl;
      is_valid = false;
    }

    if (body_fat_gross_energy <= 0.0) {
      stream << "`body_fat.gross_energy` must be a positive number."
             << " (current value: " << body_fat_gross_energy << ")"
             << std::endl;
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

    if (body_mass_empty <= 0.0 || body_mass_empty >= 1.0) {
      stream << "body_mass.empty must be a number between 0 and 1. "
             << "Current value: " << body_mass_empty << std::endl;
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
             << ")" << std::endl;
      is_valid = false;
    }

    if (digestion_digestibility_multiplier <= 0.0 ||
        digestion_digestibility_multiplier > 1.0) {
      stream
          << "digestion.digestibility_multiplier must be in the interval (0,1]."
          << " (current value: " << digestion_digestibility_multiplier << ")"
          << std::endl;
      is_valid = false;
    }

    if (digestion_limit == DigestiveLimit::None) {
      stream << "No digestive limit defined." << std::endl;
      // the HFT is still valid (e.g. for testing purpose)
    }

    if (digestion_net_energy_model == NetEnergyModel::GrossEnergyFraction) {
      if (digestion_k_fat <= 0.0 || digestion_k_fat >= 1.0) {
        stream << "digestion.k_fat is not between 0 and 1"
               << " (current value: " << digestion_k_fat << ")" << std::endl;
        is_valid = false;
      }
      if (digestion_k_maintenance <= 0.0 || digestion_k_maintenance >= 1.0) {
        stream << "digestion.k_maintenance is not between 0 and 1"
               << " (current value: " << digestion_k_maintenance << ")"
               << std::endl;
        is_valid = false;
      }
      if (digestion_me_coefficient <= 0.0 || digestion_me_coefficient >= 1.0) {
        stream << "digestion.me_coefficient is not between 0 and 1"
               << " (current value: " << digestion_me_coefficient << ")"
               << std::endl;
        is_valid = false;
      }
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

    if (expenditure_components.count(
            ExpenditureComponent::BasalMetabolicRate) &&
        expenditure_components.count(
            ExpenditureComponent::FieldMetabolicRate)) {
      stream << "Both \"BasalMetabolicRate\" and \"FieldMetabolicRate\" are "
             << "chosen as expenditure components, but \"FieldMetabolicRate\" "
             << "already includes the basal metabolic rate." << std::endl;
      is_valid = false;
    }

    if (expenditure_components.count(
            ExpenditureComponent::BasalMetabolicRate) ||
        expenditure_components.count(
            ExpenditureComponent::FieldMetabolicRate)) {
      if (expenditure_basal_rate.value_male_adult <= 0.0) {
        stream << "Daily expenditure (MJ/day) in an adult male in "
               << "`expenditure.basal_rate.value_male_adult` must be a "
               << "positive number"
               << " (current value: " << expenditure_basal_rate.value_male_adult
               << ")" << std::endl;
        is_valid = false;
      }
    }

    if (expenditure_components.count(
            ExpenditureComponent::FieldMetabolicRate) &&
        expenditure_fmr_multiplier < 1.0) {
      stream << "The field metabolic rate (FMR) multiplier "
             << "`expenditure.fmr_multiplier` must be >= 1 because FMR is "
             << "larger than BMR."
             << " (current value: " << expenditure_fmr_multiplier << ")"
             << std::endl;
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

    if (digestion_limit == DigestiveLimit::Allometric &&
        (digestion_allometric.value_male_adult <= 0.0 ||
         digestion_allometric.value_male_adult >= 1.0)) {
      stream << "Dry-matter intake as fraction of body mass in an adult male "
             << "in `digestion.value_male_adult` must be between "
             << "in the interval (0,1)."
             << " (current value: " << digestion_allometric.value_male_adult
             << ")" << std::endl;
      is_valid = false;
    }

    if (digestion_limit == DigestiveLimit::FixedFraction &&
        (digestion_fixed_fraction <= 0.0 || digestion_fixed_fraction >= 1.0)) {
      stream << "Body mass fraction `digestion.fixed_fraction` must be in "
                "interval (0,1) if 'FixedFraction' is set as the digestive "
                "limit."
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

    if (reproduction_model == ReproductionModel::Logistic ||
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

    if (reproduction_model == ReproductionModel::Logistic) {
      // Growth rate
      if (reproduction_logistic[0] <= 0.0) {
        stream << "reproduction.logistic.growth_rate must be a positive number."
               << " (" << reproduction_logistic[0] << ")" << std::endl;
        is_valid = false;
      }
      // Midpoint
      if (reproduction_logistic[1] <= 0.0 || reproduction_logistic[1] >= 1.0) {
        stream << "reproduction.logistic.midpoint must be in interval (0,1). "
               << "(" << reproduction_logistic[1] << ")" << std::endl;
        is_valid = false;
      }
    }
    // add more checks in alphabetical order

    // SANITY CHECK OF PARAMETER COMBINATIONS

    // Minimum mortality exceeds maximum reproduction
    // A female must give birth to at least 2.0 animals that will survive until
    // sexual maturity. Otherwise the population is not able to survive.
    if ((reproduction_model == ReproductionModel::Logistic ||
         reproduction_model == ReproductionModel::ConstantMaximum ||
         reproduction_model == ReproductionModel::Linear) &&
        (mortality_factors.count(MortalityFactor::Background) &&
         mortality_factors.count(MortalityFactor::Lifespan))) {
      // Calculate the number of births a female may give, under ideal
      // conditions (max. reproductive rate) over the course of her reproductive
      // life, considering the background adult mortality that is threatening
      // her.
      double births_per_female = reproduction_annual_maximum;
      for (int i = 1;
           i < (life_history_lifespan - life_history_sexual_maturity); i++) {
        births_per_female +=
            reproduction_annual_maximum * pow(1.0 - mortality_adult_rate, i);
      }
      assert(births_per_female >= reproduction_annual_maximum);
      // Calculate the probability of a newborn to survive until (female) sexual
      // maturity.
      const double survive_to_reproduce =
          (1.0 - mortality_juvenile_rate) *
          pow(1.0 - mortality_adult_rate, life_history_sexual_maturity);
      // Combining the two yields the effective reproduction rate of a female
      // over the course of her life.
      if (births_per_female * survive_to_reproduce < 2.0) {
        stream << std::fixed;           // Print fixed number of decimal places.
        stream << std::setprecision(1)  // Print 1 decimal place.
               << "Background mortality exceeds the maximum possible "
                  "reproduction rate.\n"
               << "Considering background mortality, a female may give birth "
                  "to at most "
               << births_per_female
               << " animals over the course of her life. Their chance of "
                  "surviving until (female) sexual maturity is only "
               << survive_to_reproduce << ". The product ("
               << births_per_female * survive_to_reproduce
               << ") is less than 2, which would be the number of surviving "
                  "offspring a female must produce over the course of her "
                  "life. The population is not viable.\n"
               << "You can reduce mortality, increase reproduction rate or "
                  "increase the female reproductive lifespan."
               << std::endl;
        is_valid = false;
      }
    }

    // Minimum expenditure exceeds maximum energy intake
    double min_exp_newborn, min_exp_male, min_exp_female = 0.0;  // MJ/day/ind
    if (expenditure_components.count(
            ExpenditureComponent::BasalMetabolicRate) ||
        expenditure_components.count(
            ExpenditureComponent::FieldMetabolicRate)) {
      // First calculate only the BMR.
      min_exp_newborn =
          expenditure_basal_rate.extrapolate(body_mass_male, body_mass_birth);
      min_exp_male = expenditure_basal_rate.value_male_adult;
      min_exp_newborn =
          expenditure_basal_rate.extrapolate(body_mass_male, body_mass_female);
      // Then add the FMR multiplier.
      if (expenditure_components.count(
              ExpenditureComponent::FieldMetabolicRate)) {
        min_exp_newborn *= expenditure_fmr_multiplier;
        min_exp_male *= expenditure_fmr_multiplier;
        min_exp_female *= expenditure_fmr_multiplier;
      }
    }
    // Some reasonably high digestibility value, at least for wild forage.
    static const double DIGESTIBILITY = 0.7;
    // Set default energy content to a very high number so that the test won’t
    // fail when a new NetEnergyModel is implemented that is not considered
    // here.
    ForageEnergyContent energy_content(99999);
    if (digestion_net_energy_model == NetEnergyModel::GrossEnergyFraction) {
      energy_content = get_net_energy_from_gross_energy(
          params.forage_gross_energy, Digestibility(DIGESTIBILITY),
          digestion_me_coefficient, digestion_k_maintenance);
    }
    if (digestion_limit == DigestiveLimit::Allometric &&
        foraging_diet_composer == DietComposer::PureGrazer) {
      // Male adults
      if ((energy_content * digestion_allometric.value_male_adult *
           body_mass_male)[ForageType::Grass] < min_exp_male) {
        stream << "Based on the digestive limit and the energy expenditure, "
                  "male adults will never be able to eat enough forage to meet "
                  "their energy needs. (This assumes rich forage with a "
                  "digestibility of "
               << DIGESTIBILITY << ".)" << std::endl;
        is_valid = false;
      }
      // TODO: Females
      // TODO: Juveniles
    }
    // TODO: Other digestive limits
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
