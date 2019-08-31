#ifndef WRITER_INTERFACE_H
#define WRITER_INTERFACE_H

namespace Fauna {
// Forward declarations:
class SimulationUnit;

namespace Output {
// Forward declarations:
class Datapoint;

/// Interface class for all classes that implement writing output.
struct WriterInterface {
  /// Write spatially & temporally aggregated output data.
  /**
   * \param datapoint The data to write.
   * \throw std::invalid_argument If `datapoint.combined_data.datapoint_count`
   * is zero.
   */
  virtual void write_datapoint(const Datapoint& datapoint) = 0;
};
}  // namespace Output
}  // namespace Fauna
#endif  // WRITER_INTERFACE_H
