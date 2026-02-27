#include "CSVBusSystem.h"
#include <unordered_map>
#include <iostream>
#include <vector>

struct CCSVBusSystem::SImplementation{
    // once we read an invalid input, file is "corrupt" and we should no longer parse (STOP HERE) since we dont want to assume or guess future data
    // once a single invalid line is read, false for following lines.
    bool isInvalidRouteFile = false;
    bool isInvalidStopFile = false;

    // define SStop
    struct SStop: public CBusSystem::SStop{
        TStopID DID;
        CStreetMap::TNodeID DNodeID;

        // constructor
        SStop(TStopID stop, CStreetMap::TNodeID node) {
            DID = stop;
            DNodeID = node;
        }

        ~SStop(){}

        // return id of stop
        TStopID ID() const noexcept{
            return DID;
        }

        // return node id of bus stop
        CStreetMap::TNodeID NodeID() const noexcept{
            return DNodeID;
        }
    };

    // define SRoute
    struct SRoute: public CBusSystem::SRoute{
        std::string DName;
        int DStopCount;
        std::vector<TStopID> DStopsForRoute;
        // constructor (remember a route can have multiple stopIds)
        // but when creating a route, all we need is the name
        SRoute(std::string name, int stop_count) {
            DName = name;
            DStopCount = stop_count;
        }

        ~SRoute(){}

        // return name of route
        std::string Name() const noexcept{
            return DName;
        }

        std::size_t StopCount() const noexcept{
            return DStopCount;
        }

        // return invalid object if no index found, otherwise return stop at index
        TStopID GetStopID(std::size_t index) const noexcept{
            if(index >= DStopCount) {
                return InvalidStopID;
            }
            return DStopsForRoute[index];
        }
    };

    // dont put strings because we can have a typo in a string and it will still compile
    const std::string STOP_ID_HEADER    = "stop_id";
    const std::string NODE_ID_HEADER    = "node_id";

    std::vector< std::shared_ptr< SStop > > DStopsByIndex;
    std::unordered_map< TStopID, std::shared_ptr< SStop > > DStopsByID;

    /*
    --------------------------------------------------------------------------------------------------------
    READ STOPS SECTION
    --------------------------------------------------------------------------------------------------------
    */

    // reading stops
    bool ReadStops(std::shared_ptr< CDSVReader > stopsrc){
        // if(isInvalidStopFile) {
        //     std::cout << "invalid case enncountered, file corrupted, no parsing" << std::endl;
        //     return false;
        // }
        std::vector<std::string> TempRow;

        if(stopsrc->ReadRow(TempRow)){
            size_t StopColumn = -1;
            size_t NodeColumn = -1;
            for(size_t Index = 0; Index < TempRow.size(); Index++){
                if(TempRow[Index] == STOP_ID_HEADER){
                    StopColumn = Index;
                }
                else if(TempRow[Index] == NODE_ID_HEADER){
                    NodeColumn = Index;
                }
            }
            if(StopColumn == -1 || NodeColumn == -1){
                return false;
            }
            // DStopsById and DStopsbyIndex start at index 0 since we DONT read the header row
            while(stopsrc->ReadRow(TempRow)){
                std::cout <<"STOP = " << TempRow[StopColumn] << " NODE = " << TempRow[NodeColumn] << std::endl;
                try {
                    TStopID StopID = std::stoull(TempRow[StopColumn]);

                    for(auto i = 0; i < DStopsByIndex.size(); i++) {
                        if(StopID == DStopsByIndex[i]->DID) {
                            // duplicate stop
                            std::cout << "stop "<< StopID << "already in system dupe stop CORRUPT FILE" << std::endl;
                            isInvalidStopFile = true;
                            return false;
                        }
                    }

                    CStreetMap::TNodeID NodeID = std::stoull(TempRow[NodeColumn]);
                    auto NewStop = std::make_shared< SStop >(StopID,NodeID);
                    DStopsByIndex.push_back(NewStop);
                    DStopsByID[StopID] = NewStop;
                } catch (std::invalid_argument &) { // if we are missing columns, and we try to create a new stop, stoull() will throw error, so catch it and return false
                // THIS ALSO catches if the stop/node is not a valid argument such as not a number
                    isInvalidStopFile = true;
                    return false;
                }

             }
            return true;
            // for(auto &val : DStopsByIndex) {
            //     std::cout << val << std::endl;
            // }

            // for(auto &pair : DStopsByID) {
            //     std::cout << "<" << pair.first << "," << pair.second << ">" << std::endl;
            // }
        }
        return false;
    }


    /*
    --------------------------------------------------------------------------------------------------------
    READ ROUTES SECTION
    --------------------------------------------------------------------------------------------------------
    */

    // dont put strings because we can have a typo in a string and it will still compile
    const std::string ROUTE_HEADER    = "route";
    const std::string ROUTE_STOP_HEADER    = "stop_id";
    
    std::vector< std::shared_ptr< SRoute > >DRoutesByIndex;
    std::unordered_map<std::string, std::shared_ptr<SRoute> >DRoutesByName;

