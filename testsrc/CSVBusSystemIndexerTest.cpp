#include <gtest/gtest.h>
#include "XMLReader.h"
#include "StringUtils.h"
#include "StringDataSource.h"
#include "DSVReader.h"
#include "CSVBusSystem.h"
#include "BusSystemIndexer.h"

TEST(CSVBusSystemIndexer, SimpleTest){
    auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id");
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    CBusSystemIndexer BusSystemIndexer(BusSystem);
    EXPECT_EQ(BusSystemIndexer.StopCount(),0);
    EXPECT_EQ(BusSystemIndexer.RouteCount(),0);
}

TEST(CSVBusSystemIndexer, StopTest){
    auto InStreamStops = std::make_shared<CStringDataSource>(   "stop_id,node_id\n"
                                                                "2,102\n"
                                                                "1,101");
    auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id");
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    CBusSystemIndexer BusSystemIndexer(BusSystem);

    EXPECT_EQ(BusSystemIndexer.StopCount(),2);
    EXPECT_EQ(BusSystemIndexer.RouteCount(),0);
    auto Stop1Index = BusSystemIndexer.SortedStopByIndex(0);
    auto Stop1Node = BusSystemIndexer.StopByNodeID(101);
    EXPECT_EQ(Stop1Index,Stop1Node);
    ASSERT_TRUE(bool(Stop1Index));
    EXPECT_EQ(Stop1Index->ID(),1);
    EXPECT_EQ(Stop1Index->NodeID(),101);
    auto Stop2Index = BusSystemIndexer.SortedStopByIndex(1);
    auto Stop2Node = BusSystemIndexer.StopByNodeID(102);
    EXPECT_EQ(Stop2Index,Stop2Node);
    ASSERT_TRUE(bool(Stop2Index));
    EXPECT_EQ(Stop2Index->ID(),2);
    EXPECT_EQ(Stop2Index->NodeID(),102);
}

TEST(CSVBusSystemIndexer, RouteTest){
    auto InStreamStops = std::make_shared<CStringDataSource>(   "stop_id,node_id\n"
                                                                "1,101\n"
                                                                "2,102");
    auto InStreamRoutes = std::make_shared<CStringDataSource>(  "route,stop_id\n"
                                                                "B,1\n"
                                                                "B,2\n"
                                                                "B,1\n"
                                                                "A,2\n"
                                                                "A,1\n"
                                                                "A,2");
    
    /*
    stops B = [1,2,1]
    stops A = [2,1,2]

    (101,102) = {B, A}
    (102,101) = {B, A}
    */
    auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
    auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
    auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
    CBusSystemIndexer BusSystemIndexer(BusSystem);

    EXPECT_EQ(BusSystemIndexer.StopCount(),2);
    EXPECT_EQ(BusSystemIndexer.RouteCount(),2);
    auto Route1Index = BusSystemIndexer.SortedRouteByIndex(0);
    ASSERT_TRUE(bool(Route1Index));
    EXPECT_EQ(Route1Index->Name(),"A");
    EXPECT_EQ(Route1Index->StopCount(),3);
    EXPECT_EQ(Route1Index->GetStopID(0),2);
    EXPECT_EQ(Route1Index->GetStopID(1),1);
    EXPECT_EQ(Route1Index->GetStopID(2),2);
    auto Route2Index = BusSystemIndexer.SortedRouteByIndex(1);
    ASSERT_TRUE(bool(Route2Index));
    EXPECT_EQ(Route2Index->Name(),"B");
    EXPECT_EQ(Route2Index->StopCount(),3);
    EXPECT_EQ(Route2Index->GetStopID(0),1);
    EXPECT_EQ(Route2Index->GetStopID(1),2);
    EXPECT_EQ(Route2Index->GetStopID(2),1);
    std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes;
    EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(101,102,Routes));
    EXPECT_EQ(Routes.size(),2);
    EXPECT_TRUE(Routes.find(Route1Index) != Routes.end());
    EXPECT_TRUE(Routes.find(Route2Index) != Routes.end());

    std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes2;
    EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(102,101,Routes2));
    EXPECT_EQ(Routes2.size(),2);
    EXPECT_TRUE(Routes2.find(Route1Index) != Routes2.end());
    EXPECT_TRUE(Routes2.find(Route2Index) != Routes2.end());

    std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes3;
    EXPECT_FALSE(BusSystemIndexer.RoutesByNodeIDs(101,103,Routes2));

}

TEST(CSVBusSystemIndexer, EmptySystemTest){
   auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n");
   auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n");
   auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
   auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
   auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
   CBusSystemIndexer BusSystemIndexer(BusSystem);

   EXPECT_EQ(BusSystemIndexer.StopCount(),0);
   EXPECT_EQ(BusSystemIndexer.RouteCount(),0);
   EXPECT_EQ(BusSystemIndexer.SortedStopByIndex(0),nullptr);
   EXPECT_EQ(BusSystemIndexer.SortedRouteByIndex(0),nullptr);
   EXPECT_EQ(BusSystemIndexer.StopByNodeID(101),nullptr);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes;
   EXPECT_FALSE(BusSystemIndexer.RoutesByNodeIDs(101,102,Routes));
   EXPECT_EQ(Routes.size(),0);
   EXPECT_FALSE(BusSystemIndexer.RouteBetweenNodeIDs(101,102));
}


