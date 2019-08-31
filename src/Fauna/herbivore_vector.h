#ifndef HERBIVORE_VECTOR_H
#define HERBIVORE_VECTOR_H
#include <vector>

namespace Fauna {
// Forward Declarations
class HerbivoreInterface;

/// A list of herbivore interface pointers.
typedef std::vector<HerbivoreInterface*> HerbivoreVector;

/// A list of read-only herbivore interface pointers.
typedef std::vector<const HerbivoreInterface*> ConstHerbivoreVector;

}  // namespace Fauna
#endif  // HERBIVORE_VECTOR_H
