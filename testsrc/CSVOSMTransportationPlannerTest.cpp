#include <gtest/gtest.h>
#include "XMLReader.h"
#include "StringUtils.h"
#include "StringDataSource.h"
#include "OpenStreetMap.h"
#include "CSVBusSystem.h"
#include "TransportationPlannerConfig.h"
#include "DijkstraTransportationPlanner.h"
#include "GeographicUtils.h"
#include <unordered_set>

namespace {

class CTestStreetMap : public CStreetMap {
    private:
        struct SNodeImpl : public CStreetMap::SNode {
            TNodeID DID;
            SLocation DLocation;
            std::vector<std::pair<std::string, std::string>> DAttributes;

            SNodeImpl(TNodeID id, double lat, double lon) : DID(id), DLocation(lat, lon) {}

            TNodeID ID() const noexcept override { return DID; }
            SLocation Location() const noexcept override { return DLocation; }
            std::size_t AttributeCount() const noexcept override { return DAttributes.size(); }
            std::string GetAttributeKey(std::size_t index) const noexcept override {
                return index < DAttributes.size() ? DAttributes[index].first : std::string();
            }
            bool HasAttribute(const std::string &key) const noexcept override {
                for (const auto &Attribute : DAttributes) {
                    if (Attribute.first == key) {
                        return true;
                    }
                }
                return false;
            }
            std::string GetAttribute(const std::string &key) const noexcept override {
                for (const auto &Attribute : DAttributes) {
                    if (Attribute.first == key) {
                        return Attribute.second;
                    }
                }
                return std::string();
            }
        };

        struct SWayImpl : public CStreetMap::SWay {
            TWayID DID;
            std::vector<TNodeID> DNodeIDs;
            std::vector<std::pair<std::string, std::string>> DAttributes;

            explicit SWayImpl(TWayID id) : DID(id) {}

            TWayID ID() const noexcept override { return DID; }
            std::size_t NodeCount() const noexcept override { return DNodeIDs.size(); }
            TNodeID GetNodeID(std::size_t index) const noexcept override {
                return index < DNodeIDs.size() ? DNodeIDs[index] : InvalidNodeID;
            }
            std::size_t AttributeCount() const noexcept override { return DAttributes.size(); }
            std::string GetAttributeKey(std::size_t index) const noexcept override {
                return index < DAttributes.size() ? DAttributes[index].first : std::string();
            }
            bool HasAttribute(const std::string &key) const noexcept override {
                for (const auto &Attribute : DAttributes) {
                    if (Attribute.first == key) {
                        return true;
                    }
                }
                return false;
            }
            std::string GetAttribute(const std::string &key) const noexcept override {
                for (const auto &Attribute : DAttributes) {
                    if (Attribute.first == key) {
                        return Attribute.second;
                    }
                }
                return std::string();
            }
        };

        std::vector<std::shared_ptr<SNodeImpl>> DNodesByIndex;
        std::unordered_map<TNodeID, std::shared_ptr<SNodeImpl>> DNodesByID;
        std::vector<std::shared_ptr<SWayImpl>> DWaysByIndex;
        std::unordered_map<TWayID, std::shared_ptr<SWayImpl>> DWaysByID;
        std::unordered_set<TNodeID> DHiddenNodeIDs;
        std::unordered_map<TNodeID, std::vector<SLocation>> DNodeLookupResponses;
        mutable std::unordered_map<TNodeID, std::size_t> DNodeLookupCounts;

    public:
        ~CTestStreetMap() override = default;

        std::shared_ptr<SNodeImpl> AddNode(TNodeID id, double lat, double lon, bool addtoindex = true) {
            auto Node = std::make_shared<SNodeImpl>(id, lat, lon);
            DNodesByID[id] = Node;
            if (addtoindex) {
                DNodesByIndex.push_back(Node);
            }
            return Node;
        }

        std::shared_ptr<SWayImpl> AddWay(TWayID id, const std::vector<TNodeID> &nodeids,
                                         const std::vector<std::pair<std::string, std::string>> &attributes = {}) {
            auto Way = std::make_shared<SWayImpl>(id);
            Way->DNodeIDs = nodeids;
            Way->DAttributes = attributes;
            DWaysByIndex.push_back(Way);
            DWaysByID[id] = Way;
            return Way;
        }

        void HideNode(TNodeID id) {
            DHiddenNodeIDs.insert(id);
        }

        void SetNodeLookupResponses(TNodeID id, const std::vector<SLocation> &responses) {
            DNodeLookupResponses[id] = responses;
            DNodeLookupCounts.erase(id);
        }

