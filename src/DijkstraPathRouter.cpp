#include "DijkstraPathRouter.h"
#include <queue>
#include <stack>
#include <iostream>

struct CDijkstraPathRouter::SImplementation {
    // set up adjacency list
    struct SVertex;
    using TEdge = std::pair<double, std::shared_ptr<SVertex>>;
    struct SVertex {
        std::vector<TEdge> DEdges;
        std::any DTag;
        TVertexID DID;
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
        newVertex->DID = NewID;
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
                DVertices[dest]->DEdges.push_back(std::make_pair(weight, DVertices[src]));
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
        /* intution: 
            start at a src node, distance to everything else is infinity
            get node with smallest known weight, explore it
            if going through this node improves a neighboring distance, update and push neighbonr into PQ
        */
       
       std::priority_queue<std::pair<double, TVertexID>, std::vector<std::pair<double, TVertexID>>, std::greater<std::pair<double, TVertexID>>> pqueue;
       Weights[src] = 0;
       pqueue.push({0.0, src});

       while (pqueue.size() > 0) {
            // grab the next smallest, load info, and pop
            auto Pair = pqueue.top();
            double current_distance = Pair.first;
            TVertexID current_id = Pair.second;
            pqueue.pop();

            // since we may push the same node multiple times with different distances, nitta mentioned we can save time
            // by only popping if less than best known weight so far
            if(current_distance > Weights[current_id]) {
                continue;
            }

            // now we explore all the nodes neighbors
            for(auto edge: DVertices[current_id]->DEdges) {
                // we want to grab the weights and know what the next node is
                // each edge stored as <weight, vertex>
                double next_weight = edge.first;
                auto next_neighbor_id = edge.second->DID;

                auto new_distance = Weights[current_id] + next_weight;
                // if new distance found is less, update accordingly and push this new pair into queue
                if(new_distance < Weights[next_neighbor_id]) {
                    Weights[next_neighbor_id] = new_distance;
                    Previous[next_neighbor_id] = current_id;
                    pqueue.push({new_distance, next_neighbor_id});
                }
            }
       }
        // if the destination's weight was never updated/set --> we never reached it so no path exists
        if(Weights[dest] == std::numeric_limits<double>::max()) {
            return NoPathExists;
        }

        // now we want to reconstruct our path, like nitta explained in class, we can use a stack to reverse
        std::stack<TVertexID> stack;
        TVertexID curr = dest; // since we are essentially walking back to the start
        while(curr != std::numeric_limits<TVertexID>::max()) {
            std::cout << "previous " << Previous[curr] << std::endl;
            stack.push(curr);
            if(curr == src) {
                std::cout << "found path from des to src" << std::endl;
                break;
            }
            // move back
            curr = Previous[curr];
        }

        // now from the stack, just pop off for correct order
        while(stack.size() > 0) {
            TVertexID top = stack.top();
            stack.pop();
            path.push_back(top);
        }
        return Weights[dest];
    }
};

CDijkstraPathRouter::CDijkstraPathRouter() {
    DImplementation = std::make_unique<SImplementation>();
}

CDijkstraPathRouter::~CDijkstraPathRouter() {}

std::size_t CDijkstraPathRouter::VertexCount() const noexcept {
    return DImplementation->VertexCount();
}

CPathRouter::TVertexID CDijkstraPathRouter::AddVertex(std::any tag) noexcept {
    return DImplementation->AddVertex(tag);
}

std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept {
    return DImplementation->GetVertexTag(id);
}

bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept {
    return DImplementation->AddEdge(src, dest, weight, bidir=false);
}

bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept {
    return DImplementation->Precompute(deadline);
}

double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept {
    return DImplementation->FindShortestPath(src, dest, path);
}

       