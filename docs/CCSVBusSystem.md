# CSV Bus System

## Overview
`CCSVBusSystem` is a derived and concrete implementaion of our abstract base class CBusSystem. It loads stop and route data using our CDSV reader. This class is responsible for loading in DSV data (stops/routes), storing these stops/routes inernally, and can lookup stopes/routes by implementing the access functions provided in base BusSystem class.

## CCSVBusSystem Class
```cpp 
CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc);
~CCSVBusSystem();

std::size_t StopCount() const noexcept override;
std::size_t RouteCount() const noexcept override;
std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept override;
std::shared_ptr<SStop> StopByID(TStopID id) const noexcept override;
std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept override;
std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept override;
```

### `CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc);`

- This is the constructor that creates a CSVBusSystem by reading stop and route data from two different data sources 

### `~CCSVBusSystem();`

- This is the destructor for the CCSVBusSystem

### `std::size_t StopCount() const noexcept override;`

- returns total number of stops loaded from stopSrc

### `std::size_t RouteCount() const noexcept override;`

- returns total number of routes loaded from routeSrc

### `std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept override;`

- returns stop at specified index, nullptr if index invalid

### `std::shared_ptr<SStop> StopByID(TStopID id) const noexcept override;`

- returns stop at specified stopID, nullptr if ID not found

### `std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept override;`

- returns route at specified index, nullptr if index not found

### `std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept override;`

- returns route at specified name, nullptr if name not found

## Example Usage

### Writing
```cpp
// create two reader objects to feed into our bus system (one for stops, other for routes)
// lets assume our stop source and route source contain valid comma separated values

std::shared_ptr<CDSVReader> StopReader = std::make_shared<CDSVReader>(StopSource, ',');
std::shared_ptr<CDSVReader> RouteReader = std::make_shared<CDSVReader>(RouteSource, ',');

// create our bus system object with loaded data
CCSVBusSystem BusSystem(StopReader, RouteReader);

// access functions 
std::size_t totalStops = BusSystem->StopCount(); // returns # of stops
std::size_t totalRoutes = BusSystem->RouteCount(); // returns # of routes
```