    // reading routes
    bool ReadRoutes(std::shared_ptr< CDSVReader > routesrc) {
        // if(isInvalidRouteFile) {
        //     std::cout << "invalid case enncountered, file corrupted, no parsing" << std::endl;
        //     return false;
        // }
        // read our header
        std::vector<std::string> TempRow;
        if(routesrc->ReadRow(TempRow)) {
            size_t RouteColumn = -1;
            size_t StopColumn = -1;
            for(size_t Index = 0; Index < TempRow.size(); Index++){
                if(TempRow[Index] == ROUTE_HEADER){
                    RouteColumn = Index;
                }
                else if(TempRow[Index] == ROUTE_STOP_HEADER){
                    StopColumn = Index;
                }
            }
            if(StopColumn == -1 || RouteColumn == -1){
                return false;
            }

            // for each row after the header that we read, we want to create a new route name ONLY if its not already there
            while(routesrc->ReadRow(TempRow)) {
                try {
                    std::string RouteName = TempRow[RouteColumn];
                    if(RouteName == "") { // check if route name is empty
                        isInvalidRouteFile = true;
                        return false;
                    }

                    TStopID StopId = std::stoull(TempRow[StopColumn]);

                    // try to see if this StopId exists in our stop system, if it doesnt, return false immediately
                    for (auto &pair : DStopsByID) {
                        std::cout << "STOP PAIR: " << "[" << pair.first <<", "<< pair.second << "]" << std::endl;
                    }
                    if(DStopsByID.find(StopId) == DStopsByID.end()) {
                        std::cout << StopId << " is not in STOPS " << std::endl;
                        return false;
                    } else {
                        std::cout << StopId << " in STOPS " << std::endl;
                    }

                    // if route is not already in our map, we want to create one and add to our MAP
                    if(DRoutesByName.find(RouteName) == DRoutesByName.end()) {
                        // new route should have name, then we want to add a stop 
                        auto newRoute = std::make_shared<SRoute>(RouteName, 0);
                        DRoutesByIndex.push_back(newRoute);
                        DRoutesByName[RouteName] = newRoute;

                        newRoute->DStopsForRoute.push_back(StopId);
                    }  else { // otherwise route already exists, just add a stop
                        // find the route in our map, access that specific routes' stops, and append a stop to it.

                        // however we need to check if stopId already exists, we dont want duplicates
                        // dont make copy, reference the vector, otherwise we wont be adding the stopId to the original vector
                        auto &currRouteStops = DRoutesByName[RouteName]->DStopsForRoute;
                        for(auto i = 0; i < currRouteStops.size(); i++) {
                            if (currRouteStops[i] == StopId) {
                                isInvalidRouteFile = true;
                                return false;
                            }
                        }
                        currRouteStops.push_back(StopId);
                    }
                    // increment number of stops by 1 
                    DRoutesByName[RouteName]->DStopCount += 1;
                } catch (std::invalid_argument &) { // catch stoull() exception if our columns are missing
                    isInvalidRouteFile = true;
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    SImplementation(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc){
        ReadStops(stopsrc);
        ReadRoutes(routesrc);
    }

    ~SImplementation(){};

    std::size_t StopCount() const noexcept{
        // return number of stops stored in our vector
        return DStopsByIndex.size();
    }

    std::size_t RouteCount() const noexcept{
        return DRoutesByIndex.size();
    }

    std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept{
        // return the stop at specified index
        if (index >= DStopsByIndex.size()) {
            return nullptr; // return nullptr if index is invalid
        }

        for(size_t i = 0; i < DStopsByIndex.size() ; i++) {
            if(i == index) {
                return DStopsByIndex[index]; // should return a stop object
            }
        }
    }

    std::shared_ptr<SStop> StopByID(TStopID id) const noexcept{
        // stored as (id, stop) object, so we just grab stop by its key
        // map.end() is the end iterator, so theoretically if we try to access something invalid, we'll
        for(auto i = DStopsByID.begin(); i != DStopsByID.end(); ++i) {
            if(i->first == id) {
                auto stop_res = i->second;
                return stop_res;
            }
        }
        return nullptr;
    }

    std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept{
        if(index >= RouteCount()) {
            return nullptr;
        }

        for(size_t i = 0; i < DRoutesByIndex.size(); i++) {
            if(i == index) {
                return DRoutesByIndex[index];
            }
        }
    }

    std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept{
        if(DRoutesByName.find(name) == DRoutesByName.end()) {
            // we did not find the name, return nullptr
            return nullptr;
        }
        // since our DRoutesByName is defined as <name, route>
        return DRoutesByName.find(name)->second;
    }


};    
CCSVBusSystem::CCSVBusSystem(std::shared_ptr< CDSVReader > stopsrc, std::shared_ptr< CDSVReader > routesrc){
    DImplementation = std::make_unique<SImplementation>(stopsrc,routesrc);
}

CCSVBusSystem::~CCSVBusSystem(){}

std::size_t CCSVBusSystem::StopCount() const noexcept{
    return DImplementation->StopCount();
}

std::size_t CCSVBusSystem::RouteCount() const noexcept{
    return DImplementation->RouteCount();
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByIndex(std::size_t index) const noexcept{
    return DImplementation->StopByIndex(index);
}

std::shared_ptr<CBusSystem::SStop> CCSVBusSystem::StopByID(TStopID id) const noexcept{
    return DImplementation->StopByID(id);
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByIndex(std::size_t index) const noexcept{
    return DImplementation->RouteByIndex(index);
}

std::shared_ptr<CBusSystem::SRoute> CCSVBusSystem::RouteByName(const std::string &name) const noexcept{
    return DImplementation->RouteByName(name);
}



