#include <gtest/gtest.h>

#include "StringDataSource.h"
#include "XMLReader.h"
#include "OpenStreetMap.h"

static std::shared_ptr<COpenStreetMap> BuildMapFromXML(const std::string &xml)
{
    auto source = std::make_shared<CStringDataSource>(xml);
    auto reader = std::make_shared<CXMLReader>(source);
    return std::make_shared<COpenStreetMap>(reader);
}

TEST(OpenStreetMapTest, SingleNodeByIndexAndID)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    EXPECT_EQ(Map->NodeCount(), 1U);
    EXPECT_EQ(Map->WayCount(), 0U);

    auto Node0 = Map->NodeByIndex(0);
    ASSERT_NE(Node0, nullptr);

    auto NodeByID = Map->NodeByID(1ULL);
    ASSERT_NE(NodeByID, nullptr);

    EXPECT_EQ(Map->NodeByIndex(1), nullptr);
    EXPECT_EQ(Map->NodeByID(999ULL), nullptr);

    EXPECT_EQ(Node0->ID(), 1ULL);

    auto Loc = Node0->Location();
    EXPECT_DOUBLE_EQ(Loc.first, 38.5);    // lat
    EXPECT_DOUBLE_EQ(Loc.second, -121.7); // lon
}

TEST(OpenStreetMapTest, EmptyMapCounts)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    EXPECT_EQ(Map->NodeCount(), 0U);
    EXPECT_EQ(Map->WayCount(), 0U);
}

TEST(OpenStreetMapTest, MissingLookupsReturnNull)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    EXPECT_EQ(Map->NodeCount(), 0U);
    EXPECT_EQ(Map->WayCount(), 0U);

    EXPECT_EQ(Map->NodeByIndex(0), nullptr);
    EXPECT_EQ(Map->NodeByID(999ULL), nullptr);
    EXPECT_EQ(Map->WayByIndex(0), nullptr);
    EXPECT_EQ(Map->WayByID(999ULL), nullptr);
}

TEST(OpenStreetMapTest, NodeAttributes)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <node id=\"1\" lat=\"38.5\" lon=\"-121.7\">"
        "    <tag k=\"name\" v=\"Test Stop\"/>"
        "    <tag k=\"highway\" v=\"bus_stop\"/>"
        "  </node>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    ASSERT_EQ(Map->NodeCount(), 1U);

    auto Node0 = Map->NodeByID(1ULL);
    ASSERT_NE(Node0, nullptr);

    EXPECT_EQ(Node0->AttributeCount(), 2U);
    EXPECT_TRUE(Node0->HasAttribute("name"));
    EXPECT_TRUE(Node0->HasAttribute("highway"));
    EXPECT_FALSE(Node0->HasAttribute("missing"));

    EXPECT_EQ(Node0->GetAttribute("name"), "Test Stop");
    EXPECT_EQ(Node0->GetAttribute("highway"), "bus_stop");
    EXPECT_EQ(Node0->GetAttribute("missing"), "");

    EXPECT_EQ(Node0->GetAttributeKey(0), "name");
    EXPECT_EQ(Node0->GetAttributeKey(1), "highway");
    EXPECT_EQ(Node0->GetAttributeKey(2), "");
}

TEST(OpenStreetMapTest, SingleWayRefsAndAttributes)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <way id=\"8699536\">"
        "    <nd ref=\"258592863\"/>"
        "    <nd ref=\"4399281377\"/>"
        "    <nd ref=\"62224286\"/>"
        "    <tag k=\"lanes\" v=\"2\"/>"
        "    <tag k=\"oneway\" v=\"yes\"/>"
        "    <tag k=\"highway\" v=\"motorway_link\"/>"
        "  </way>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    EXPECT_EQ(Map->NodeCount(), 0U);
    EXPECT_EQ(Map->WayCount(), 1U);

    auto Way0 = Map->WayByIndex(0);
    ASSERT_NE(Way0, nullptr);

    auto WayByID = Map->WayByID(8699536ULL);
    ASSERT_NE(WayByID, nullptr);

    EXPECT_EQ(Way0->ID(), 8699536ULL);

    EXPECT_EQ(Way0->NodeCount(), 3U);
    EXPECT_EQ(Way0->GetNodeID(0), 258592863ULL);
    EXPECT_EQ(Way0->GetNodeID(1), 4399281377ULL);
    EXPECT_EQ(Way0->GetNodeID(2), 62224286ULL);

    EXPECT_EQ(Way0->AttributeCount(), 3U);
    EXPECT_TRUE(Way0->HasAttribute("lanes"));
    EXPECT_TRUE(Way0->HasAttribute("oneway"));
    EXPECT_TRUE(Way0->HasAttribute("highway"));
    EXPECT_FALSE(Way0->HasAttribute("name"));

    EXPECT_EQ(Way0->GetAttribute("lanes"), "2");
    EXPECT_EQ(Way0->GetAttribute("oneway"), "yes");
    EXPECT_EQ(Way0->GetAttribute("highway"), "motorway_link");
    EXPECT_EQ(Way0->GetAttribute("name"), "");
}

