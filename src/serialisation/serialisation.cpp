#include "amanuensis/serialisation.hpp"

// Serialisation is almost entirely header-based (templates must be visible
// at the point of instantiation).  This translation unit exists so that
// the CMake target has a source file to compile, and to verify that the
// header parses cleanly on its own.

namespace Amanuensis {

// Intentionally empty — all logic lives in serialisation.hpp.

} // namespace Amanuensis