TEST(CSVBusSystemIndexer, AdjacentNodesTest){
   auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "1,101\n"
                                                            "2,102\n"
                                                            "3,103\n"
                                                            "4,104");
   auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              "A,1\n"
                                                              "A,2\n"
                                                              "A,3\n"
                                                              "A,4");
    /*
    (101, 102) = {A}
    (102, 103) = {A}
    (103, 104) = {A}
    */
   auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
   auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
   auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
   CBusSystemIndexer BusSystemIndexer(BusSystem);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes1;
   EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(101,102,Routes1));
   EXPECT_EQ(Routes1.size(),1);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes2;
   EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(102,103,Routes2));
   EXPECT_EQ(Routes2.size(),1);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes3;
   EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(103,104,Routes3));
   EXPECT_EQ(Routes3.size(),1);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes4;
   EXPECT_FALSE(BusSystemIndexer.RoutesByNodeIDs(101,103,Routes4));
   EXPECT_EQ(Routes4.size(),0);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes5;
   EXPECT_FALSE(BusSystemIndexer.RoutesByNodeIDs(101,104,Routes5));
   EXPECT_EQ(Routes5.size(),0);

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes6;
   EXPECT_FALSE(BusSystemIndexer.RoutesByNodeIDs(101,101,Routes6));
   EXPECT_EQ(Routes6.size(),0);
}

TEST(CSVBusSystemIndexer, ComprehensiveTes){
   auto InStreamStops = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                            "1,101\n"
                                                            "2,102\n"
                                                            "3,103\n"
                                                            "4,104");
   auto InStreamRoutes = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              "A,1\n"
                                                              "A,2\n"
                                                              "A,3\n"
                                                              "B,2\n"
                                                              "B,3\n"
                                                              "C,3\n"
                                                              "C,2\n"
                                                              "D,3\n"
                                                              "D,4");
    /*
    stop A = [1,2,3]
    stop B = [2,3]
    stop C = [3,2]
    stop D = [3,4]

    (101, 102) = {A}
    (102, 103) = {A, B}
    (103, 102) = {C}
    (103, 104) = {D}
    */
   auto CSVReaderStops = std::make_shared<CDSVReader>(InStreamStops,',');
   auto CSVReaderRoutes = std::make_shared<CDSVReader>(InStreamRoutes,',');
   auto BusSystem = std::make_shared<CCSVBusSystem>(CSVReaderStops, CSVReaderRoutes);
   CBusSystemIndexer BusSystemIndexer(BusSystem);

    auto Stop1 = BusSystemIndexer.SortedStopByIndex(0);
    auto Stop2 = BusSystemIndexer.SortedStopByIndex(1);
    auto Stop3 = BusSystemIndexer.SortedStopByIndex(2);
    auto Stop4 = BusSystemIndexer.SortedStopByIndex(3);


   auto RouteA = BusSystemIndexer.SortedRouteByIndex(0);
   auto RouteB = BusSystemIndexer.SortedRouteByIndex(1);
   auto RouteC = BusSystemIndexer.SortedRouteByIndex(2);
   auto RouteD = BusSystemIndexer.SortedRouteByIndex(3);

   ASSERT_TRUE(bool(RouteA));
   ASSERT_TRUE(bool(RouteB));
   ASSERT_TRUE(bool(RouteC));
   ASSERT_TRUE(bool(RouteD));

   EXPECT_EQ(BusSystemIndexer.SortedStopByIndex(4), nullptr);
   EXPECT_EQ(BusSystemIndexer.SortedRouteByIndex(4), nullptr);

   EXPECT_EQ(BusSystemIndexer.StopByNodeID(101),Stop1);
   EXPECT_EQ(BusSystemIndexer.StopByNodeID(102),Stop2);
   EXPECT_EQ(BusSystemIndexer.StopByNodeID(103),Stop3);
   EXPECT_EQ(BusSystemIndexer.StopByNodeID(104),Stop4);
   EXPECT_EQ(BusSystemIndexer.StopByNodeID(999),nullptr);

   EXPECT_EQ(RouteA->Name(),"A");
   EXPECT_EQ(RouteB->Name(),"B");
   EXPECT_EQ(RouteC->Name(),"C");
   EXPECT_EQ(RouteD->Name(),"D");

   EXPECT_EQ(RouteA->StopCount(), 3);
   EXPECT_EQ(RouteB->StopCount(), 2);
   EXPECT_EQ(RouteC->StopCount(), 2);
   EXPECT_EQ(RouteD->StopCount(), 2);
   
    EXPECT_EQ(RouteA->GetStopID(0),1);
    EXPECT_EQ(RouteA->GetStopID(1),2);
    EXPECT_EQ(RouteA->GetStopID(2),3);
    EXPECT_EQ(RouteA->GetStopID(3), std::numeric_limits<CBusSystem::TStopID>::max());

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes23;
   EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(102,103,Routes23));
   EXPECT_EQ(Routes23.size(),2);
   EXPECT_TRUE(Routes23.find(RouteA) != Routes23.end());
   EXPECT_TRUE(Routes23.find(RouteB) != Routes23.end());

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes32;
   EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(103,102,Routes32));
   EXPECT_EQ(Routes32.size(),1);
   EXPECT_TRUE(Routes32.find(RouteC) != Routes32.end());

   std::unordered_set< std::shared_ptr<CBusSystem::SRoute> > Routes34;
   EXPECT_TRUE(BusSystemIndexer.RoutesByNodeIDs(103,104,Routes34));
   EXPECT_EQ(Routes34.size(),1);
   EXPECT_TRUE(Routes34.find(RouteD) != Routes34.end());

   EXPECT_TRUE(BusSystemIndexer.RouteBetweenNodeIDs(103,102));
   EXPECT_TRUE(BusSystemIndexer.RouteBetweenNodeIDs(102,103));
   EXPECT_TRUE(BusSystemIndexer.RouteBetweenNodeIDs(103,104));
   EXPECT_FALSE(BusSystemIndexer.RouteBetweenNodeIDs(101,104));
}