        std::size_t NodeCount() const noexcept override { return DNodesByIndex.size(); }
        std::size_t WayCount() const noexcept override { return DWaysByIndex.size(); }
        std::shared_ptr<SNode> NodeByIndex(std::size_t index) const noexcept override {
            return index < DNodesByIndex.size() ? DNodesByIndex[index] : nullptr;
        }
        std::shared_ptr<SNode> NodeByID(TNodeID id) const noexcept override {
            if (DHiddenNodeIDs.find(id) != DHiddenNodeIDs.end()) {
                return nullptr;
            }
            auto Search = DNodesByID.find(id);
            if (Search == DNodesByID.end()) {
                return nullptr;
            }

            auto ResponseSearch = DNodeLookupResponses.find(id);
            if ((ResponseSearch == DNodeLookupResponses.end()) || ResponseSearch->second.empty()) {
                return Search->second;
            }

            auto &LookupCount = DNodeLookupCounts[id];
            auto ResponseIndex = std::min(LookupCount, ResponseSearch->second.size() - 1);
            LookupCount++;

            auto Response = ResponseSearch->second[ResponseIndex];
            auto Node = std::make_shared<SNodeImpl>(id, Response.DLatitude, Response.DLongitude);
            Node->DAttributes = Search->second->DAttributes;
            return Node;
        }
        std::shared_ptr<SWay> WayByIndex(std::size_t index) const noexcept override {
            return index < DWaysByIndex.size() ? DWaysByIndex[index] : nullptr;
        }
        std::shared_ptr<SWay> WayByID(TWayID id) const noexcept override {
            auto Search = DWaysByID.find(id);
            return Search == DWaysByID.end() ? nullptr : Search->second;
        }
};

class CTestBusSystem : public CBusSystem {
    private:
        struct SStopImpl : public CBusSystem::SStop {
            TStopID DID;
            CStreetMap::TNodeID DNodeID;

            SStopImpl(TStopID id, CStreetMap::TNodeID nodeid) : DID(id), DNodeID(nodeid) {}

            TStopID ID() const noexcept override { return DID; }
            CStreetMap::TNodeID NodeID() const noexcept override { return DNodeID; }
        };

        struct SRouteImpl : public CBusSystem::SRoute {
            std::string DName;
            std::vector<TStopID> DStopIDs;

            explicit SRouteImpl(std::string name) : DName(std::move(name)) {}

            std::string Name() const noexcept override { return DName; }
            std::size_t StopCount() const noexcept override { return DStopIDs.size(); }
            TStopID GetStopID(std::size_t index) const noexcept override {
                return index < DStopIDs.size() ? DStopIDs[index] : InvalidStopID;
            }
        };

        std::vector<std::shared_ptr<SStopImpl>> DStopsByIndex;
        std::unordered_map<TStopID, std::shared_ptr<SStopImpl>> DStopsByID;
        std::vector<std::shared_ptr<SRouteImpl>> DRoutesByIndex;
        std::vector<std::size_t> DRouteCountResponses;
        mutable std::size_t DRouteCountCalls = 0;
        std::unordered_map<TStopID, std::vector<bool>> DStopLookupResponses;
        mutable std::unordered_map<TStopID, std::size_t> DStopLookupCalls;

    public:
        ~CTestBusSystem() override = default;

        std::shared_ptr<SStopImpl> AddIndexedStop(TStopID id, CStreetMap::TNodeID nodeid) {
            auto Stop = std::make_shared<SStopImpl>(id, nodeid);
            DStopsByIndex.push_back(Stop);
            DStopsByID[id] = Stop;
            return Stop;
        }

        std::shared_ptr<SStopImpl> AddLookupOnlyStop(TStopID id, CStreetMap::TNodeID nodeid) {
            auto Stop = std::make_shared<SStopImpl>(id, nodeid);
            DStopsByID[id] = Stop;
            return Stop;
        }

        std::shared_ptr<SRouteImpl> AddRoute(const std::string &name, const std::vector<TStopID> &stopids) {
            auto Route = std::make_shared<SRouteImpl>(name);
            Route->DStopIDs = stopids;
            DRoutesByIndex.push_back(Route);
            return Route;
        }

        void SetRouteCountResponses(const std::vector<std::size_t> &responses) {
            DRouteCountResponses = responses;
            DRouteCountCalls = 0;
        }

        void SetStopLookupResponses(TStopID id, const std::vector<bool> &responses) {
            DStopLookupResponses[id] = responses;
            DStopLookupCalls.erase(id);
        }

        std::size_t StopCount() const noexcept override { return DStopsByIndex.size(); }
        std::size_t RouteCount() const noexcept override {
            if (DRouteCountResponses.empty()) {
                return DRoutesByIndex.size();
            }
            auto ResponseIndex = std::min(DRouteCountCalls, DRouteCountResponses.size() - 1);
            DRouteCountCalls++;
            return DRouteCountResponses[ResponseIndex];
        }
        std::shared_ptr<SStop> StopByIndex(std::size_t index) const noexcept override {
            return index < DStopsByIndex.size() ? DStopsByIndex[index] : nullptr;
        }
        std::shared_ptr<SStop> StopByID(TStopID id) const noexcept override {
            auto ResponseSearch = DStopLookupResponses.find(id);
            if ((ResponseSearch != DStopLookupResponses.end()) && !ResponseSearch->second.empty()) {
                auto &LookupCount = DStopLookupCalls[id];
                auto ResponseIndex = std::min(LookupCount, ResponseSearch->second.size() - 1);
                LookupCount++;
                if (!ResponseSearch->second[ResponseIndex]) {
                    return nullptr;
                }
            }
            auto Search = DStopsByID.find(id);
            return Search == DStopsByID.end() ? nullptr : Search->second;
        }
        std::shared_ptr<SRoute> RouteByIndex(std::size_t index) const noexcept override {
            return index < DRoutesByIndex.size() ? DRoutesByIndex[index] : nullptr;
        }
        std::shared_ptr<SRoute> RouteByName(const std::string &name) const noexcept override {
            for (const auto &Route : DRoutesByIndex) {
                if (Route->DName == name) {
                    return Route;
                }
            }
            return nullptr;
        }
};

} // namespace

