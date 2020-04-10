/**
 * \file
 * \brief Helper class to determine how much an herbivore wants to eat.
 * \copyright ...
 * \date 2019
 */
#include "get_forage_demands.h"
#include "foraging_limits.h"
#include "herbivore_base.h"
#include "hft.h"
using namespace Fauna;

double Fauna::calc_allometry(const GivenPointAllometry& allometry,
                             const double bodymass_male_adult,
                             const double bodymass) {
  if (bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::calc_allometry() `bodymass` is negative or zero.");
  if (bodymass_male_adult <= 0.0)
    throw std::invalid_argument(
        "Fauna::calc_allometry() `bodymass_male_adult` is negative or zero.");
  if (allometry.exponent <= 0.0)
    throw std::invalid_argument(
        "Fauna::calc_allometry() `allometry.exponent` is negative or zero.");
  const double c = allometry.value_male_adult *
                   pow(bodymass_male_adult, -allometry.exponent);
  return c * pow(bodymass, allometry.exponent);
}

GetForageDemands::GetForageDemands(std::shared_ptr<const Hft> hft,
                                   const Sex sex)
    : hft(hft),
      sex(sex),
      today(-1)  // indicate that day has not been initialized.
{
  if (hft == NULL)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::GetForageDemands() "
        "Parameter `hft` is NULL.");
}

double GetForageDemands::get_bodymass_adult() const {
  if (sex == Sex::Male)
    return get_hft().body_mass_male;
  else
    return get_hft().body_mass_female;
}

void GetForageDemands::add_eaten(ForageMass eaten_forage) {
  // Check if we are eating more than possible, but leave some room for
  // floating point imprecision.
  if (!(eaten_forage <= max_intake * 1.001))
    throw std::logic_error(
        "Fauna::GetForageDemands::add_eaten() "
        "Eaten forage is greater than maximum intake.");

  // Since we just left some room for error, we are now responsible to make
  // sure that the `max_intake` *really* has no negative values.
  eaten_forage = eaten_forage.min(max_intake);

  // Now we can be sure that `eaten_forage` will not make `max_intake`
  // go below zero.
  max_intake -= eaten_forage;
}

ForageFraction GetForageDemands::get_diet_composition() const {
  // Initialize result with zero and let the algorithms set their
  // particular forage types.
  ForageFraction result(0.0);

  switch (get_hft().foraging_diet_composer) {
    case (DietComposer::PureGrazer): {
      result.set(ForageType::Grass, 1.0);  // put all into grass
      break;
    }
    // ** Add new diet composer algorithms here in case statements. **
    default:
      throw std::logic_error(
          "Fauna::GetForageDemands::get_diet_composition() "
          "The selected algorithm for diet composition "
          "(Hft::diet_composer) is not implemented.");
  }

  // Check the result, but leave some rounding error tolerance.
  if (result.sum() < 0.999 && result.sum() > 1.001)
    throw std::logic_error(
        "Fauna::GetForageDemands::get_diet_composition() "
        "The sum of the diet fractions is not 1.0. This is an "
        "implementation fault.");
  return result;
}

ForageMass GetForageDemands::get_max_digestion() const {
  assert(today > -1);  // check that init_today() has been called

  switch (get_hft().digestion_limit) {
    case (DigestiveLimit::None): {
      return ForageMass(100000);
    }
    case (DigestiveLimit::Allometric): {
      return get_max_intake_as_total_mass(
          diet_composition, energy_content,
          calc_allometry(get_hft().digestion_allometric,
                         get_hft().body_mass_male, bodymass));
    }
    case (DigestiveLimit::FixedFraction): {
      double fraction = get_hft().digestion_fixed_fraction;
      // If it is a juvenile, we need to scale maximum intake with the
      // mass-related expenditure. (See doxygen doc of `DL_FIXED_FRACTION` for
      // details.)
      const double bodymass_adult = sex == Sex::Male
                                        ? get_hft().body_mass_male
                                        : get_hft().body_mass_female;
      if (bodymass < bodymass_adult)
        fraction = fraction / pow(bodymass_adult, -0.75) * pow(bodymass, -0.75);
      return get_max_intake_as_total_mass(diet_composition, energy_content,
                                          fraction * bodymass);
    }
    case (DigestiveLimit::IlliusGordon1992): {
      // Check that we are only handling grass here. This should be have been
      // already checked in Hft::is_valid().
      assert(get_hft().foraging_diet_composer == DietComposer::PureGrazer);

      // calculate the digestive limit [MJ/ind/day]
      const ForageEnergy limit_mj = get_digestive_limit_illius_gordon_1992(
          get_bodymass_adult(), bodymass, digestibility[ForageType::Grass],
          get_hft().digestion_i_g_1992_ijk);

      // Convert energy to kg dry matter
      // kg * MJ/kg = kg; where zero values remain zero values even
      // on division by zero.
      const ForageMass limit_kg = limit_mj.divide_safely(energy_content, 0.0);

      // Set the maximum foraging limit [kgDM/ind/day]
      return limit_kg;
    }
    // ** add new digestive constraints in new else-if statements here **
    default:
      throw std::logic_error(
          "Fauna::GetForageDemands::get_max_digestion() "
          "The value for `Hft::digestive_limit` is not implemented.");
  }
}

