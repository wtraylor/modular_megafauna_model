#ifndef DUMMY_HABITAT_H
#define DUMMY_HABITAT_H

#include "habitat.h"

namespace Fauna {
/// A dummy habitat that does nothing
class DummyHabitat : public Habitat {
 public:
  virtual void add_excreted_nitrogen(const double) {}  // deactivated
  virtual HabitatForage get_available_forage() const { return HabitatForage(); }
  virtual HabitatEnvironment get_environment() const {
    return HabitatEnvironment();
  }
  int get_day_public() const { return get_day(); }
};

}

#endif // DUMMY_HABITAT_H
