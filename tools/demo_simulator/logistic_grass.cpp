#include "logistic_grass.h"
#include <sstream>

using namespace Fauna::Demo;

bool LogisticGrass::Parameters::is_valid(std::string& msg) const {
  // The message text is written into an output string stream
  std::ostringstream stream;
  bool is_valid = true;

  if (decay_monthly.empty()) {
    stream << "decay_monthly contains no values" << std::endl;
    is_valid = false;
  }

  for (std::vector<double>::const_iterator itr = decay_monthly.begin();
       itr != decay_monthly.end(); itr++)
    if (*itr < 0.0) {
      stream << "A monthly decay value is below zero." << std::endl;
      is_valid = false;
    }

  if (digestibility.empty()) {
    stream << "`digestibility` contains no values" << std::endl;
    is_valid = false;
  }

  for (std::vector<double>::const_iterator itr = decay_monthly.begin();
       itr != decay_monthly.end(); itr++) {
    if (*itr < 0.0) {
      stream << "One monthly value in digestibility < 0.0" << std::endl;
      is_valid = false;
    }
    if (*itr > 1.0) {
      stream << "One monthly value in digestibility > 1.0" << std::endl;
      is_valid = false;
    }
  }

  if (fpc < 0.0) {
    stream << "fpc < 0.0" << std::endl;
    is_valid = false;
  }

  if (fpc > 1.0) {
    stream << "fpc > 1.0" << std::endl;
    is_valid = false;
  }

  if (growth_monthly.empty()) {
    stream << "growth_monthly contains no values" << std::endl;
    is_valid = false;
  }

  for (std::vector<double>::const_iterator itr = growth_monthly.begin();
       itr != growth_monthly.end(); itr++)
    if (*itr < 0.0) {
      stream << "A monthly growth value is below zero." << std::endl;
      is_valid = false;
    }

  if (growth_monthly.size() != decay_monthly.size()) {
    stream
        << "Warning: The numbers of monthly values for growth and decay "
           "differ. Because values are recycled, growth and decay will diverge "
           "over time."
        << std::endl;
  }

  if (init_mass < 0.0) {
    stream << "init_mass < 0.0" << std::endl;
    is_valid = false;
  }

  if (reserve <= 0.0) {
    stream << "reserve <= 0.0" << std::endl;
    is_valid = false;
  }

  if (saturation < 0.0) {
    stream << "saturation < 0.0" << std::endl;
    is_valid = false;
  }

  if (init_mass > saturation) {
    stream << "init_mass > saturation" << std::endl;
    is_valid = false;
  }

  msg = stream.str();
  return is_valid;
}

LogisticGrass::LogisticGrass(const LogisticGrass::Parameters& settings)
    : settings(settings), simulation_month(-1) {
  std::string msg;
  if (!settings.is_valid(msg))
    throw std::invalid_argument(
        "FaunaSim::LogisticGrass::LogisticGrass() "
        "Parameters are not valid: " +
        msg);
  // initialize forage
  forage.set_mass(settings.init_mass);
  assert(!settings.digestibility.empty());
  forage.set_digestibility(settings.digestibility[0]);
  if (forage.get_mass() > 0.0)
    forage.set_fpc(settings.fpc);
  else
    forage.set_fpc(0.0);
}

void LogisticGrass::grow_daily(const int day_of_year) {
  if (day_of_year >= 365 || day_of_year < 0)
    throw std::invalid_argument(
        "FaunaSim::LogisticGrass::grow_daily() "
        "day_of_year out of range");

  // Increment simulation month on first day of month.
  // On the very first call, `simulation_month` is incremented from -1 to 0.
  if (Date(day_of_year,0).get_day_of_month() == 0 || simulation_month == -1)
    simulation_month++;
  assert(simulation_month >= 0);

  // Get the vector address for the current growth value.
  assert(!settings.growth_monthly.empty());
  const int growth_id = simulation_month % settings.growth_monthly.size();
  assert(growth_id >= 0 && growth_id < settings.growth_monthly.size());

  // Get the vector address for the current decay value.
  assert(!settings.decay_monthly.empty());
  const int decay_id = simulation_month % settings.decay_monthly.size();
  assert(decay_id >= 0 && decay_id < settings.decay_monthly.size());

  // available dry matter
  const double dm_avail = forage.get_mass();
  // total dry matter
  const double dm_total = dm_avail + settings.reserve;

  // Total grass maximum dry matter
  const double total_saturation = settings.saturation + settings.reserve;

  // proportional net increase of total dry matter
  const double net_growth =
      settings.growth_monthly[growth_id] * (1.0 - dm_total / total_saturation) -
      settings.decay_monthly[decay_id];

  // Absolute change in total dry matter
  const double dm_total_change = dm_total * net_growth;

  // new total dry matter
  const double dm_total_new = dm_total + dm_total_change;
  // new available dry matter
  const double dm_avail_new = std::max(dm_total_new - settings.reserve, 0.0);
  // With negative net growth, the available dry matter can drop below zero.
  // That’s why max() is used to here

  forage.set_mass(dm_avail_new);

  if (forage.get_mass() > 0.0)
    forage.set_fpc(settings.fpc);
  else
    forage.set_fpc(0.0);

  // Get the vector address for the current digestibility value.
  assert(!settings.digestibility.empty());
  const int digestibility_id = simulation_month % settings.digestibility.size();
  assert(digestibility_id >= 0 &&
         digestibility_id < settings.digestibility.size());

  forage.set_digestibility(settings.digestibility[digestibility_id]);
}