TEST(CSVOSMTransporationPlanner, SimpleTest){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap,BusSystem);
    CDijkstraTransportationPlanner Planner(Config);
    std::vector< CTransportationPlanner::TNodeID > ShortestPath;
    std::vector< CTransportationPlanner::TTripStep > FastestPath;
    EXPECT_EQ(Planner.FindShortestPath(0,1,ShortestPath),CPathRouter::NoPathExists);
    EXPECT_EQ(Planner.FindFastestPath(0,1,FastestPath),CPathRouter::NoPathExists);
}

TEST(CSVOSMTransporationPlanner, SortedNodeTest){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">"
                                                            "<node id=\"4\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.6\" lon=\"-121.7\"/>"
                                                            "<node id=\"1\" lat=\"38.6\" lon=\"-121.8\"/>"
                                                            "<node id=\"3\" lat=\"38.5\" lon=\"-121.8\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<tag k=\"oneway\" v=\"yes\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"3\"/>"
                                                            "<nd ref=\"4\"/>"
                                                            "<nd ref=\"1\"/>"
                                                            "<tag k=\"oneway\" v=\"yes\"/>"
                                                            "</way>"
                                                            "</osm>");
    // 6.9090909 mil 1 -> 2
    // 5.4 mile  2 -> 3
    // 6.9090909 mil 3 -> 4
    // 5.407386 mi 4 -> 1
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap,BusSystem);
    CDijkstraTransportationPlanner Planner(Config);
    EXPECT_EQ(Planner.NodeCount(),4);
    for(std::size_t Index = 0; Index < Planner.NodeCount(); Index++){
        auto Node = Planner.SortedNodeByIndex(Index);
        ASSERT_TRUE(Node);
        EXPECT_EQ(Node->ID(),Index+1);
    }
}


TEST(CSVOSMTransporationPlanner, ShortestPathTest){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.6\" lon=\"-121.7\"/>"
                                                            "<node id=\"3\" lat=\"38.6\" lon=\"-121.8\"/>"
                                                            "<node id=\"4\" lat=\"38.5\" lon=\"-121.8\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<tag k=\"oneway\" v=\"yes\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"3\"/>"
                                                            "<nd ref=\"4\"/>"
                                                            "<nd ref=\"1\"/>"
                                                            "<tag k=\"oneway\" v=\"yes\"/>"
                                                            "</way>"
                                                            "</osm>");
    // 6.9090909 mil 1 -> 2
    // 5.4 mile  2 -> 3
    // 6.9090909 mil 3 -> 4
    // 5.407386 mi 4 -> 1
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap,BusSystem);
    CDijkstraTransportationPlanner Planner(Config);
    std::vector< CTransportationPlanner::TNodeID > ShortestPath, ExpectedShortestPath = {1,2,3,4};
    double ExpectedDistance = SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.5,-121.7),CStreetMap::SLocation(38.6,-121.7)) + 
                                SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.6,-121.7),CStreetMap::SLocation(38.6,-121.8)) + 
                                SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.6,-121.8),CStreetMap::SLocation(38.5,-121.8));
    EXPECT_EQ(Planner.FindShortestPath(1,4,ShortestPath),ExpectedDistance);
    EXPECT_EQ(ShortestPath,ExpectedShortestPath);
}

