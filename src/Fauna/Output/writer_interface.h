// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Abstract definition for output writing classes.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_WRITER_INTERFACE_H
#define FAUNA_OUTPUT_WRITER_INTERFACE_H

namespace Fauna {
namespace Output {
// Forward declarations:
class Datapoint;

/// Interface class for all classes that implement writing output.
struct WriterInterface {
  /// Virtual Destructor
  /** Destructor must be virtual in an interface. */
  virtual ~WriterInterface() {}

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
#endif  // FAUNA_OUTPUT_WRITER_INTERFACE_H
