#ifndef OUTPUT_WRITER_INTERFACE_H
#define OUTPUT_WRITER_INTERFACE_H

namespace Fauna {
// Forward declarations:
class SimulationUnit;

namespace Output {
// Forward declarations:
class Datapoint;

/// Interface class for all classes that implement writing output.
struct OutputWriterInterface {
  /// Write spatially & temporally aggregated output data.
  virtual void write_datapoint(const Datapoint& datapoint) = 0;
};
}  // namespace Output
}  // namespace Fauna
#endif  // OUTPUT_WRITER_INTERFACE_H