TEST(CSVOSMTransporationPlanner, FastestPathTest){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.6\" lon=\"-121.7\"/>"
                                                            "<node id=\"3\" lat=\"38.6\" lon=\"-121.8\"/>"
                                                            "<node id=\"4\" lat=\"38.5\" lon=\"-121.8\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<nd ref=\"4\"/>"
                                                            "<tag k=\"maxspeed\" v=\"20 mph\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"4\"/>"
                                                            "<nd ref=\"1\"/>"
                                                            "</way>"
                                                            "</osm>");
    // 6.9090909 mil 1 <-> 2
    // 5.4 mile  2 <-> 3
    // 6.9090909 mil 3 <-> 4
    // 5.407386 mi 4 <-> 1
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "101,1\n"
                                                            "102,2\n"
                                                            "103,3\n"
                                                            "104,4"
                                                            );
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "A,101\n"
                                                             "A,102\n"
                                                             "A,103\n"
                                                             "A,104\n"
                                                             "A,101\n"
                                                             "B,104\n"
                                                             "B,103\n"
                                                             "B,102\n"
                                                             "B,103\n"
                                                             "B,104");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap,BusSystem);
    CDijkstraTransportationPlanner Planner(Config);
    std::vector< CTransportationPlanner::TTripStep > BusFastestPath, ExpectedBusFastestPath = {{CTransportationPlanner::ETransportationMode::Walk,1},
                                                                                        {CTransportationPlanner::ETransportationMode::Bus,2},
                                                                                        {CTransportationPlanner::ETransportationMode::Bus,3}};
    double ExpectedBusDistance = SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.5,-121.7),CStreetMap::SLocation(38.6,-121.7)) + 
                                SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.6,-121.7),CStreetMap::SLocation(38.6,-121.8));
    double ExpectedBusTime = ExpectedBusDistance / 20.0 + (60.0 / 3600.0);
    EXPECT_EQ(Planner.FindFastestPath(1,3,BusFastestPath),ExpectedBusTime);
    EXPECT_EQ(BusFastestPath,ExpectedBusFastestPath);
    std::vector< CTransportationPlanner::TTripStep > BikeFastestPath, ExpectedBikeFastestPath = {{CTransportationPlanner::ETransportationMode::Bike,1},
                                                                                        {CTransportationPlanner::ETransportationMode::Bike,4}};
    double ExpectedBikeDistance = SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.5,-121.7),CStreetMap::SLocation(38.5,-121.8));
    double ExpectedBikeTime = ExpectedBikeDistance / 8.0;
    EXPECT_EQ(Planner.FindFastestPath(1,4,BikeFastestPath),ExpectedBikeTime);
    EXPECT_EQ(BikeFastestPath,ExpectedBikeFastestPath);

}

TEST(CSVOSMTransporationPlanner, PathDescription){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\" generator=\"osmconvert 0.8.5\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.55\" lon=\"-121.7\"/>"
                                                            "<node id=\"3\" lat=\"38.6\" lon=\"-121.7\"/>"
                                                            "<node id=\"4\" lat=\"38.6\" lon=\"-121.78\"/>"
                                                            "<node id=\"5\" lat=\"38.6\" lon=\"-121.8\"/>"
                                                            "<node id=\"6\" lat=\"38.55\" lon=\"-121.8\"/>"
                                                            "<node id=\"7\" lat=\"38.5\" lon=\"-121.8\"/>"
                                                            "<node id=\"8\" lat=\"38.5\" lon=\"-121.72\"/>"
                                                            "<node id=\"9\" lat=\"38.45\" lon=\"-121.72\"/>"
                                                            "<node id=\"10\" lat=\"38.4\" lon=\"-121.72\"/>"
                                                            "<node id=\"11\" lat=\"38.7\" lon=\"-121.78\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<tag k=\"name\" v=\"A St.\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"3\"/>"
                                                            "<nd ref=\"4\"/>"
                                                            "<nd ref=\"5\"/>"
                                                            "<tag k=\"name\" v=\"2nd St.\"/>"
                                                            "</way>"
                                                            "<way id=\"12\">"
                                                            "<nd ref=\"5\"/>"
                                                            "<nd ref=\"6\"/>"
                                                            "<nd ref=\"7\"/>"
                                                            "<tag k=\"name\" v=\"B St.\"/>"
                                                            "</way>"
                                                            "<way id=\"13\">"
                                                            "<nd ref=\"7\"/>"
                                                            "<nd ref=\"8\"/>"
                                                            "<nd ref=\"1\"/>"
                                                            "<tag k=\"name\" v=\"Main St.\"/>"
                                                            "</way>"
                                                            "<way id=\"14\">"
                                                            "<nd ref=\"8\"/>"
                                                            "<nd ref=\"9\"/>"
                                                            "<nd ref=\"10\"/>"
                                                            "</way>"
                                                            "<way id=\"15\">"
                                                            "<nd ref=\"4\"/>"
                                                            "<nd ref=\"11\"/>"
                                                            "</way>"
                                                            "</osm>");
    // 6.9090909 mil 1 <-> 2
    // 5.4 mile  2 <-> 3
    // 6.9090909 mil 3 <-> 4
    // 5.407386 mi 4 <-> 1
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "101,1\n"
                                                            "102,3\n"
                                                            "103,5\n"
                                                            "104,7"
                                                            );
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "A,101\n"
                                                             "A,102\n"
                                                             "A,103\n"
                                                             "A,104\n"
                                                             "A,101\n"
                                                             "B,104\n"
                                                             "B,103\n"
                                                             "B,102\n"
                                                             "B,103\n"
                                                             "B,104");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap,BusSystem);
    CDijkstraTransportationPlanner Planner(Config);
    std::vector< CTransportationPlanner::TTripStep > Path1 = {{CTransportationPlanner::ETransportationMode::Walk,8},
                                                                {CTransportationPlanner::ETransportationMode::Walk,1},
                                                                {CTransportationPlanner::ETransportationMode::Bus,3},
                                                                {CTransportationPlanner::ETransportationMode::Bus,5},
                                                                {CTransportationPlanner::ETransportationMode::Walk,4},
                                                                {CTransportationPlanner::ETransportationMode::Walk,11}};

    std::vector<std::string> Description1, ExpectedDescription1 = {"Start at 38d 30' 0\" N, 121d 43' 12\" W",
                                                                    "Walk E along Main St. for 1.1 mi",
                                                                    "Take Bus A from stop 101 to stop 103",
                                                                    "Walk E along 2nd St. for 1.1 mi",
                                                                    "Walk N toward End for 6.9 mi",
                                                                    "End at 38d 42' 0\" N, 121d 46' 48\" W"};
    EXPECT_TRUE(Planner.GetPathDescription(Path1,Description1));
    EXPECT_EQ(Description1, ExpectedDescription1);
    std::vector< CTransportationPlanner::TTripStep > Path2 = {{CTransportationPlanner::ETransportationMode::Bike,8},
                                                                {CTransportationPlanner::ETransportationMode::Bike,7},
                                                                {CTransportationPlanner::ETransportationMode::Bike,6},
                                                                {CTransportationPlanner::ETransportationMode::Bike,5},
                                                                {CTransportationPlanner::ETransportationMode::Bike,4}};

    std::vector<std::string> Description2, ExpectedDescription2 = {"Start at 38d 30' 0\" N, 121d 43' 12\" W",
                                                                    "Bike W along Main St. for 4.3 mi",
                                                                    "Bike N along B St. for 6.9 mi",
                                                                    "Bike E along 2nd St. for 1.1 mi",
                                                                    "End at 38d 36' 0\" N, 121d 46' 48\" W"};
    EXPECT_TRUE(Planner.GetPathDescription(Path2,Description2));
    EXPECT_EQ(Description2, ExpectedDescription2);

    std::vector< CTransportationPlanner::TTripStep > Path3 = {{CTransportationPlanner::ETransportationMode::Bike,10},
                                                                {CTransportationPlanner::ETransportationMode::Bike,9},
                                                                {CTransportationPlanner::ETransportationMode::Bike,8},
                                                                {CTransportationPlanner::ETransportationMode::Bike,7},
                                                                {CTransportationPlanner::ETransportationMode::Bike,6}};

    std::vector<std::string> Description3, ExpectedDescription3 = {"Start at 38d 23' 60\" N, 121d 43' 12\" W",
                                                                    "Bike N toward Main St. for 6.9 mi",
                                                                    "Bike W along Main St. for 4.3 mi",
                                                                    "Bike N along B St. for 3.5 mi",
                                                                    "End at 38d 32' 60\" N, 121d 47' 60\" W"};
    EXPECT_TRUE(Planner.GetPathDescription(Path3,Description3));
    EXPECT_EQ(Description3, ExpectedDescription3);

}

