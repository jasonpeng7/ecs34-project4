# Transportation Planner Command Line

## Overview
`CTransportationPlannerCommandLine` is our command-line wrapper around `CTransportationPlanner`. Here, I read commands from a `CDataSource`, write output/errors to `CDataSink` objects. Then, I use a `CDataFactory` to save the most recently computed fastest path as a CSV file.

The class supports these commands:

- `help`
- `exit`
- `count`
- `node [index]`
- `shortest [start] [end]`
- `fastest [start] [end]`
- `save`
- `print`

## CTransportationPlannerCommandLine Class
```cpp
CTransportationPlannerCommandLine(std::shared_ptr<CDataSource> cmdsrc,
                                  std::shared_ptr<CDataSink> outsink,
                                  std::shared_ptr<CDataSink> errsink,
                                  std::shared_ptr<CDataFactory> results,
                                  std::shared_ptr<CTransportationPlanner> planner);
~CTransportationPlannerCommandLine();
bool ProcessCommands();
```

### `CTransportationPlannerCommandLine(std::shared_ptr<CDataSource> cmdsrc, std::shared_ptr<CDataSink> outsink, std::shared_ptr<CDataSink> errsink, std::shared_ptr<CDataFactory> results, std::shared_ptr<CTransportationPlanner> planner);`

- this function constructs my command-line interface around the given planner/ I/O objects
- `cmdsrc` to supply user commands
- `outsink` to receive prompts prompts/command output
- `errsink` for the errors
- `results` is used by `save` to save the output CSV

### `~CTransportationPlannerCommandLine();`

- destructor

### `bool ProcessCommands();`

- runs the command loop until `exit` is read or the command source ends
- returns `true` after processing finishes
- also tracks last valid fastest path for `save` and `print` later

## Behavior Notes

### `count`

- prints no # of sorted nodes availablefrom our planner

### `node`

- expects a 0-based indexing
- prints the node ID and formatted lat/long if the index is valid

### `shortest`

- computes a shortest path by miles between two node IDs
- prints `No path found.` if no route exists

### `fastest`

- computes the fastest path in hours between two node IDs
- stores the path later for `print` and `save`

### `print`

- asks planner for a human-readable description of our last fastest path
- prints one description line per step

### `save`

- writes the last fastest path to a CSV file with columns `mode,node_id`
- uses a filename of the form `src_dest_timehr.csv`

## Example Usage

```cpp
auto Input = std::make_shared<CStringDataSource>(
    "count\n"
    "fastest 1 25\n"
    "print\n"
    "save\n"
    "exit\n");

auto Output = std::make_shared<CStringDataSink>();
auto Errors = std::make_shared<CStringDataSink>();
auto Results = std::make_shared<CFileDataFactory>("./results/");
auto Planner = std::make_shared<CDijkstraTransportationPlanner>(Config);

CTransportationPlannerCommandLine CommandLine(Input, Output, Errors, Results, Planner);
CommandLine.ProcessCommands();
```
