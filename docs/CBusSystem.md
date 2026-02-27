# Bus System

## Overview
`CBusSystem` is an abstract base class that represents a bus system with stops/routes. It provides us access to these stops and routes either by name or index through various base implementations that we would need to implement in a concrete class (CSVBusSystem) 

## CBusSystem Class
```cpp 
using TStopID = uint64_t;

static const TStopID InvalidStopID = std::numeric_limits<TStopID>::max();

struct SStop{
    virtual ~SStop(){};
    virtual TStopID ID() const noexcept = 0;
    virtual CStreetMap::TNodeID NodeID() const noexcept= 0;
};

struct SRoute{
    virtual ~SRoute(){};
    virtual std::string Name() const noexcept = 0;
    virtual std::size_t StopCount() const noexcept = 0;
    virtual TStopID GetStopID(std::size_t index) const noexcept = 0;
};

virtual ~CBusSystem(){};

virtual std::size_t StopCount() const noexcept = 0;
virtual std::size_t RouteCount() const noexcept = 0;
virtual std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept = 0;
virtual std::shared_ptr<SStop> StopByID(TStopID id) const noexcept = 0;
virtual std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept = 0;
virtual std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept = 0;

```

### `TStopID = uint64_t`

- unique identifier of a bus stop

### `static const TStopID InvalidStopID = std::numeric_limits<TStopID>::max();`

- invalid or non existent stop ID, we return this specifically if we try to access an out of range stop at a route

### `struct SStop{}`

- This is our stop structure, where each stop has an ID, and NodeID
- We have methods in this class to return a stopID: ID(), and nodeId: virtual CStreetMap::TNodeID NodeID() const noexcept= 0

### `struct SRoute{}}`

- This is our route structure, where each route has an name, a collection of stop(s) for this specific route
- We have methods in this class to return a route name: Name(), stopCount: StopCount(), and grabbing a specific routes' stop id: GetStopId()

### `virtual std::size_t StopCount() const noexcept = 0;`

- returns number of stops in the bus system

### `virtual std::size_t RouteCount() const noexcept = 0;`

- returns number of routes in the bus system

### `virtual std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept = 0;`

- returns the stop at a specific index 
- Stop indexes stored in a vector, so access should be 0-indexed
- returns nullptr if no index is found or index is invalid 

### `virtual std::shared_ptr<SStop> StopByID(TStopID id) const noexcept = 0;`

- returns stop at a specified stop ID
- argument passed in is NOT index/position related, we use a map to find/store the stops
- returns nullptr is no stopId is found

### `virtual std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept = 0;`

- returns route at a specified index
- route indexes are also stored in a vector, so access is 0-indexed
- returns nullptr if index is invalid or not found

### `virtual std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept = 0;`

- returns route at a specified route name
- similar to stops, route names are stored as a `<route_name, stop_id>` pair, so we access this differently
- returns nullptr if name is not found in system
 
### `virtual ~CBusSystem(){};`

- destructor for the BusSystem class

## Example Usage

```cpp
// creates BusSystem object
std::shared_ptr<CBusSystem> BusSystem = std::make_shared<CSVBusSystem>();

// grab route/stop counts from bus system
std::size_t routeCount = BusSystem->RouteCount();
std::size_t stopCount = BusSystem->StopCount();

// grab a route by index
auto route = BusSystem->RouteByIndex(0);
```