TEST(CSVOSMTransporationPlanner, NullConfigurationComponents){
    auto Config = std::make_shared<STransportationPlannerConfig>(nullptr, nullptr);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TNodeID> ShortestPath;
    std::vector<CTransportationPlanner::TTripStep> FastestPath = {
        {CTransportationPlanner::ETransportationMode::Walk, 1}
    };
    std::vector<std::string> Description;

    EXPECT_EQ(Planner.NodeCount(), 0);
    EXPECT_EQ(Planner.SortedNodeByIndex(0), nullptr);
    EXPECT_EQ(Planner.FindShortestPath(1, 2, ShortestPath), CPathRouter::NoPathExists);
    EXPECT_EQ(Planner.FindFastestPath(1, 2, FastestPath), CPathRouter::NoPathExists);
    EXPECT_FALSE(Planner.GetPathDescription(FastestPath, Description));
}

TEST(CSVOSMTransporationPlanner, DisconnectedGraphNoPath){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.6\" lon=\"-121.7\"/>"
                                                            "<node id=\"3\" lat=\"38.7\" lon=\"-121.7\"/>"
                                                            "<node id=\"4\" lat=\"38.8\" lon=\"-121.7\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"3\"/>"
                                                            "<nd ref=\"4\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "101,1\n"
                                                            "102,4");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "A,101\n"
                                                             "A,102");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TNodeID> ShortestPath = {99};
    std::vector<CTransportationPlanner::TTripStep> FastestPath = {{CTransportationPlanner::ETransportationMode::Walk, 99}};

    EXPECT_EQ(Planner.FindShortestPath(1, 4, ShortestPath), CPathRouter::NoPathExists);
    EXPECT_TRUE(ShortestPath.empty());
    EXPECT_EQ(Planner.FindFastestPath(1, 4, FastestPath), CPathRouter::NoPathExists);
    EXPECT_TRUE(FastestPath.empty());
}

