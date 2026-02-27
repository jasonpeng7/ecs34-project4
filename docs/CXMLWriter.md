# CXML Writer

## Overview
`CXMLWriter` is a class for writing XML data to the specified data sink. It takes `SXMLEntity` and serializes them into XML with  escaping for text and attributes.

## CXMLWriter Class
```cpp
CXMLWriter(std::shared_ptr< CDataSink > sink);
~CXMLWriter();

bool Flush();
bool WriteEntity(const SXMLEntity &entity);
```

### `CXMLWriter(std::shared_ptr< CDataSink > sink);`
- Constructor which creates an XML writer and attaches it to a data sink.

### `~CXMLWriter();`
- Destructor for `CXMLWriter`.

### `bool Flush();`
- writes closing for any elements that were started but not ended.

### `bool WriteEntity(const SXMLEntity &entity);`
- writes  a single entity (start element, end element, complete element, or character data).
-   true on success, false on failure.

## Example Usage
```cpp
std::sared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
CXMLWriter Writer(DataSink);

SXMLEntity Start;
Start.DType = SXMLEntity::EType::StartElement;
Start.DNameData = "root";
Start.SetAttribute("id", "1");

SXMLEntity Text;
Text.DType = SXMLEntity::EType::CharData;
Text.DNameData = "hello";

SXMLEntity End;
End.DType = SXMLEntity::EType::EndElement;
End.DNameData = "root";

Writer.WriteEntity(Start);
Writer.WriteEntity(Text);
Writer.WriteEntity(End);

std::cout << DataSink->String();
// output should look like <root id="1">hello</root>
```
