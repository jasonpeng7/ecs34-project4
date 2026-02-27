# Street Map

## Overview
`CStreetMap` is an abstract base class that we use to represent a map made up of nodes and ways. We provide access access to map nodes and ways by either index or ID, through a common interface. We then use this class in  in a concrete implementation (`COpenStreetMap`) handles loading and storing the data.

## CStreetMap Class
```cpp
using TNodeID = uint64_t;
using TWayID = uint64_t;
using TLocation = std::pair<double, double>;

static const TNodeID InvalidNodeID = std::numeric_limits<TNodeID>::max();
static const TWayID InvalidWayID = std::numeric_limits<TWayID>::max();

struct SNode{
    virtual ~SNode(){};
    virtual TNodeID ID() const noexcept = 0;
    virtual TLocation Location() const noexcept = 0;
    virtual std::size_t AttributeCount() const noexcept = 0;
    virtual std::string GetAttributeKey(std::size_t index) const noexcept = 0;
    virtual bool HasAttribute(const std::string &key) const noexcept = 0;
    virtual std::string GetAttribute(const std::string &key) const noexcept = 0;
};

struct SWay{
    virtual ~SWay(){};
    virtual TWayID ID() const noexcept = 0;
    virtual std::size_t NodeCount() const noexcept = 0;
    virtual TNodeID GetNodeID(std::size_t index) const noexcept = 0;
    virtual std::size_t AttributeCount() const noexcept = 0;
    virtual std::string GetAttributeKey(std::size_t index) const noexcept = 0;
    virtual bool HasAttribute(const std::string &key) const noexcept = 0;
    virtual std::string GetAttribute(const std::string &key) const noexcept = 0;
};

virtual ~CStreetMap(){};

virtual std::size_t NodeCount() const noexcept = 0;
virtual std::size_t WayCount() const noexcept = 0;
virtual std::shared_ptr<SNode> NodeByIndex(std::size_t index) const noexcept = 0;
virtual std::shared_ptr<SNode> NodeByID(TNodeID id) const noexcept = 0;
virtual std::shared_ptr<SWay> WayByIndex(std::size_t index) const noexcept = 0;
virtual std::shared_ptr<SWay> WayByID(TWayID id) const noexcept = 0;
```

### `TNodeID = uint64_t`

- unique identifier type for a single map nodee

### `TWayID = uint64_t`

- unique identifier type for a map "way"

### `TLocation = std::pair<double, double>`

- stores a node location as a `(lat, long)` pair

### `static const TNodeID InvalidNodeID = std::numeric_limits<TNodeID>::max();`

- invalid or non-existent node ID
- returned by `SWay::GetNodeID()` when the requested index is out of range

### `static const TWayID InvalidWayID = std::numeric_limits<TWayID>::max();`

- invalid/non-existent way ID
- can be used as a sentinel value when a way ID is invalid

### `struct SNode{}`

- abstract node interface used by `CStreetMap` implementations
- each node stores an ID, a lat/long location, and optional attributes (parsed from OSM `<tag>` elements)

### `virtual TNodeID ID() const noexcept = 0;`

- returns the node ID for this node

### `virtual TLocation Location() const noexcept = 0;`

- returns the node location as `(lat, lon)`

### `virtual std::size_t AttributeCount() const noexcept = 0;`

- returns the # number of attributes attached to the node

### `virtual std::string GetAttributeKey(std::size_t index) const noexcept = 0;`

- returns the attribute key at `index`
- returns an empty string if `index` is >= to `AttributeCount()`

### `virtual bool HasAttribute(const std::string &key) const noexcept = 0;`

- returns `True` if the node has an attribute with the given key

### `virtual std::string GetAttribute(const std::string &key) const noexcept = 0;`

- returns the value for the specified attribute key
- returns an empty string if the key is not attached to the node

### `struct SWay{}`

- abstract way interface used by `CStreetMap` implementations
- each way stores a way ID, an ordered list of node IDs, and optional attributes

### `virtual TWayID ID() const noexcept = 0;`

- returns the way ID for this way

### `virtual std::size_t NodeCount() const noexcept = 0;`

- returns the number of node references in this specific way

### `virtual TNodeID GetNodeID(std::size_t index) const noexcept = 0;`

- returns the node ID at `index` in the way
- returns `InvalidNodeID` if `index` is >= to `NodeCount()`

### `virtual std::size_t AttributeCount() const noexcept = 0;`

- returns no # of attributes attached to the way

### `virtual std::string GetAttributeKey(std::size_t index) const noexcept = 0;`

- returns the attribute key at `index`
- but returns an empty string if `index` is >= to `AttributeCount()`

### `virtual bool HasAttribute(const std::string &key) const noexcept = 0;`

- returns true if the way has an attribute with the given key

### `virtual std::string GetAttribute(const std::string &key) const noexcept = 0;`

- returns the value for the specified attribute key
- returns an empty string if the key is not attached to the way

### `virtual std::size_t NodeCount() const noexcept = 0;`

- returns the total number of nodes stored in the street map

### `virtual std::size_t WayCount() const noexcept = 0;`

- returns the total number of ways stored in the street map

### `virtual std::shared_ptr<SNode> NodeByIndex(std::size_t index) const noexcept = 0;`

- returns the node at the specified index
- node indexes are 0-indexed
- returns `nullptr` if `index` is invalid or out of range

### `virtual std::shared_ptr<SNode> NodeByID(TNodeID id) const noexcept = 0;`

- returns the node with the specified node ID
- this lookup is by ID value, not by position
- returns `nullptr` if the ID is not found

### `virtual std::shared_ptr<SWay> WayByIndex(std::size_t index) const noexcept = 0;`

- returns the way at the specified index
- way indexes are 0-indexed
- returns `nullptr` if `index` is invalid or out of range

### `virtual std::shared_ptr<SWay> WayByID(TWayID id) const noexcept = 0;`

- returns the way with the specified way ID
- returns `nullptr` if the ID is not found

### `virtual ~CStreetMap(){};`

- destructor for the `CStreetMap` base class

## Example Usage

```cpp
// example using a concrete implementation of the abstract CStreetMap interface.
std::shared_ptr<CStreetMap> Map = SomeConcreteStreetMapFactory();

std::size_t nodeCount = Map->NodeCount();
std::size_t wayCount = Map->WayCount();

auto Node0 = Map->NodeByIndex(0);
if(Node0){
    auto Loc = Node0->Location();
    // Loc.first = latitude, Loc.second = longitude
}

auto Way0 = Map->WayByIndex(0);
if(Way0 && Way0->NodeCount() > 0){
    auto FirstNodeID = Way0->GetNodeID(0);
}
```

