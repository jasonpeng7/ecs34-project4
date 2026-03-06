#include "DijkstraPathRouter.h"

struct CDijkstraPathRouter::SImplementation {
    // set up adjacency list
    struct SVertex;
    using TEdge = std::pair<double, std::shared_ptr<SVertex>>;
    struct SVertex {
        std::vector<TEdge> DEdges;
        std::any DTag;
    };

    std::vector<std::shared_ptr<SVertex>> DVertices;

    SImplementation() {

    }
    ~SImplementation() {}

    std::size_t VertexCount() const noexcept {
        return DVertices.size();
    }

    TVertexID AddVertex(std::any tag) noexcept {
        auto newVertex = std::make_shared<SVertex>();
        newVertex->DTag = tag;
        TVertexID NewID = DVertices.size();
        DVertices.push_back(newVertex);
        return NewID;
    }

    std::any GetVertexTag(TVertexID id) const noexcept {
        // vertex id are just indices
        if(id < DVertices.size()) {
            return DVertices[id]->DTag;
        }
        return std::any(); //creates empty standard any
    }

    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept {
        if(src < DVertices.size() && dest < DVertices.size()) {
            DVertices[src]->DEdges.push_back(std::make_pair(weight, DVertices[dest]));
            if(bidir) {
                DVertices[src]->DEdges.push_back(std::make_pair(weight, DVertices[src]));
            }
            return true;
        } 
        return false;
    }

    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept {
        // extra credit
        return true;
    }
    double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept {
        std::vector<double> Weights;
        Weights.resize(DVertices.size(), std::numeric_limits<double>::max());

        // we want vertex ID as previous since edge is shared pointer
        std::vector<TVertexID> Previous;
        Previous.resize(DVertices.size(), std::numeric_limits<TVertexID>::max());

        // make priority queue and put weight + shared_ptr and just keep going until we have empty PQ

        // construct path is to do reverse probably use stack



    }
}
       