ForageMass GetForageDemands::get_max_foraging() const {
  assert(today > -1);  // check that init_today() has been called

  // set the maximum, and then let the foraging limit algorithms
  // reduce the maximum by using ForageMass::min()
  ForageMass result(10000);  // [kgDM/ind/day]
  // (Note that using DBL_MAX here does not work because converting it to
  //  energy may result in INFINITY values.)

  // Go through all forage intake limits
  std::set<ForagingLimit>::const_iterator itr;
  for (const auto& limit : get_hft().foraging_limits) switch (limit) {
      case (ForagingLimit::IlliusOConnor2000): {
        // Check that we are only handling grass here. This should be
        // already checked in Hft::is_valid().
        assert(get_hft().foraging_diet_composer == DietComposer::PureGrazer);

        // Create functional response with digestive limit as maximum.
        // Convert half_max_intake_density from gDM/m² to kgDM/km²
        const HalfMaxIntake half_max(
            get_hft().foraging_half_max_intake_density * 1000.0,
            get_digestive_limit_illius_gordon_1992(
                get_bodymass_adult(), bodymass,
                digestibility[ForageType::Grass],
                get_hft().digestion_i_g_1992_ijk));

        // Like Pachzelt et al. (2013), we use the whole-habitat grass density,
        // not the ‘sward density’.
        const double grass_limit_mj = half_max.get_intake_rate(
            available_forage.grass.get_mass());  // [MJ/day]

        double grass_limit_kg;
        if (energy_content[ForageType::Grass] > 0.0)
          grass_limit_kg = grass_limit_mj / energy_content[ForageType::Grass];
        else
          grass_limit_kg = 0.0;  // no energy ⇒ no feeding

        // The Illius & O’Connor (2000) model applies only to grass, and
        // hence we only constrain the grass part of `result`.
        result.set(ForageType::Grass,
                   std::min(result[ForageType::Grass], grass_limit_kg));
        break;
      }
      case (ForagingLimit::GeneralFunctionalResponse):
        break;
        // Silently ignore the limit “general_functional_response” here
        // because it is applied later “on top” of all other limits.
      default:
        // ADD MORE LIMITS HERE IN NEW ELSE-IF STATEMENTS
        throw std::logic_error(
            "Fauna::GetForageDemands::get_max_foraging() "
            "One of the selected foraging limits is not implemented.");
    }
  return result;
}

ForageMass GetForageDemands::get_max_intake_as_total_mass(
    const ForageFraction& mj_proportions, const ForageEnergyContent& mj_per_kg,
    const double kg_total) {
  if (kg_total < 0.0)
    throw std::invalid_argument(
        "Fauna::get_max_intake_as_total_mass() "
        "Parameter `kg_total` is negative.");
  if (kg_total == 0.0) return ForageMass(0.0);
  if (mj_proportions.sum() < 0.999 && mj_proportions.sum() > 1.001)
    throw std::invalid_argument(
        "Fauna::get_max_intake_as_total_mass() "
        "Values in `mj_proportions` don’t sum up to 100%");

  // The energy-wise proportions of the diet are given by
  // `mj_proportions`. Now we need to obtain the mass-wise
  // composition.
  const ForageFraction mass_proportions =
      convert_mj_to_kg_proportionally(mj_per_kg, mj_proportions);

  // Make sure that the sum of mass proportions doesn’t diverge from the
  // sum of energy proportions.
  assert(mass_proportions.sum() >= 0.99 * mj_proportions.sum() &&
         mass_proportions.sum() <= 1.01 * mj_proportions.sum());

  // Multiply the maximum foraging with the mass-wise proportions.
  // To get the maximum intake for each individual forage type.
  const ForageMass mass =
      kg_total * (mass_proportions.divide_safely(mass_proportions.sum(), 0.0));

  // Make sure that the sum of mass parts matches the prescribed sum.
  assert(mass.sum() >= 0.99 * kg_total && mass.sum() <= 1.01 * kg_total);

  return mass;
}

int GetForageDemands::get_today() const {
  if (today == -1)  // initial value from constructor
    throw std::logic_error(
        "Fauna::GetForageDemands::get_today() "
        "Current day not yet initialized. Has `init_today()` "
        "been called first?");
  assert(today >= 0 && today < 365);
  return today;
}

