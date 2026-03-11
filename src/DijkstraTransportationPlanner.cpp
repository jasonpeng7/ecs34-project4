#include "DijkstraTransportationPlanner.h"

#include "BusSystemIndexer.h"
#include "DijkstraPathRouter.h"
#include "GeographicUtils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

    template <typename T>
    struct SPairHash
    {
        std::size_t operator()(const std::pair<T, T> &value) const noexcept
        {
            return std::hash<T>{}(value.first) ^ (std::hash<T>{}(value.second) << 1);
        }
    };

    struct SStreetSegmentInfo
    {
        CStreetMap::TWayID DWayID = CStreetMap::InvalidWayID;
        double DDistance = 0.0;
        std::string DName;
    };

    struct SFastestEdgeInfo
    {
        CTransportationPlanner::ETransportationMode DMode = CTransportationPlanner::ETransportationMode::Walk;
        double DWeight = CPathRouter::NoPathExists;
        std::string DRouteName;
    };

    std::string ModeToString(CTransportationPlanner::ETransportationMode mode)
    {
        if (mode == CTransportationPlanner::ETransportationMode::Bike)
        {
            return "Bike";
        }
        if (mode == CTransportationPlanner::ETransportationMode::Bus)
        {
            return "Bus";
        }
        return "Walk";
    }

    std::string DistanceToString(double distance)
    {
        std::ostringstream Out;
        Out << std::fixed << std::setprecision(1) << distance << " mi";
        return Out.str();
    }

    double ParseSpeedLimit(const std::shared_ptr<CStreetMap::SWay> &way, double defaultspeed)
    {
        if (!way || !way->HasAttribute("maxspeed"))
        {
            return defaultspeed;
        }

        std::string Digits;
        bool SeenDigit = false;
        auto Value = way->GetAttribute("maxspeed");
        for (char Character : Value)
        {
            auto UnsignedCharacter = static_cast<unsigned char>(Character);
            if (std::isdigit(UnsignedCharacter) || Character == '.')
            {
                Digits.push_back(Character);
                SeenDigit = true;
            }
            else if (SeenDigit)
            {
                break;
            }
        }

        if (Digits.empty())
        {
            return defaultspeed;
        }

        auto Parsed = std::strtod(Digits.c_str(), nullptr);
        return Parsed > 0.0 ? Parsed : defaultspeed;
    }

} // namespace

struct CDijkstraTransportationPlanner::SImplementation
{
    using TNodeID = CTransportationPlanner::TNodeID;
    using TNodePair = std::pair<TNodeID, TNodeID>;

    std::shared_ptr<SConfiguration> DConfig;
    std::shared_ptr<CStreetMap> DStreetMap;
    std::shared_ptr<CBusSystem> DBusSystem;
    std::unique_ptr<CBusSystemIndexer> DBusIndexer;

    std::vector<std::shared_ptr<CStreetMap::SNode>> DSortedNodes;

    std::unordered_map<TNodeID, CPathRouter::TVertexID> DShortestVertexByNode;
    std::unordered_map<TNodeID, CPathRouter::TVertexID> DFastestVertexByNode;
    std::vector<TNodeID> DNodeByShortestVertex;
    std::vector<TNodeID> DNodeByFastestVertex;

    std::unordered_map<TNodeID, std::vector<std::pair<TNodeID, double>>> DDriveAdjacency;
    std::unordered_map<TNodePair, SStreetSegmentInfo, SPairHash<TNodeID>> DStreetSegments;
    std::unordered_map<TNodePair, std::vector<SFastestEdgeInfo>, SPairHash<TNodeID>> DFastestEdgesByPair;

    CDijkstraPathRouter DShortestRouter;
    CDijkstraPathRouter DFastestRouter;

    explicit SImplementation(std::shared_ptr<SConfiguration> config)
        : DConfig(config),
          DStreetMap(config ? config->StreetMap() : nullptr),
          DBusSystem(config ? config->BusSystem() : nullptr)
    {
        if (DBusSystem)
        {
            DBusIndexer = std::make_unique<CBusSystemIndexer>(DBusSystem);
        }
        BuildNodeLists();
        BuildStreetGraphs();
        BuildBusEdges();
    }

