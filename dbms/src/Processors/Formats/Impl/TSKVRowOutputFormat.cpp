#include <IO/WriteHelpers.h>
#include <IO/WriteBufferFromString.h>
#include <Processors/Formats/Impl/TSKVRowOutputFormat.h>
#include <Formats/FormatFactory.h>


namespace DB
{

TSKVRowOutputFormat::TSKVRowOutputFormat(WriteBuffer & out_, const Block & header, FormatFactory::WriteCallback callback, const FormatSettings & format_settings_)
    : TabSeparatedRowOutputFormat(out_, header, false, false, callback, format_settings_)
{
    auto & sample = getPort(PortKind::Main).getHeader();
    NamesAndTypesList columns(sample.getNamesAndTypesList());
    fields.assign(columns.begin(), columns.end());

    for (auto & field : fields)
    {
        WriteBufferFromOwnString wb;
        writeAnyEscapedString<'='>(field.name.data(), field.name.data() + field.name.size(), wb);
        writeCString("=", wb);
        field.name = wb.str();
    }
}


void TSKVRowOutputFormat::writeField(const IColumn & column, const IDataType & type, size_t row_num)
{
    writeString(fields[field_number].name, out);
    type.serializeAsTextEscaped(column, row_num, out, format_settings);
    ++field_number;
}


void TSKVRowOutputFormat::writeRowEndDelimiter()
{
    writeChar('\n', out);
    field_number = 0;
}


void registerOutputFormatProcessorTSKV(FormatFactory & factory)
{
    factory.registerOutputFormatProcessor("TSKV", [](
        WriteBuffer & buf,
        const Block & sample,
        FormatFactory::WriteCallback callback,
        const FormatSettings & settings)
    {
        return std::make_shared<TSKVRowOutputFormat>(buf, sample, callback, settings);
    });
}

}
