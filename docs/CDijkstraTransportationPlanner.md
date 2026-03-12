# Dijkstra Transportation Planner

## Overview
`CDijkstraTransportationPlanner` is my implementation of `CTransportationPlanner`. Here I build graph data from a `CStreetMap` and a `CBusSystem`. Then I uses our previously-written Dijkstra routing to figure out shortest/fastest-paths.

To answer both types of queries, I store both:
- shortest-path graph weighted by street distance
- fastest-path graph weighted by travel time for walking, biking, and bus travel

Along with metadata to turn a computed path into human-readable directions.

## CDijkstraTransportationPlanner class

```cpp
CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config);
~CDijkstraTransportationPlanner();

std::size_t NodeCount() const noexcept override;
std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept override;

double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path) override;
double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path) override;
bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const override;
```

### `CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config);`

- constructs our planner from a config
- loads street-map nodes into sorted orders by shortest/fastest
- builds shortest and fastest routing graphs from road segments
- adds bus edges when a bus system is available from our bus map

### `~CDijkstraTransportationPlanner();`

- our destructor

### `std::size_t NodeCount() const noexcept override;`

- returns the number of street-map nodes stored in sorted ID order

### `std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept override;`

- returns the node at the given index (after sorting)
- but returns `nullptr` if the index is out of range

### `double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TnodeeID> &path) override`

- computes shortest path between two node IDs using road distance
- fills `path` with node IDs in travel order
- returns the total distance in miles
- but this willl return `CPathRouter::NoPathExists` if we can't find the path

### `double FindFastestPath(TNodeID src, TnodeID dest, std::vector<TTripStep> &path) override;`

- computes fastest path between two node IDs by all modes (walking, biking, bus)
- returns total travel time in no # of hours
- returns `CPathRouter::NoPathExists` if no path is found or node invalid

### `bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const override;`

We didn't do this

## Configuration Notes

The planner an object that implmenets `CTransportationPlanner::SConfiguration`, and our config is `STransportationPlannerConfig`, which has: 
- a `CStreetMap`
- a bus system
- walking speed
- biking speed
- default road speed
- bus stop delay time

## Example Usage

```cpp
auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
CDijkstraTransportationPlanner Planner(Config);

std::vector<CTransportationPlanner::TNodeID> ShortestPath;
double Distance = Planner.FindShortestPath(1, 25, ShortestPath);

std::vector<CTransportationPlanner::TTripStep> FastestPath;
double Time = Planner.FindFastestPath(1, 25, FastestPath);

std::vector<std::string> Description;
if (Planner.GetPathDescription(FastestPath, Description)) {
    for (const auto &Line : Description) {
        std::cout << Line << std::endl;
    }
}
```