    void BuildNodeLists()
    {
        if (!DStreetMap)
        {
            return;
        }

        DSortedNodes.reserve(DStreetMap->NodeCount());
        for (std::size_t Index = 0; Index < DStreetMap->NodeCount(); Index++)
        {
            auto Node = DStreetMap->NodeByIndex(Index);
            if (Node)
            {
                DSortedNodes.push_back(Node);
            }
        }

        std::sort(
            DSortedNodes.begin(),
            DSortedNodes.end(),
            [](const auto &Left, const auto &Right)
            {
                return Left->ID() < Right->ID();
            });

        for (const auto &Node : DSortedNodes)
        {
            auto ShortestVertex = DShortestRouter.AddVertex(Node->ID());
            DShortestVertexByNode[Node->ID()] = ShortestVertex;
            DNodeByShortestVertex.push_back(Node->ID());

            auto FastestVertex = DFastestRouter.AddVertex(Node->ID());
            DFastestVertexByNode[Node->ID()] = FastestVertex;
            DNodeByFastestVertex.push_back(Node->ID());
        }
    }

    void RememberStreetSegment(
        TNodeID src,
        TNodeID dest,
        CStreetMap::TWayID wayid,
        const std::string &name,
        double distance)
    {
        auto Key = std::make_pair(src, dest);
        auto Search = DStreetSegments.find(Key);
        if (Search == DStreetSegments.end())
        {
            DStreetSegments[Key] = {wayid, distance, name};
            return;
        }

        auto ReplaceExisting = false;
        if (Search->second.DName.empty() && !name.empty())
        {
            ReplaceExisting = true;
        }
        if (ReplaceExisting)
        {
            Search->second = {wayid, distance, name};
        }
    }

    void AddDriveEdge(TNodeID src, TNodeID dest, double weight)
    {
        DDriveAdjacency[src].push_back(std::make_pair(dest, weight));
    }

    void AddFastestCandidate(TNodeID src, TNodeID dest, const SFastestEdgeInfo &edge)
    {
        DFastestEdgesByPair[std::make_pair(src, dest)].push_back(edge);
    }

    void BuildStreetGraphs()
    {
        if (!DStreetMap || !DConfig)
        {
            return;
        }

        for (std::size_t Index = 0; Index < DStreetMap->WayCount(); Index++)
        {
            auto Way = DStreetMap->WayByIndex(Index);
            if (!Way || Way->NodeCount() < 2)
            {
                continue;
            }

            auto SpeedLimit = ParseSpeedLimit(Way, DConfig->DefaultSpeedLimit());
            auto Name = Way->HasAttribute("name") ? Way->GetAttribute("name") : std::string();
            auto Bidirectional = !(Way->HasAttribute("oneway") && Way->GetAttribute("oneway") == "yes");

            for (std::size_t NodeIndex = 1; NodeIndex < Way->NodeCount(); NodeIndex++)
            {
                auto SrcNodeID = Way->GetNodeID(NodeIndex - 1);
                auto DestNodeID = Way->GetNodeID(NodeIndex);

                auto SrcNode = DStreetMap->NodeByID(SrcNodeID);
                auto DestNode = DStreetMap->NodeByID(DestNodeID);
                if (!SrcNode || !DestNode)
                {
                    continue;
                }

                auto SrcShortestVertex = DShortestVertexByNode.at(SrcNodeID);
                auto DestShortestVertex = DShortestVertexByNode.at(DestNodeID);
                auto SrcFastestVertex = DFastestVertexByNode.at(SrcNodeID);
                auto DestFastestVertex = DFastestVertexByNode.at(DestNodeID);

                auto Distance = SGeographicUtils::HaversineDistanceInMiles(
                    SrcNode->Location(),
                    DestNode->Location());

                auto WalkTime = Distance / DConfig->WalkSpeed();
                auto BikeTime = Distance / DConfig->BikeSpeed();
                auto DriveTime = Distance / SpeedLimit;

                DShortestRouter.AddEdge(
                    SrcShortestVertex,
                    DestShortestVertex,
                    Distance,
                    Bidirectional);

                DFastestRouter.AddEdge(
                    SrcFastestVertex,
                    DestFastestVertex,
                    WalkTime,
                    Bidirectional);
                DFastestRouter.AddEdge(
                    SrcFastestVertex,
                    DestFastestVertex,
                    BikeTime,
                    Bidirectional);

                RememberStreetSegment(SrcNodeID, DestNodeID, Way->ID(), Name, Distance);
                AddDriveEdge(SrcNodeID, DestNodeID, DriveTime);
                AddFastestCandidate(
                    SrcNodeID,
                    DestNodeID,
                    {ETransportationMode::Walk, WalkTime, std::string()});
                AddFastestCandidate(
                    SrcNodeID,
                    DestNodeID,
                    {ETransportationMode::Bike, BikeTime, std::string()});

                if (Bidirectional)
                {
                    RememberStreetSegment(DestNodeID, SrcNodeID, Way->ID(), Name, Distance);
                    AddDriveEdge(DestNodeID, SrcNodeID, DriveTime);
                    AddFastestCandidate(
                        DestNodeID,
                        SrcNodeID,
                        {ETransportationMode::Walk, WalkTime, std::string()});
                    AddFastestCandidate(
                        DestNodeID,
                        SrcNodeID,
                        {ETransportationMode::Bike, BikeTime, std::string()});
                }
            }
        }
    }

