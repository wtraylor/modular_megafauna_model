#include "text_table_writer.h"

using namespace Fauna;
using namespace Fauna::Output;

TextTableWriter::TextTableWriter(
    const OutputInterval interval,
    const Parameters::TextTableWriterOptions& options)
    : interval(interval), options(options) {}

void TextTableWriter::write_datapoint(const DataPoint& datapoint) {}
