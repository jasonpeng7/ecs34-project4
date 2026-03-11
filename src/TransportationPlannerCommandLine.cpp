#include "TransportationPlannerCommandLine.h"

#include "GeographicUtils.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string ReadLine(const std::shared_ptr<CDataSource> &source) {
    std::string Result;
    char Character;

    while (source && source->Get(Character)) {
        if (Character == '\r') {
            continue;
        }
        if (Character == '\n') {
            break;
        }
        Result.push_back(Character);
    }

    return Result;
}

bool TryParseUnsigned(const std::string &text, std::uint64_t &value) {
    try {
        std::size_t ParsedCharacters = 0;
        value = std::stoull(text, &ParsedCharacters);
        return ParsedCharacters == text.size();
    }
    catch (...) {
        return false;
    }
}

std::string FormatDistance(double distance) {
    std::ostringstream Out;
    Out << std::fixed << std::setprecision(1) << distance;
    return Out.str();
}

std::string FormatHoursForFilename(double hours) {
    std::ostringstream Out;
    Out << std::fixed << std::setprecision(6) << hours;
    return Out.str();
}

std::string FormatDuration(double hours) {
    auto TotalSeconds = static_cast<long long>(hours * 3600.0 + 0.5);
    auto HourCount = TotalSeconds / 3600;
    TotalSeconds %= 3600;
    auto MinuteCount = TotalSeconds / 60;
    auto SecondCount = TotalSeconds % 60;

    std::ostringstream Out;
    if (HourCount > 0) {
        Out << HourCount << " hr";
        if (MinuteCount > 0 || SecondCount > 0) {
            Out << " ";
        }
    }
    if (MinuteCount > 0) {
        Out << MinuteCount << " min";
        if (SecondCount > 0) {
            Out << " ";
        }
    }
    if (SecondCount > 0 || (HourCount == 0 && MinuteCount == 0)) {
        Out << SecondCount << " sec";
    }

    return Out.str();
}

std::string ModeToString(CTransportationPlanner::ETransportationMode mode) {
    if (mode == CTransportationPlanner::ETransportationMode::Bike) {
        return "Bike";
    }
    if (mode == CTransportationPlanner::ETransportationMode::Bus) {
        return "Bus";
    }
    return "Walk";
}

} // namespace

struct CTransportationPlannerCommandLine::SImplementation {
    std::shared_ptr<CDataSource> DCommandSource;
    std::shared_ptr<CDataSink> DOutputSink;
    std::shared_ptr<CDataSink> DErrorSink;
    std::shared_ptr<CDataFactory> DResultsFactory;
    std::shared_ptr<CTransportationPlanner> DPlanner;

    bool DHasValidFastestPath = false;
    CTransportationPlanner::TNodeID DLastFastestSrc = 0;
    CTransportationPlanner::TNodeID DLastFastestDest = 0;
    double DLastFastestTime = CPathRouter::NoPathExists;
    std::vector<CTransportationPlanner::TTripStep> DLastFastestPath;

    SImplementation(
        std::shared_ptr<CDataSource> cmdsrc,
        std::shared_ptr<CDataSink> outsink,
        std::shared_ptr<CDataSink> errsink,
        std::shared_ptr<CDataFactory> results,
        std::shared_ptr<CTransportationPlanner> planner
    )
        : DCommandSource(cmdsrc),
          DOutputSink(outsink),
          DErrorSink(errsink),
          DResultsFactory(results),
          DPlanner(planner) {
    }

    void WriteToSink(const std::shared_ptr<CDataSink> &sink, const std::string &text) const {
        if (!sink) {
            return;
        }
        sink->Write(std::vector<char>(text.begin(), text.end()));
    }

    void WriteOutput(const std::string &text) const {
        WriteToSink(DOutputSink, text);
    }

    void WriteError(const std::string &text) const {
        WriteToSink(DErrorSink, text);
    }

    void WritePrompt() const {
        WriteOutput("> ");
    }

    void InvalidateLastFastestPath() {
        DHasValidFastestPath = false;
        DLastFastestTime = CPathRouter::NoPathExists;
        DLastFastestPath.clear();
    }

    bool HandleHelp() {
        WriteOutput(
            "------------------------------------------------------------------------\n"
            "help     Display this help menu\n"
            "exit     Exit the program\n"
            "count    Output the number of nodes in the map\n"
            "node     Syntax \"node [0, count)\" \n"
            "         Will output node ID and Lat/Lon for node\n"
            "fastest  Syntax \"fastest start end\" \n"
            "         Calculates the time for fastest path from start to end\n"
            "shortest Syntax \"shortest start end\" \n"
            "         Calculates the distance for the shortest path from start to end\n"
            "save     Saves the last calculated path to file\n"
            "print    Prints the steps for the last calculated path\n"
        );
        return true;
    }

    bool HandleCount() {
        WriteOutput(std::to_string(DPlanner->NodeCount()) + " nodes\n");
        return true;
    }

    bool HandleNode(const std::vector<std::string> &tokens) {
        if (tokens.size() != 2) {
            WriteError("Invalid node command, see help.\n");
            return true;
        }

        std::uint64_t Index = 0;
        if (!TryParseUnsigned(tokens[1], Index)) {
            WriteError("Invalid node parameter, see help.\n");
            return true;
        }

        if (Index >= DPlanner->NodeCount()) {
            WriteError("Invalid node parameter, see help.\n");
            return true;
        }

        auto Node = DPlanner->SortedNodeByIndex(Index);
        if (!Node) {
            WriteError("Invalid node parameter, see help.\n");
            return true;
        }

        WriteOutput(
            "Node " + std::to_string(Index) +
            ": id = " + std::to_string(Node->ID()) +
            " is at " + SGeographicUtils::ConvertLLToDMS(Node->Location()) + "\n"
        );
        return true;
    }