    double FindDriveTimeBetweenNodes(TNodeID src, TNodeID dest) const
    {
        if (src == dest)
        {
            return 0.0;
        }

        using TQueueEntry = std::pair<double, TNodeID>;
        std::priority_queue<TQueueEntry, std::vector<TQueueEntry>, std::greater<TQueueEntry>> Pending;
        std::unordered_map<TNodeID, double> BestTime;

        Pending.push(std::make_pair(0.0, src));
        BestTime[src] = 0.0;

        while (!Pending.empty())
        {
            auto Current = Pending.top();
            Pending.pop();

            auto CurrentTime = Current.first;
            auto CurrentNode = Current.second;

            auto BestSearch = BestTime.find(CurrentNode);
            if ((BestSearch != BestTime.end()) && (CurrentTime > BestSearch->second))
            {
                continue;
            }
            if (CurrentNode == dest)
            {
                return CurrentTime;
            }

            auto EdgeSearch = DDriveAdjacency.find(CurrentNode);
            if (EdgeSearch == DDriveAdjacency.end())
            {
                continue;
            }

            for (const auto &Edge : EdgeSearch->second)
            {
                auto NextNode = Edge.first;
                auto NextTime = CurrentTime + Edge.second;
                auto NextSearch = BestTime.find(NextNode);
                if ((NextSearch == BestTime.end()) || (NextTime < NextSearch->second))
                {
                    BestTime[NextNode] = NextTime;
                    Pending.push(std::make_pair(NextTime, NextNode));
                }
            }
        }

        return CPathRouter::NoPathExists;
    }

    void BuildBusEdges()
    {
        if (!DBusSystem || !DConfig)
        {
            return;
        }

        for (std::size_t RouteIndex = 0; RouteIndex < DBusSystem->RouteCount(); RouteIndex++)
        {
            auto Route = DBusSystem->RouteByIndex(RouteIndex);
            if (!Route || Route->StopCount() < 2)
            {
                continue;
            }

            for (std::size_t StopIndex = 1; StopIndex < Route->StopCount(); StopIndex++)
            {
                auto SrcStop = DBusSystem->StopByID(Route->GetStopID(StopIndex - 1));
                auto DestStop = DBusSystem->StopByID(Route->GetStopID(StopIndex));

                auto SrcVertex = DFastestVertexByNode.at(SrcStop->NodeID());
                auto DestVertex = DFastestVertexByNode.at(DestStop->NodeID());

                auto DriveTime = FindDriveTimeBetweenNodes(SrcStop->NodeID(), DestStop->NodeID());
                if (DriveTime == CPathRouter::NoPathExists)
                {
                    continue;
                }

                auto BusTime = DriveTime + (DConfig->BusStopTime() / 3600.0);
                DFastestRouter.AddEdge(SrcVertex, DestVertex, BusTime, false);
                AddFastestCandidate(
                    SrcStop->NodeID(),
                    DestStop->NodeID(),
                    {ETransportationMode::Bus, BusTime, Route->Name()});
            }
        }
    }

    std::size_t NodeCount() const noexcept
    {
        return DSortedNodes.size();
    }

    std::shared_ptr<CStreetMap::SNode> SortedNodeByIndex(std::size_t index) const noexcept
    {
        if (index >= DSortedNodes.size())
        {
            return nullptr;
        }
        return DSortedNodes[index];
    }

    bool LookupStreetSegment(TNodeID src, TNodeID dest, SStreetSegmentInfo &segment) const
    {
        auto Search = DStreetSegments.find(std::make_pair(src, dest));
        if (Search == DStreetSegments.end())
        {
            return false;
        }
        segment = Search->second;
        return true;
    }

    const SStreetSegmentInfo &StreetSegment(TNodeID src, TNodeID dest) const
    {
        return DStreetSegments.at(std::make_pair(src, dest));
    }

