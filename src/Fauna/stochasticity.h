#ifndef FAUNA_STOCHASTICITY_H
#define FAUNA_STOCHASTICITY_H

#include <cstdlib>
#include <ctime>

namespace Fauna {

/// Get a random number in the interval [0,1].
/**
 * For stochastic events.
 *
 * This implementation simply uses the standard library
 * function `rand()` with the current time as random seed.
 * \param seed With the same seed the calls to this function create the same
 * results, i.e. this function is deterministic for a constant seed.
 */
inline double get_random_fraction(const unsigned int seed) {
  srand(seed);
  return (double)rand() / RAND_MAX;
}

}  // namespace Fauna
#endif  // FAUNA_STOCHASTICITY_H