TEST(CSVOSMTransporationPlanner, DuplicateSegmentsPreferNamedStreetAndDefaultBusSpeed){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.5\" lon=\"-121.8\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"maxspeed\" v=\"fast\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"name\" v=\"Named Rd.\"/>"
                                                            "<tag k=\"maxspeed\" v=\"fast\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "101,1\n"
                                                            "102,2");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "A,101\n"
                                                             "A,102");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem, 1.0, 1.0, 10.0, 0.0);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TTripStep> FastestPath;
    auto Distance = SGeographicUtils::HaversineDistanceInMiles(
        CStreetMap::SLocation(38.5, -121.7),
        CStreetMap::SLocation(38.5, -121.8)
    );
    std::vector<CTransportationPlanner::TTripStep> ExpectedPath = {
        {CTransportationPlanner::ETransportationMode::Walk, 1},
        {CTransportationPlanner::ETransportationMode::Bus, 2}
    };
    std::vector<std::string> Description;
    std::vector<std::string> ExpectedDescription = {
        "Start at 38d 30' 0\" N, 121d 42' 0\" W",
        "Bike W along Named Rd. for 5.4 mi",
        "End at 38d 30' 0\" N, 121d 47' 60\" W"
    };
    std::vector<CTransportationPlanner::TTripStep> BikePath = {
        {CTransportationPlanner::ETransportationMode::Bike, 1},
        {CTransportationPlanner::ETransportationMode::Bike, 2}
    };

    EXPECT_EQ(Planner.FindFastestPath(1, 2, FastestPath), Distance / 10.0);
    EXPECT_EQ(FastestPath, ExpectedPath);
    EXPECT_TRUE(Planner.GetPathDescription(BikePath, Description));
    EXPECT_EQ(Description, ExpectedDescription);
}

TEST(CSVOSMTransporationPlanner, PathDescriptionFailureCases){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
                                                            "<node id=\"2\" lat=\"38.6\" lon=\"-121.7\"/>"
                                                            "<node id=\"3\" lat=\"38.7\" lon=\"-121.7\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"name\" v=\"A St.\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<tag k=\"name\" v=\"B St.\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);

    auto NullBusConfig = std::make_shared<STransportationPlannerConfig>(StreetMap, nullptr);
    CDijkstraTransportationPlanner PlannerWithoutBus(NullBusConfig);

    std::vector<std::string> Description;
    EXPECT_FALSE(PlannerWithoutBus.GetPathDescription({}, Description));
    EXPECT_FALSE(PlannerWithoutBus.GetPathDescription({{CTransportationPlanner::ETransportationMode::Walk, 999}}, Description));
    EXPECT_FALSE(PlannerWithoutBus.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Walk, 1},
        {CTransportationPlanner::ETransportationMode::Bus, 2}
    }, Description));
    EXPECT_FALSE(PlannerWithoutBus.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Bike, 1},
        {CTransportationPlanner::ETransportationMode::Bike, 3}
    }, Description));

    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "101,1\n"
                                                            "102,2\n"
                                                            "103,3");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "A,101\n"
                                                             "A,102\n"
                                                             "B,102\n"
                                                             "B,103");
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
    CDijkstraTransportationPlanner Planner(Config);

    EXPECT_FALSE(Planner.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Walk, 1},
        {CTransportationPlanner::ETransportationMode::Bus, 3}
    }, Description));

    std::vector<CTransportationPlanner::TTripStep> MultiBusPath = {
        {CTransportationPlanner::ETransportationMode::Walk, 1},
        {CTransportationPlanner::ETransportationMode::Bus, 2},
        {CTransportationPlanner::ETransportationMode::Bus, 3}
    };
    std::vector<std::string> ExpectedDescription = {
        "Start at 38d 30' 0\" N, 121d 42' 0\" W",
        "Take Bus A from stop 101 to stop 102",
        "Take Bus B from stop 102 to stop 103",
        "End at 38d 42' 0\" N, 121d 42' 0\" W"
    };

    EXPECT_TRUE(Planner.GetPathDescription(MultiBusPath, Description));
    EXPECT_EQ(Description, ExpectedDescription);
}

TEST(CSVOSMTransporationPlanner, ConstructorEdgeCasesCoverage){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.70\"/>"
                                                            "<node id=\"2\" lat=\"38.5\" lon=\"-121.71\"/>"
                                                            "<node id=\"3\" lat=\"38.5\" lon=\"-121.72\"/>"
                                                            "<node id=\"5\" lat=\"38.6\" lon=\"-121.70\"/>"
                                                            "<node id=\"6\" lat=\"38.6\" lon=\"-121.71\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"999\"/>"
                                                            "</way>"
                                                            "<way id=\"12\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"maxspeed\" v=\"999999999999999999999999999999999999999999999999\"/>"
                                                            "</way>"
                                                            "<way id=\"13\">"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<tag k=\"maxspeed\" v=\"10 mph\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "201,1\n"
                                                            "202,500\n"
                                                            "203,2\n"
                                                            "204,5\n"
                                                            "205,6\n"
                                                            "206,3\n"
                                                            "207,3");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "Solo,201\n"
                                                             "MissingVertex,202\n"
                                                             "MissingVertex,203\n"
                                                             "NoRoad,204\n"
                                                             "NoRoad,205\n"
                                                             "SameNode,206\n"
                                                             "SameNode,207");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem, 1.0, 1.0, 25.0, 30.0);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TNodeID> ShortestPath;
    EXPECT_EQ(Planner.NodeCount(), 5);
    EXPECT_EQ(Planner.FindShortestPath(1, 3, ShortestPath), 
              SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.5, -121.70), CStreetMap::SLocation(38.5, -121.71)) +
              SGeographicUtils::HaversineDistanceInMiles(CStreetMap::SLocation(38.5, -121.71), CStreetMap::SLocation(38.5, -121.72)));
}

