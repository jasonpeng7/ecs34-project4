# CPathRouter

## Overview
`CPathRouter` is a abstract interface for path routing in a weigthed graph. We can add vertices, associate tags or labels with vertices, add edges, and compute shortest paths.

## CPathRouter Class
```cpp 
    static constexpr TVertexID InvalidVertexID = std::numeric_limits<TVertexID>::max();
    static constexpr double NoPathExists = std::numeric_limits<double>::max();

    virtual ~CPathRouter(){};

    virtual std::size_t VertexCount() const noexcept = 0;
    virtual TVertexID AddVertex(std::any tag) noexcept = 0;
    virtual std::any GetVertexTag(TVertexID id) const noexcept = 0;
    virtual bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept = 0;
    virtual bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept = 0;
    virtual double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept = 0;

```

### `virtual ~CPathRouter(){}`

- This is the destructor for CPathRouter 

### `virtual std::size_t VertexCount() const noexcept = 0;`

- returns total number of vertices currently stored in the graph

### `virtual TVertexID AddVertex(std::any tag) noexcept = 0;`

- adds a vertex to graph, tag stores the data associated with the new node such as label, and returns unique vertex ID 

### `virtual std::any GetVertexTag(TVertexID id) const noexcept = 0;`

- gets tag associated with vertex, returns std::any if vertex ID is invalid

### `virtual bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept = 0;`

- adds weighted edge between two vertices
- src is source, dest is destination, weight is cost, and if bidirectional is true, then reverse edge also added

### `virtual double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept = 0;`
- compute shortest path between two vertices using dijsktras

## Example Usage

```cpp
#include "DijkstraPathRouter.h"

CDijkstraPathRouter Router;

// create vertices
auto A = Router.AddVertex(std::string("A"));
auto B = Router.AddVertex(std::string("B"));
auto C = Router.AddVertex(std::string("C"));

// add edges
Router.AddEdge(A, B, 4.0, true);
Router.AddEdge(B, C, 2.0, true);
Router.AddEdge(A, C, 10.0, true);

// compute shortest path
std::vector<CPathRouter::TVertexID> path;
double distance = 0.0;

if(Router.FindShortestPath(A, C, path, distance)){
std::cout << "Shortest distance: " << distance << std::endl;
}

```