    SFastestEdgeInfo BestFastestEdge(TNodeID src, TNodeID dest) const
    {
        const auto &Candidates = DFastestEdgesByPair.at(std::make_pair(src, dest));
        auto BestEdge = Candidates.front();
        for (const auto &Candidate : Candidates)
        {
            if (Candidate.DWeight < BestEdge.DWeight)
            {
                BestEdge = Candidate;
            }
            else if ((Candidate.DWeight == BestEdge.DWeight) && (Candidate.DRouteName < BestEdge.DRouteName))
            {
                BestEdge = Candidate;
            }
        }
        return BestEdge;
    }

    double FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path)
    {
        path.clear();

        auto SrcVertex = DShortestVertexByNode.find(src);
        auto DestVertex = DShortestVertexByNode.find(dest);
        if ((SrcVertex == DShortestVertexByNode.end()) || (DestVertex == DShortestVertexByNode.end()))
        {
            return CPathRouter::NoPathExists;
        }

        std::vector<CPathRouter::TVertexID> VertexPath;
        auto Distance = DShortestRouter.FindShortestPath(SrcVertex->second, DestVertex->second, VertexPath);
        if (Distance == CPathRouter::NoPathExists)
        {
            return Distance;
        }

        path.reserve(VertexPath.size());
        for (auto VertexID : VertexPath)
        {
            path.push_back(DNodeByShortestVertex[VertexID]);
        }

        return Distance;
    }

    double FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path)
    {
        path.clear();

        auto SrcVertex = DFastestVertexByNode.find(src);
        auto DestVertex = DFastestVertexByNode.find(dest);
        if ((SrcVertex == DFastestVertexByNode.end()) || (DestVertex == DFastestVertexByNode.end()))
        {
            return CPathRouter::NoPathExists;
        }

        std::vector<CPathRouter::TVertexID> VertexPath;
        auto Time = DFastestRouter.FindShortestPath(SrcVertex->second, DestVertex->second, VertexPath);
        if (Time == CPathRouter::NoPathExists)
        {
            return Time;
        }

        std::vector<TNodeID> NodePath;
        NodePath.reserve(VertexPath.size());
        for (auto VertexID : VertexPath)
        {
            NodePath.push_back(DNodeByFastestVertex[VertexID]);
        }

        std::vector<SFastestEdgeInfo> EdgeModes;
        EdgeModes.reserve(NodePath.size());
        for (std::size_t Index = 1; Index < NodePath.size(); Index++)
        {
            EdgeModes.push_back(BestFastestEdge(NodePath[Index - 1], NodePath[Index]));
        }

        auto StartingMode = ETransportationMode::Walk;
        if (!EdgeModes.empty())
        {
            StartingMode = EdgeModes.front().DMode;
            if (StartingMode == ETransportationMode::Bus)
            {
                StartingMode = ETransportationMode::Walk;
            }
        }

        path.push_back(std::make_pair(StartingMode, NodePath.front()));
        for (std::size_t Index = 1; Index < NodePath.size(); Index++)
        {
            path.push_back(std::make_pair(EdgeModes[Index - 1].DMode, NodePath[Index]));
        }

        return Time;
    }

    std::set<std::string> RouteNamesBetweenNodes(TNodeID src, TNodeID dest) const
    {
        std::set<std::string> RouteNames;
        if (!DBusIndexer)
        {
            return RouteNames;
        }

        std::unordered_set<std::shared_ptr<CBusSystem::SRoute>> Routes;
        if (!DBusIndexer->RoutesByNodeIDs(src, dest, Routes))
        {
            return RouteNames;
        }

        for (const auto &Route : Routes)
        {
            if (Route)
            {
                RouteNames.insert(Route->Name());
            }
        }
        return RouteNames;
    }

    std::string NextNamedStreetTarget(const std::vector<TTripStep> &path, std::size_t fromindex) const
    {
        for (std::size_t Index = fromindex; Index < path.size(); Index++)
        {
            if (path[Index].first == ETransportationMode::Bus)
            {
                continue;
            }

            SStreetSegmentInfo Segment;
            if (!LookupStreetSegment(path[Index - 1].second, path[Index].second, Segment))
            {
                continue;
            }
            if (!Segment.DName.empty())
            {
                return Segment.DName;
            }
        }
        return "End";
    }

    bool GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const
    {
        desc.clear();

        if (!DStreetMap || path.empty())
        {
            return false;
        }

        auto StartNode = DStreetMap->NodeByID(path.front().second);
        auto EndNode = DStreetMap->NodeByID(path.back().second);
        if (!StartNode || !EndNode)
        {
            return false;
        }

        desc.push_back("Start at " + SGeographicUtils::ConvertLLToDMS(StartNode->Location()));

        std::size_t Index = 1;
        while (Index < path.size())
        {
            if (path[Index].first == ETransportationMode::Bus)
            {
                auto RouteNames = RouteNamesBetweenNodes(path[Index - 1].second, path[Index].second);
                if (RouteNames.empty())
                {
                    return false;
                }

                auto GroupEnd = Index;
                while ((GroupEnd + 1 < path.size()) && (path[GroupEnd + 1].first == ETransportationMode::Bus))
                {
                    auto NextRoutes = RouteNamesBetweenNodes(path[GroupEnd].second, path[GroupEnd + 1].second);
                    std::set<std::string> Intersection;
                    std::set_intersection(
                        RouteNames.begin(),
                        RouteNames.end(),
                        NextRoutes.begin(),
                        NextRoutes.end(),
                        std::inserter(Intersection, Intersection.begin()));
                    if (Intersection.empty())
                    {
                        break;
                    }
                    RouteNames = Intersection;
                    GroupEnd++;
                }

                auto StartStop = DBusIndexer->StopByNodeID(path[Index - 1].second);
                auto EndStop = DBusIndexer->StopByNodeID(path[GroupEnd].second);

                desc.push_back(
                    "Take Bus " + *RouteNames.begin() +
                    " from stop " + std::to_string(StartStop->ID()) +
                    " to stop " + std::to_string(EndStop->ID()));
                Index = GroupEnd + 1;
                continue;
            }

            SStreetSegmentInfo Segment;
            if (!LookupStreetSegment(path[Index - 1].second, path[Index].second, Segment))
            {
                return false;
            }

            auto GroupEnd = Index;
            auto TotalDistance = Segment.DDistance;
            while ((GroupEnd + 1 < path.size()) && (path[GroupEnd + 1].first == path[Index].first))
            {
                auto NextSegment = StreetSegment(path[GroupEnd].second, path[GroupEnd + 1].second);

                auto MergeNamed = !Segment.DName.empty() && (Segment.DName == NextSegment.DName);
                auto MergeUnnamed = Segment.DName.empty() && NextSegment.DName.empty() && (Segment.DWayID == NextSegment.DWayID);
                if (!MergeNamed && !MergeUnnamed)
                {
                    break;
                }

                TotalDistance += NextSegment.DDistance;
                GroupEnd++;
            }

            auto GroupStartNode = DStreetMap->NodeByID(path[Index - 1].second);
            auto GroupEndNode = DStreetMap->NodeByID(path[GroupEnd].second);

            auto Bearing = SGeographicUtils::CalculateBearing(
                GroupStartNode->Location(),
                GroupEndNode->Location());
            auto Direction = SGeographicUtils::BearingToDirection(Bearing);
            auto Prefix = ModeToString(path[Index].first) + " " + Direction + " ";

            if (Segment.DName.empty())
            {
                desc.push_back(
                    Prefix + "toward " + NextNamedStreetTarget(path, GroupEnd + 1) +
                    " for " + DistanceToString(TotalDistance));
            }
            else
            {
                desc.push_back(
                    Prefix + "along " + Segment.DName +
                    " for " + DistanceToString(TotalDistance));
            }

            Index = GroupEnd + 1;
        }

        desc.push_back("End at " + SGeographicUtils::ConvertLLToDMS(EndNode->Location()));
        return true;
    }
};

