# CDijsktraPathRouter

## Overview
`CDijsktraPathRouter` is a concrete implementation of the avtrsact PathRouter class. It computes the shortest path using Dijkstras. Assumes all edges are non negative.

## CDijkstraPathRouter Class
```cpp 
    CDijkstraPathRouter();
    ~CDijkstraPathRouter();

    std::size_t VertexCount() const noexcept;
    TVertexID AddVertex(std::any tag) noexcept;
    std::any GetVertexTag(TVertexID id) const noexcept;
    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept;
    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept;
    double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept;
```

### `CDijkstraPathRouter();`

- constructor to inititalize empty graph for path routing

### `~CDijkstraPathRouter();`

- destructor for this class

### `std::size_t VertexCount() const noexcept;`

- adds vertex to internal grpah representation, stores tag and returns new id

### `std::any GetVertexTag(TVertexID id) const noexcept;`

- returns tag associated with vertex, if ID invalid, std::any is returned

### `bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept;`

- adds weighted edge between two ndoes, if bidirectional true then reverse edge created
-returns true if edge created succesfully.

### `double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept;`

- uses dijsktras to compute shortest path between two vertices

## Example Usage

```cpp
CDijkstraPathRouter Router;

// add vertices
auto A = Router.AddVertex(std::string("Start"));
auto B = Router.AddVertex(std::string("Middle"));
auto C = Router.AddVertex(std::string("End"));

// connect vertices
Router.AddEdge(A, B, 3.0, true);
Router.AddEdge(B, C, 2.0, true);

// compute shortest path
std::vector<CPathRouter::TVertexID> path;
double distance = 0.0;

if(Router.FindShortestPath(A, C, path, distance)){
    std::cout << "Distance: " << distance << std::endl;
}
```





