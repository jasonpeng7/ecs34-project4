#include "DijkstraTransportationPlanner.h"
#include "StreetMap.h"
#include "DijkstraPathRouter.h"
#include "PathRouter.h"
#include <queue>
#include <stack>
#include <iostream>

struct CDijkstraTransportationPlanner::SImplementation
{
    std::shared_ptr<SConfiguration> DConfig;
    std::vector<std::shared_ptr<CStreetMap::SNode>> DSortedNodes;
    std::unordered_map<TNodeID, CPathRouter::TVertexID> DShortestVertexByNode;
    std::unordered_map<TNodeID, CPathRouter::TVertexID> DFastestVertexByNode;
    CDijkstraPathRouter DShortestRouter;
    CDijkstraPathRouter DFastestRouter;

    std::vector<CPathRouter::TVertexID, CPathRouter::TVertexID> distance;
    std::vector<CPathRouter::TVertexID, CPathRouter::TVertexID> street_name;

    SImplementation()
    {
    }

    ~SImplementation() {}
};
