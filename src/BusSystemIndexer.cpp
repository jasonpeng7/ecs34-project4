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
    std::unordered_map<TNodeID,std::shared_ptr<SRoute>> DRoutesByNodeID;
    
    SImplementation(std::shared_ptr<CBusSystem> bussystem){
        DBusSystem = bussystem;
        for(size_t Index = 0; Index < DBusSystem->StopCount(); Index++){
            auto Stop = DBusSystem->StopByIndex(Index);
            DSortedStopsByIndex.push_back(Stop);
            // store as [nodeId, stop]
            DStopsByNodeID[Stop->NodeID()] = Stop;
        }
        std::sort(DSortedStopsByIndex.begin(), DSortedStopsByIndex.end(), [](std::shared_ptr<SStop> l, std::shared_ptr<SStop> r) -> bool{
            return l->ID() < r->ID();
        });

        for(size_t Index = 0; Index < DBusSystem->RouteCount(); Index++) {
            auto Route = DBusSystem->RouteByIndex(Index);
            DSortedRoutesByIndex.push_back(Route);
        }

        std::sort(DSortedRoutesByIndex.begin(), DSortedRoutesByIndex.end(), [](std::shared_ptr<SRoute> l, std::shared_ptr<SRoute> r) -> bool{
            return l->Name() < r->Name();
        });
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
        return DSortedStopsByIndex[index]; // do a check 
    }

    std::shared_ptr<SRoute> SortedRouteByIndex(std::size_t index) const noexcept{
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
        return false;
    }

    bool RouteBetweenNodeIDs(TNodeID src, TNodeID dest) const noexcept{
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