TEST(CSVOSMTransporationPlanner, FastestPathTieBreakAndQueueCoverage){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.70\"/>"
                                                            "<node id=\"2\" lat=\"38.5\" lon=\"-121.71\"/>"
                                                            "<node id=\"3\" lat=\"38.5\" lon=\"-121.72\"/>"
                                                            "<node id=\"4\" lat=\"38.5\" lon=\"-121.73\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"maxspeed\" v=\"1 mph\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"3\"/>"
                                                            "<tag k=\"maxspeed\" v=\"10 mph\"/>"
                                                            "</way>"
                                                            "<way id=\"12\">"
                                                            "<nd ref=\"3\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"maxspeed\" v=\"10 mph\"/>"
                                                            "</way>"
                                                            "<way id=\"13\">"
                                                            "<nd ref=\"2\"/>"
                                                            "<nd ref=\"4\"/>"
                                                            "<tag k=\"maxspeed\" v=\"1 mph\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "301,1\n"
                                                            "304,4");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "B,301\n"
                                                             "B,304\n"
                                                             "A,301\n"
                                                             "A,304");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem, 0.25, 0.25, 25.0, 0.0);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TTripStep> FastestPath;
    std::vector<CTransportationPlanner::TTripStep> ExpectedPath = {
        {CTransportationPlanner::ETransportationMode::Walk, 1},
        {CTransportationPlanner::ETransportationMode::Bus, 4}
    };
    std::vector<std::string> Description;
    std::vector<std::string> ExpectedDescription = {
        "Start at 38d 30' 0\" N, 121d 42' 0\" W",
        "Take Bus A from stop 301 to stop 304",
        "End at 38d 30' 0\" N, 121d 43' 48\" W"
    };

    EXPECT_NE(Planner.FindFastestPath(1, 4, FastestPath), CPathRouter::NoPathExists);
    EXPECT_EQ(FastestPath, ExpectedPath);
    EXPECT_TRUE(Planner.GetPathDescription(FastestPath, Description));
    EXPECT_EQ(Description, ExpectedDescription);
}

TEST(CSVOSMTransporationPlanner, NextNamedStreetTargetCoverage){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"10\" lat=\"38.5\" lon=\"-121.72\"/>"
                                                            "<node id=\"11\" lat=\"38.5\" lon=\"-121.73\"/>"
                                                            "<node id=\"12\" lat=\"38.5\" lon=\"-121.74\"/>"
                                                            "<node id=\"13\" lat=\"38.5\" lon=\"-121.75\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"10\"/>"
                                                            "<nd ref=\"11\"/>"
                                                            "</way>"
                                                            "<way id=\"11\">"
                                                            "<nd ref=\"12\"/>"
                                                            "<nd ref=\"13\"/>"
                                                            "<tag k=\"name\" v=\"C St.\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "401,11\n"
                                                            "402,12");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                             "A,401\n"
                                                             "A,402");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TTripStep> GoodPath = {
        {CTransportationPlanner::ETransportationMode::Bike, 10},
        {CTransportationPlanner::ETransportationMode::Bike, 11},
        {CTransportationPlanner::ETransportationMode::Bus, 12},
        {CTransportationPlanner::ETransportationMode::Walk, 13}
    };
    std::vector<std::string> Description;
    std::vector<std::string> ExpectedDescription = {
        "Start at 38d 30' 0\" N, 121d 43' 12\" W",
        "Bike W toward C St. for 0.5 mi",
        "Take Bus A from stop 401 to stop 402",
        "Walk W along C St. for 0.5 mi",
        "End at 38d 30' 0\" N, 121d 45' 0\" W"
    };

    EXPECT_TRUE(Planner.GetPathDescription(GoodPath, Description));
    EXPECT_EQ(Description, ExpectedDescription);

    std::vector<CTransportationPlanner::TTripStep> BadPath = {
        {CTransportationPlanner::ETransportationMode::Bike, 10},
        {CTransportationPlanner::ETransportationMode::Bike, 11},
        {CTransportationPlanner::ETransportationMode::Walk, 999},
        {CTransportationPlanner::ETransportationMode::Bike, 12},
        {CTransportationPlanner::ETransportationMode::Bike, 13}
    };
    EXPECT_FALSE(Planner.GetPathDescription(BadPath, Description));
}

TEST(CSVOSMTransporationPlanner, ParseSpeedLimitCatchCoverage){
    auto InStreamOSM = std::make_shared<CStringDataSource>( "<?xml version='1.0' encoding='UTF-8'?>"
                                                            "<osm version=\"0.6\">"
                                                            "<node id=\"1\" lat=\"38.5\" lon=\"-121.70\"/>"
                                                            "<node id=\"2\" lat=\"38.5\" lon=\"-121.71\"/>"
                                                            "<way id=\"10\">"
                                                            "<nd ref=\"1\"/>"
                                                            "<nd ref=\"2\"/>"
                                                            "<tag k=\"maxspeed\" v=\"....\"/>"
                                                            "</way>"
                                                            "</osm>");
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id");
    auto XMLReader = std::make_shared<CXMLReader>(InStreamOSM);
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops, ',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes, ',');
    auto StreetMap = std::make_shared<COpenStreetMap>(XMLReader);
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem, 3.0, 8.0, 25.0, 30.0);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<CTransportationPlanner::TTripStep> Path;
    auto Distance = SGeographicUtils::HaversineDistanceInMiles(
        CStreetMap::SLocation(38.5, -121.70),
        CStreetMap::SLocation(38.5, -121.71)
    );
    EXPECT_EQ(Planner.FindFastestPath(1, 2, Path), Distance / 8.0);
}

