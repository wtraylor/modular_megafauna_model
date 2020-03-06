#include "static_reindeer.h"

using namespace Fauna;

Hft StaticReindeerPopulation::reindeer_dummy;

const Hft& StaticReindeer::get_hft() const {
  return StaticReindeerPopulation::reindeer_dummy;
}

ConstHerbivoreVector StaticReindeerPopulation::get_list() const {
  ConstHerbivoreVector result = {list[0]};
  return result;
}
