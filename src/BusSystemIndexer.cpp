#include "BusSystemIndexer.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

struct CBusSystemIndexer::SImplementation{
    std::shared_ptr<CBusSystem> DBusSystem;

    // stops
    std::vector<std::shared_ptr<SStop>> DSortedStopsByIndex;
    std::unordered_map<TNodeID,std::shared_ptr<SStop>> DStopsByNodeID;

    // routes
    std::vector<std::shared_ptr<SRoute>> DSortedRoutesByIndex;

    struct NodeIDPairHash {
        std::size_t operator()(const std::pair<TNodeID, TNodeID> &pair) const {
            std::size_t First = pair.first;
            std::size_t Second = pair.second;
            return First ^ (Second << 1); // XOR balances 0 and 1
        }
    };
    // need to hash a pair
    std::unordered_map<std::pair<TNodeID, TNodeID>,std::unordered_set<std::shared_ptr<SRoute>>, NodeIDPairHash> DRoutesByNodeID;

    
    SImplementation(std::shared_ptr<CBusSystem> bussystem){
        DBusSystem = bussystem;
        // load in stop info
        for(size_t Index = 0; Index < DBusSystem->StopCount(); Index++){
            auto Stop = DBusSystem->StopByIndex(Index);
            DSortedStopsByIndex.push_back(Stop);
            // store as [nodeId, stop]
            DStopsByNodeID[Stop->NodeID()] = Stop;
        }
        std::sort(DSortedStopsByIndex.begin(), DSortedStopsByIndex.end(), [](std::shared_ptr<SStop> l, std::shared_ptr<SStop> r) -> bool{
            return l->ID() < r->ID();
        });
        
        // load in route info
        for(size_t Index = 0; Index < DBusSystem->RouteCount(); Index++) {
            auto Route = DBusSystem->RouteByIndex(Index);
            DSortedRoutesByIndex.push_back(Route);
        }

        std::sort(DSortedRoutesByIndex.begin(), DSortedRoutesByIndex.end(), [](std::shared_ptr<SRoute> l, std::shared_ptr<SRoute> r) -> bool{
            return l->Name() < r->Name();
        });

        // build our map for route by nodeID
        for(auto Route: DSortedRoutesByIndex) {
            // start at 1 so we can get prev and next stopID
            for(size_t Index = 1; Index < Route->StopCount(); Index ++) {
                auto Previous = Route->GetStopID(Index - 1);
                auto Current = Route->GetStopID(Index);
                auto FirstNodeID = DBusSystem->StopByID(Previous)->NodeID();
                auto SecondNodeID = DBusSystem->StopByID(Current)->NodeID();
                auto Key = std::make_pair(FirstNodeID, SecondNodeID);
                // if key dne, put unordered set, else append to set
                auto Search = DRoutesByNodeID.find(Key);
                if (Search == DRoutesByNodeID.end()) {
                    DRoutesByNodeID[Key] = {Route};
                } else {
                    Search->second.insert(Route);
                }
            }
        }
    }

    ~SImplementation(){

    }

    std::size_t StopCount() const noexcept{
        return DBusSystem->StopCount();
    }

    std::size_t RouteCount() const noexcept{
        return DBusSystem->RouteCount();
    }

    std::shared_ptr<SStop> SortedStopByIndex(std::size_t index) const noexcept{
        if(index >= StopCount()) {
            return nullptr;
        }
        return DSortedStopsByIndex[index]; 
    }

    std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const noexcept{
        if(index >= RouteCount()) {
            return nullptr;
        }
        return DSortedRoutesByIndex[index]; 
    }

    std::shared_ptr<SStop> StopByNodeID(TNodeID id) const noexcept{
        // stored as [nodeId, stop], so we can reverse lookup
        auto NodeStopPair = DStopsByNodeID.find(id);
        if(NodeStopPair == DStopsByNodeID.end()) {
            return nullptr;
        }
        return NodeStopPair->second;
    }

    bool RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<SRoute> > &routes) const noexcept{
        // take pair of nodeID as a key and map to set of routes
        auto Search =  DRoutesByNodeID.find(std::make_pair(src, dest));
        if(Search != DRoutesByNodeID.end()) {
            routes = Search->second;
            // problem: if route A's pair is <102,101> but src/dest defined as <101,102>, route A never added
            for(auto route: routes) {
                std::cout << "route " << route->Name() << " has a stop between " << src << " and " << dest << std::endl;
            }
            return true;
        }
        return false;
    }

    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
        // return true if at least one route exists between the stops at src and dest
        if(dest < src) {
            std::swap(src, dest);
        }

        auto Search = DRoutesByNodeID.find(std::make_pair(src, dest));
        if(Search != DRoutesByNodeID.end()) {
            return true;
        }
        return false;
    }

};

CBusSystemIndexer::CBusSystemIndexer(std::shared_ptr<CBusSystem> bussystem){
    DImplementation = std::make_unique<SImplementation>(bussystem);
}

CBusSystemIndexer::~CBusSystemIndexer(){

}

std::size_t CBusSystemIndexer::StopCount() const noexcept{
    return DImplementation->StopCount();
}

std::size_t CBusSystemIndexer::RouteCount() const noexcept{
    return DImplementation->RouteCount();
}

std::shared_ptr<CBusSystemIndexer::SStop> CBusSystemIndexer::SortedStopByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedStopByIndex(index);
}

std::shared_ptr<CBusSystemIndexer::SRoute> CBusSystemIndexer::SortedRouteByIndex(std::size_t index) const noexcept{
    return DImplementation->SortedRouteByIndex(index);
}

std::shared_ptr<CBusSystemIndexer::SStop> CBusSystemIndexer::StopByNodeID(TNodeID id) const noexcept{
    return DImplementation->StopByNodeID(id);
}

bool CBusSystemIndexer::RoutesByNodeIDs(TNodeID src, TNodeID dest, std::unordered_set<std::shared_ptr<SRoute> > &routes) const noexcept{
    return DImplementation->RoutesByNodeIDs(src,dest,routes);
}

bool CBusSystemIndexer::RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
    return DImplementation->RouteBetweenNodeIDs(src,dest);
}