TEST(OpenStreetMapTest, WayMissingLookupsAndBounds)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <way id=\"42\">"
        "    <nd ref=\"100\"/>"
        "    <tag k=\"name\" v=\"Demo Way\"/>"
        "  </way>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    ASSERT_EQ(Map->WayCount(), 1U);

    auto Way0 = Map->WayByIndex(0);
    ASSERT_NE(Way0, nullptr);

    EXPECT_EQ(Map->WayByIndex(1), nullptr);
    EXPECT_EQ(Map->WayByID(999ULL), nullptr);

    EXPECT_EQ(Way0->GetNodeID(0), 100ULL);
    EXPECT_EQ(Way0->GetNodeID(1), std::numeric_limits<CStreetMap::TNodeID>::max());

    EXPECT_EQ(Way0->GetAttributeKey(0), "name");
    EXPECT_EQ(Way0->GetAttributeKey(1), "");
}

// Dumb tests so that gencoverage shows as 100%

TEST(OpenStreetMapTest, InvalidNodeInputsAreSkipped)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <node id=\"10\" lat=\"38.1\"/>"                 // missing lon -> 162,163
        "  <node id=\"abc\" lat=\"38.2\" lon=\"-121.2\"/>" // bad id -> 183-186
        "  <node id=\"1\" lat=\"38.5\" lon=\"-121.7\"/>"   // valid
        "  <node id=\"1\" lat=\"99.9\" lon=\"99.9\"/>"     // duplicate id -> 174,175
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    EXPECT_EQ(Map->NodeCount(), 1U);

    auto Node = Map->NodeByID(1ULL);
    ASSERT_NE(Node, nullptr);

    auto Loc = Node->Location();
    EXPECT_DOUBLE_EQ(Loc.first, 38.5);
    EXPECT_DOUBLE_EQ(Loc.second, -121.7);
}

TEST(OpenStreetMapTest, InvalidWayInputsAreSkipped)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <way>" // missing id -> 192,193
        "    <nd ref=\"1\"/>"
        "  </way>"
        "  <way id=\"abc\">" // bad way id -> 211-214
        "    <nd ref=\"2\"/>"
        "  </way>"
        "  <way id=\"42\">" // valid
        "    <nd ref=\"100\"/>"
        "    <tag k=\"name\" v=\"Main\"/>"
        "  </way>"
        "  <way id=\"42\">" // duplicate way id -> 202,203
        "    <nd ref=\"999\"/>"
        "  </way>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    EXPECT_EQ(Map->WayCount(), 1U);

    auto Way = Map->WayByID(42ULL);
    ASSERT_NE(Way, nullptr);
    EXPECT_EQ(Way->NodeCount(), 1U);
    EXPECT_EQ(Way->GetNodeID(0), 100ULL);
    EXPECT_EQ(Way->GetAttribute("name"), "Main");
}

TEST(OpenStreetMapTest, MalformedNdRefIsIgnored)
{
    std::string XML =
        "<osm version=\"0.6\">"
        "  <way id=\"7\">"
        "    <nd ref=\"not_a_number\"/>" // bad nd ref -> 225-228
        "    <nd ref=\"123\"/>"
        "    <tag k=\"highway\" v=\"residential\"/>"
        "  </way>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);

    ASSERT_NE(Map, nullptr);
    ASSERT_EQ(Map->WayCount(), 1U);

    auto Way = Map->WayByID(7ULL);
    ASSERT_NE(Way, nullptr);

    EXPECT_EQ(Way->NodeCount(), 1U); // malformed ref was ignored?
    EXPECT_EQ(Way->GetNodeID(0), 123ULL);
    EXPECT_EQ(Way->GetAttribute("highway"), "residential");
}