// Create unique pointer from SImplemntation
CDijkstraTransportationPlanner::CDijkstraTransportationPlanner(std::shared_ptr<SConfiguration> config)
{
    DImplementation = std::make_unique<SImplementation>(config);
}

CDijkstraTransportationPlanner::~CDijkstraTransportationPlanner() = default;

std::size_t CDijkstraTransportationPlanner::NodeCount() const noexcept
{
    return DImplementation->NodeCount();
}

std::shared_ptr<CStreetMap::SNode> CDijkstraTransportationPlanner::SortedNodeByIndex(std::size_t index) const noexcept
{
    return DImplementation->SortedNodeByIndex(index);
}

// wrapper for DImplementation
double CDijkstraTransportationPlanner::FindShortestPath(TNodeID src, TNodeID dest, std::vector<TNodeID> &path)
{
    return DImplementation->FindShortestPath(src, dest, path);
}

double CDijkstraTransportationPlanner::FindFastestPath(TNodeID src, TNodeID dest, std::vector<TTripStep> &path)
{
    return DImplementation->FindFastestPath(src, dest, path);
}

bool CDijkstraTransportationPlanner::GetPathDescription(const std::vector<TTripStep> &path, std::vector<std::string> &desc) const
{
    return DImplementation->GetPathDescription(path, desc);
}