    bool HandleShortest(const std::vector<std::string> &tokens) {
        if (tokens.size() != 3) {
            WriteError("Invalid shortest command, see help.\n");
            return true;
        }

        std::uint64_t Src = 0;
        std::uint64_t Dest = 0;
        if (!TryParseUnsigned(tokens[1], Src) || !TryParseUnsigned(tokens[2], Dest)) {
            WriteError("Invalid shortest parameter, see help.\n");
            return true;
        }

        std::vector<CTransportationPlanner::TNodeID> Path;
        auto Distance = DPlanner->FindShortestPath(Src, Dest, Path);
        if (Distance == CPathRouter::NoPathExists) {
            WriteError("No path found.\n");
            return true;
        }

        WriteOutput("Shortest path is " + FormatDistance(Distance) + " mi.\n");
        return true;
    }

    bool HandleFastest(const std::vector<std::string> &tokens) {
        if (tokens.size() != 3) {
            WriteError("Invalid fastest command, see help.\n");
            return true;
        }

        std::uint64_t Src = 0;
        std::uint64_t Dest = 0;
        if (!TryParseUnsigned(tokens[1], Src) || !TryParseUnsigned(tokens[2], Dest)) {
            WriteError("Invalid fastest parameter, see help.\n");
            return true;
        }

        std::vector<CTransportationPlanner::TTripStep> Path;
        auto Time = DPlanner->FindFastestPath(Src, Dest, Path);
        if (Time == CPathRouter::NoPathExists) {
            InvalidateLastFastestPath();
            WriteError("No path found.\n");
            return true;
        }

        DHasValidFastestPath = true;
        DLastFastestSrc = Src;
        DLastFastestDest = Dest;
        DLastFastestTime = Time;
        DLastFastestPath = Path;

        WriteOutput("Fastest path takes " + FormatDuration(Time) + ".\n");
        return true;
    }

    bool HandleSave() {
        if (!DHasValidFastestPath || !DResultsFactory) {
            WriteError("No valid path to save, see help.\n");
            return true;
        }

        auto Filename =
            std::to_string(DLastFastestSrc) + "_" +
            std::to_string(DLastFastestDest) + "_" +
            FormatHoursForFilename(DLastFastestTime) + "hr.csv";

        auto SaveSink = DResultsFactory->CreateSink(Filename);
        if (!SaveSink) {
            WriteError("No valid path to save, see help.\n");
            return true;
        }

        std::ostringstream Out;
        Out << "mode,node_id\n";
        for (std::size_t Index = 0; Index < DLastFastestPath.size(); Index++) {
            Out << ModeToString(DLastFastestPath[Index].first) << "," << DLastFastestPath[Index].second;
            if (Index + 1 < DLastFastestPath.size()) {
                Out << "\n";
            }
        }
        WriteToSink(SaveSink, Out.str());
        WriteOutput("Path saved to <results>/" + Filename + "\n");
        return true;
    }

    bool HandlePrint() {
        if (!DHasValidFastestPath) {
            WriteError("No valid path to print, see help.\n");
            return true;
        }

        std::vector<std::string> Description;
        if (!DPlanner->GetPathDescription(DLastFastestPath, Description)) {
            WriteError("No valid path to print, see help.\n");
            return true;
        }

        for (const auto &Line : Description) {
            WriteOutput(Line + "\n");
        }
        return true;
    }

    bool ProcessCommands() {
        while (true) {
            WritePrompt();

            if (!DCommandSource || DCommandSource->End()) {
                return true;
            }

            auto Line = ReadLine(DCommandSource);
            std::istringstream Input(Line);
            std::vector<std::string> Tokens;
            std::string Token;
            while (Input >> Token) {
                Tokens.push_back(Token);
            }

            if (Tokens.empty()) {
                continue;
            }

            if (Tokens[0] == "exit") {
                return true;
            }
            if (Tokens[0] == "help") {
                HandleHelp();
                continue;
            }
            if (Tokens[0] == "count") {
                HandleCount();
                continue;
            }
            if (Tokens[0] == "node") {
                HandleNode(Tokens);
                continue;
            }
            if (Tokens[0] == "shortest") {
                HandleShortest(Tokens);
                continue;
            }
            if (Tokens[0] == "fastest") {
                HandleFastest(Tokens);
                continue;
            }
            if (Tokens[0] == "save") {
                HandleSave();
                continue;
            }
            if (Tokens[0] == "print") {
                HandlePrint();
                continue;
            }

            WriteError("Unknown command \"" + Tokens[0] + "\" type help for help.\n");
        }
    }
};

CTransportationPlannerCommandLine::CTransportationPlannerCommandLine(
    std::shared_ptr<CDataSource> cmdsrc,
    std::shared_ptr<CDataSink> outsink,
    std::shared_ptr<CDataSink> errsink,
    std::shared_ptr<CDataFactory> results,
    std::shared_ptr<CTransportationPlanner> planner
) {
    DImplementation = std::make_unique<SImplementation>(cmdsrc, outsink, errsink, results, planner);
}

CTransportationPlannerCommandLine::~CTransportationPlannerCommandLine() = default;

bool CTransportationPlannerCommandLine::ProcessCommands() {
    return DImplementation->ProcessCommands();
}