void GetForageDemands::init_today(const int day,
                                  const HabitatForage& _available_forage,
                                  const ForageEnergyContent& _energy_content,
                                  const double _bodymass) {
  if (_bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::init_today() "
        "Parameter \"bodymass\" is zero or negative.");
  if (day < 0)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::init_today() "
        "Parameter \"day\" is negative");
  if (day >= 365)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::init_today() "
        "Parameter \"day\" is greater than 364.");

  // init today’s variables
  available_forage = _available_forage;
  bodymass = _bodymass;
  digestibility = _available_forage.get_digestibility();
  energy_content = _energy_content;
  energy_needs = 0.0;
  today = day;

  // Diet composition
  diet_composition = get_diet_composition();

  // Initialize with extreme number and then reduce it to actual maxima.
  max_intake = ForageMass(10000);

  // Reduce maximum intake by foraging limits.
  max_intake.min(get_max_foraging());

  // Reduce maximum intake by digestive limits.
  max_intake.min(get_max_digestion());

  // Apply the general functional response “on top”.
  // BUT ONLY FOR THE GRASS COMPONENT.
  if (get_hft().foraging_limits.count(
          ForagingLimit::GeneralFunctionalResponse) &&
      max_intake[ForageType::Grass] > 0.0) {
    // Create functional response with current limit as maximum.
    // Convert half_max_intake_density from gDM/m² to kgDM/km²
    const HalfMaxIntake half_max(
        get_hft().foraging_half_max_intake_density * 1000.0,
        max_intake[ForageType::Grass]);

    // Apply the result to the grass component.
    max_intake.set(ForageType::Grass,
                   half_max.get_intake_rate(
                       available_forage.grass.get_mass()));  // [kgDM/ind/day]
  }
}

bool GetForageDemands::is_day_initialized(const int day) const {
  if (day < 0)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::is_day_initialized() "
        "Parameter \"day\" is negative.");
  if (day > 364)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::is_day_initialized() "
        "Parameter \"day\" is greater than 364.");
  return day == this->today;
}

ForageMass GetForageDemands::operator()(const double _energy_needs) {
  if (today == -1)
    throw std::logic_error(
        "Fauna::GetForageDemands::operator()() "
        "This day has not yet been initialized. The function "
        "init_today() must be called before operator()().");
  if (_energy_needs < 0.0)
    throw std::invalid_argument(
        "Fauna::GetForageDemands::operator()() "
        "Parameter `energy_needs` is negative.");

  energy_needs = _energy_needs;

  // No hunger ⇒ no demands
  if (energy_needs == 0.0) return ForageMass(0.0);

  //------------------------------------------------------------------
  // CONVERT MASS TO ENERGY

  // Note that we have many variables already calculated in
  // `init_today()`

  // The maximum intake of each forage type as net energy [MJ/ind]
  const ForageEnergy max_energy_intake = max_intake * energy_content;

  // The total maximum forage intake over all forage types [kgDM/km²].
  const double max_energy_intake_sum = max_energy_intake.sum();

  // No eating capacity ⇒ no demands
  if (max_energy_intake_sum == 0.0) return ForageMass(0.0);

  //------------------------------------------------------------------
  // COMPOSE ENERGY FRACTIONS OF DIET

  // Find the forage type that is limiting the total intake the most:
  // This is the forage type where the preferred fraction
  // (↦ diet_composition) is furthest away from the fraction in the
  // possible intake (↦ max_energy_intake).

  // The fraction for a forage type of possible intake to de
  double min_fraction = 1.0;

  // Iterate through all forage types.
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    if (diet_composition[*ft] > 0)
      min_fraction = std::min(min_fraction,
                              (diet_composition[*ft] * max_energy_intake_sum) /
                                  max_energy_intake[*ft]);
  }

  // The maximum energy intake with the forage types composed in
  // the same fraction as in `diet_composition` [MJ/ind].
  const ForageEnergy max_energy_intake_comp =
      max_energy_intake * (min_fraction * diet_composition);

  // Desired forage types cannot be eaten ⇒ no demands
  if (max_energy_intake_comp.sum() == 0.0) return ForageMass(0.0);

  //------------------------------------------------------------------
  // REDUCE TO ACTUAL ENERGY NEEDS

  // The fraction to what we need to reduce the energy intake to meet
  // the actual needs.
  const double energy_reduction =
      std::min(1.0, energy_needs / max_energy_intake_comp.sum());

  // This is our finally demanded energy [MJ/ind].
  const ForageEnergy actual_energy_intake =
      max_energy_intake_comp * energy_reduction;

  // Check that we haven’t exceeded the actual needs (but leave room
  // for rounding errors).
  assert(actual_energy_intake.sum() <= energy_needs * 1.0000001);

  //------------------------------------------------------------------
  // CONVERT BACK FROM ENERGY TO MASS

  // convert MJ/ind to kgDM/ind
  ForageMass result = actual_energy_intake.divide_safely(energy_content, 0.0);

  // Make sure that we don’t exceed the total available forage.
  result.min(available_forage.get_mass());
  return result;
}