TEST(CSVOSMTransporationPlanner, InconsistentMapAndBusCoverage){
    auto StreetMap = std::make_shared<CTestStreetMap>();
    StreetMap->AddNode(1, 38.5, -121.70);
    StreetMap->AddNode(2, 38.5, -121.71);
    StreetMap->AddNode(3, 38.5, -121.72, false);
    StreetMap->AddNode(10, 38.6, -121.70);
    StreetMap->AddNode(11, 38.6, -121.71);
    StreetMap->AddWay(10, {1});
    StreetMap->AddWay(11, {1, 999});
    StreetMap->AddWay(12, {1, 3});

    auto BusSystem = std::make_shared<CTestBusSystem>();
    BusSystem->AddIndexedStop(1, 1);
    BusSystem->AddIndexedStop(2, 10);
    BusSystem->AddIndexedStop(3, 11);
    BusSystem->SetRouteCountResponses({0, 4});
    BusSystem->AddRoute("TooShort", {1});
    BusSystem->AddRoute("MissingStop", {1, 999});
    BusSystem->AddRoute("MissingVertex", {1, 500});
    BusSystem->AddLookupOnlyStop(500, 500);
    BusSystem->AddRoute("NoRoad", {2, 3});

    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
    CDijkstraTransportationPlanner Planner(Config);

    EXPECT_EQ(Planner.NodeCount(), 4);
    EXPECT_EQ(Planner.SortedNodeByIndex(99), nullptr);

    std::vector<CTransportationPlanner::TNodeID> ShortestPath;
    EXPECT_EQ(Planner.FindShortestPath(1, 3, ShortestPath), CPathRouter::NoPathExists);
}

TEST(CSVOSMTransporationPlanner, DuplicateNamedWayReplacesLongerSegment){
    auto StreetMap = std::make_shared<CTestStreetMap>();
    StreetMap->AddNode(1, 38.5, -121.70);
    StreetMap->AddNode(2, 38.5, -121.71);
    StreetMap->SetNodeLookupResponses(2, {
        {38.5, -121.80},
        {38.5, -121.71}
    });
    StreetMap->AddWay(10, {1, 2}, {{"name", "Main St."}});
    StreetMap->AddWay(11, {1, 2}, {{"name", "Main St."}});

    auto BusSystem = std::make_shared<CTestBusSystem>();
    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<std::string> Description;
    EXPECT_TRUE(Planner.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Bike, 1},
        {CTransportationPlanner::ETransportationMode::Bike, 2}
    }, Description));
    EXPECT_NE(Description[1].find("Main St."), std::string::npos);
    EXPECT_NE(Description[1].find("0.5 mi"), std::string::npos);
}

TEST(CSVOSMTransporationPlanner, PathDescriptionCorruptPublicInputs){
    auto StreetMap = std::make_shared<CTestStreetMap>();
    StreetMap->AddNode(1, 38.5, -121.70);
    StreetMap->AddNode(2, 38.5, -121.71);
    StreetMap->AddNode(3, 38.5, -121.72);
    StreetMap->AddWay(10, {1, 2}, {{"name", "Alpha St."}});

    auto BusSystem = std::make_shared<CTestBusSystem>();
    BusSystem->AddLookupOnlyStop(101, 1);
    BusSystem->AddLookupOnlyStop(102, 2);
    BusSystem->AddRoute("Ghost", {101, 102});

    auto Config = std::make_shared<STransportationPlannerConfig>(StreetMap, BusSystem);
    CDijkstraTransportationPlanner Planner(Config);

    std::vector<std::string> Description;
    EXPECT_FALSE(Planner.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Walk, 1},
        {CTransportationPlanner::ETransportationMode::Bus, 2}
    }, Description));

    EXPECT_FALSE(Planner.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Bike, 1},
        {CTransportationPlanner::ETransportationMode::Bike, 2},
        {CTransportationPlanner::ETransportationMode::Bike, 3}
    }, Description));

    StreetMap->AddWay(11, {2, 3}, {{"name", "Beta St."}});
    CDijkstraTransportationPlanner PlannerWithSecondWay(Config);
    StreetMap->HideNode(2);

    EXPECT_FALSE(PlannerWithSecondWay.GetPathDescription({
        {CTransportationPlanner::ETransportationMode::Bike, 1},
        {CTransportationPlanner::ETransportationMode::Bike, 2},
        {CTransportationPlanner::ETransportationMode::Bike, 3}
    }, Description));
}
