#include "text_table_writer.h"
#include "datapoint.h"

using namespace Fauna;
using namespace Fauna::Output;

const char* TextTableWriter::FILE_EXTENSION = ".tsv";

TextTableWriter::TextTableWriter(
    const OutputInterval interval,
    const Parameters::TextTableWriterOptions& options)
    : interval(interval), options(options) {}

void TextTableWriter::write_datapoint(const Datapoint& datapoint) {}
