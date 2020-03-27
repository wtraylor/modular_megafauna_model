#include "static_reindeer.h"

using namespace Fauna;

ConstHerbivoreVector StaticReindeerPopulation::get_list() const {
  ConstHerbivoreVector result = {list[0]};
  return result;
}
