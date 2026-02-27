# Open Street Map

## Overview
`COpenStreetMap` is an implementation of the abstract `CStreetMap` class. It loads and parses Open Street Map (OSM) XML data using the `CXMLReader` that we wrote in the earlier assignment. We store nodes and ways using maps so that we can provide lookup functions by index and ID through the `CStreetMap` interface.

The nodes and ways may include attributes parsed from OSM `<tag>` elements. These attributes are accessed through the `SNode` and `SWay` interfaces that are inherited from `CStreetMap`.

## COpenStreetMap Class
```cpp
COpenStreetMap(std::shared_ptr<CXMLReader> src);
~COpenStreetMap();

std::size_t NodeCount() const noexcept override;
std::size_t WayCount() const noexcept override;
std::shared_ptr<CStreetMap::SNode> NodeByIndex(std::size_t index) const noexcept override;
std::shared_ptr<CStreetMap::SNode> NodeByID(TNodeID id) const noexcept override;
std::shared_ptr<CStreetMap::SWay> WayByIndex(std::size_t index) const noexcept override;
std::shared_ptr<CStreetMap::SWay> WayByID(TWayID id) const noexcept override;
```

### `COpenStreetMap(std::shared_ptr<CXMLReader> src);`

- constructor that creates an Open Street Map object by reading OSM XML entities from the provided `CXMLReader`
- parses nodes and ways from the XML input and stores them internally for later access

### `~COpenStreetMap();`

- destructor for the `COpenStreetMap` class

### `std::size_t NodeCount() const noexcept override;`

- returns the total no # of nodes parsed, stored in the map

### `std::size_t WayCount() const noexcept override;`

- returns the total no # of ways parsed, stored in the map

### `std::shared_ptr<CStreetMap::SNode> NodeByIndex(std::size_t index) const noexcept override;`

- returns the node at the specified index
- node indexes are stored in a vector
- returns `nullptr` if `index` is greater than or equal to `NodeCount()`

### `std::shared_ptr<CStreetMap::SNode> NodeByID(TNodeID id) const noexcept override;`

- returns node associated with the node ID
- returns `nullptr` if the node ID is not found

### `std::shared_ptr<CStreetMap::SWay> WayByIndex(std::size_t index) const noexcept override;`

- returns way at the specified index
- way indexes are stored in a vector, so it is 0-indexed
- returns `nullptr` if `index` is >= to `WayCount()`

### `std::shared_ptr<CStreetMap::SWay> WayByID(TWayID id) const noexcept override;`

- returns the way associated with the specific way ID
- returns `nullptr` if the way ID is not found

## Example Usage

### Reading OSM XML
```cpp
// small inline OSM XML example
std::string OSMData =
    "<osm version=\"0.6\">"
    "  <node id=\"1\" lat=\"38.5\" lon=\"-121.7\">"
    "    <tag k=\"name\" v=\"Example Node\"/>"
    "  </node>"
    "  <way id=\"42\">"
    "    <nd ref=\"1\"/>"
    "    <tag k=\"highway\" v=\"residential\"/>"
    "  </way>"
    "</osm>";

std::shared_ptr<CDataSource> Source = std::make_shared<CStringDataSource>(OSMData);
std::shared_ptr<CXMLReader> Reader = std::make_shared<CXMLReader>(Source);

std::shared_ptr<CStreetMap> Map = std::make_shared<COpenStreetMap>(Reader);

std::size_t TotalNodes = Map->NodeCount();
std::size_t TotalWays = Map->WayCount();

auto Way = Map->WayByID(42);
if(Way){
    std::size_t WayNodes = Way->NodeCount();
    bool IsHighway = Way->HasAttribute("highway");
    std::string HighwayType = Way->GetAttribute("highway");
}
```

