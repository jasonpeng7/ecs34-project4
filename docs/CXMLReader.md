# CXML Reader

## Overview
`CXMLReader` is a class to read XML data from a specified source. It parses XML into a stream of `SXMLEntity` objects (start element, end element, and character data) so tests and callers can consume one entity at a time.

## CXMLReader Class
```cpp
CXMLReader(std::shared_ptr< CDataSource > src);
~CXMLReader();

bool End() const;
bool ReadEntity(SXMLEntity &entity, bool skipcdata = false);
```

### `CXMLReader(std::shared_ptr< CDataSource > src);`
- constructor that creates an XML readr and attaches it to a data source

### `~CXMLReader();`
- Destructor for `CXMLReader`.

### `bool End() const;`
- Returns true when the reader has no more entities to read.

### `bool ReadEntity(SXMLEntity &entity, bool skipcdata = false);`
- reads the next available entity from the XML stream.
- when `skipcdata` is set to true value, we skip character data entities
- Returns true on success and false when no more entities are available

## Example Usage
```cpp
std::string XMLString = "<root><child a=\"1\">hi</child></root>";
std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(XMLString);
CXMLReader Reader(DataSource);

SXMLEntity Entity;
while(Reader.ReadEntity(Entity)){
    // Entity.DType is StartElement, EndElement, or CharData
    // Entity.DNameData is tag name (or text for CharData)
}
```
