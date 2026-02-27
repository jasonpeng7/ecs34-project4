#include <gtest/gtest.h>
#include "CSVBusSystem.h"
#include "StringDataSource.h"
#include "DSVReader.h"

TEST(CSVBusSystem, SimpleFiles){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,123\n"
                                                                "2,124");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource, ',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1\n"
                                                                "A,2");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource, ',');
    
    CCSVBusSystem BusSystem(StopReader,RouteReader);
    
    EXPECT_EQ(BusSystem.StopCount(),2);
    auto StopObj = BusSystem.StopByIndex(0); // this is position by index in our StopByIndex vector
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(1);
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByID(1); // stop with matching ID, this is not position
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),1);
    EXPECT_EQ(StopObj->NodeID(),123);
    StopObj = BusSystem.StopByID(2);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),2);
    EXPECT_EQ(StopObj->NodeID(),124);

    EXPECT_EQ(BusSystem.RouteCount(), 1); 
    auto RouteObj = BusSystem.RouteByIndex(0);
    EXPECT_NE(RouteObj, nullptr);
    RouteObj = BusSystem.RouteByName("A"); // our route "A" has two stop id's
    ASSERT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->Name(), "A"); 
    EXPECT_EQ(RouteObj->StopCount(), 2); // expect two stops
    EXPECT_EQ(RouteObj->GetStopID(0), 1); // expect first one to be 1
    EXPECT_EQ(RouteObj->GetStopID(1), 2); // expect second stop to be 2
}

TEST(CSVBusSystem, InvalidAccessTest){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,123\n"
                                                                "2,124");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource, ',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1\n"
                                                                "A,2");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource, ',');
    
    CCSVBusSystem BusSystem(StopReader,RouteReader);
    
    EXPECT_EQ(BusSystem.StopCount(),2);
    auto StopObj = BusSystem.StopByIndex(0); // this is position by index in our StopByIndex vector
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(5);
    EXPECT_EQ(StopObj,nullptr);
    StopObj = BusSystem.StopByID(10);
    EXPECT_EQ(StopObj, nullptr);

    EXPECT_EQ(BusSystem.RouteCount(), 1); 
    auto RouteObj = BusSystem.RouteByIndex(0);
    EXPECT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->Name(), "A"); 
    EXPECT_EQ(RouteObj->StopCount(), 2); 
    EXPECT_EQ(RouteObj->GetStopID(0), 1); 
    EXPECT_EQ(RouteObj->GetStopID(1), 2); 
    //try to access index that is invalid
    EXPECT_EQ(RouteObj->GetStopID(2), std::numeric_limits<CBusSystem::TStopID>::max());

    RouteObj = BusSystem.RouteByIndex(2);
    ASSERT_EQ(RouteObj, nullptr);

    RouteObj = BusSystem.RouteByName("B");
    ASSERT_EQ(RouteObj, nullptr);
}

TEST(CSVBusSystem, MultipleRouteAndStopsFile){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,123\n"
                                                                "2,124\n"
                                                                "22003,95717604\n"
                                                                "22002,95717425\n"
                                                                "22005,2584424865\n"
                                                                "22076,9571742512\n"
                                                                "22033,21873881\n"
                                                                "22075,74892373\n"
                                                                "22073,432424344"
                                                                );
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource, ',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1\n"
                                                                "A,2\n"
                                                                "B,22033\n"
                                                                "B,22076\n"
                                                                "B,22075\n"
                                                                "C,22073\n"
                                                                "D,22073");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource, ',');
    
    CCSVBusSystem BusSystem(StopReader,RouteReader);
    
    EXPECT_EQ(BusSystem.StopCount(),9);
    // check that each object exists
    auto StopObj = BusSystem.StopByIndex(0); // this is position by index in our StopByIndex vector
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(1);
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(2);
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(3);
    EXPECT_NE(StopObj,nullptr);
    StopObj = BusSystem.StopByIndex(4);
    EXPECT_NE(StopObj,nullptr);
    
    // check each stop grabs correct id and has correct associated node_id
    StopObj = BusSystem.StopByID(1); 
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),1);
    EXPECT_EQ(StopObj->NodeID(),123);
    StopObj = BusSystem.StopByID(2);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),2);
    EXPECT_EQ(StopObj->NodeID(),124);
    StopObj = BusSystem.StopByID(22003);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22003);
    EXPECT_EQ(StopObj->NodeID(),95717604);
    StopObj = BusSystem.StopByID(22005);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22005);
    EXPECT_EQ(StopObj->NodeID(),2584424865);
    StopObj = BusSystem.StopByID(22002);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22002);
    EXPECT_EQ(StopObj->NodeID(),95717425);
    StopObj = BusSystem.StopByID(22005);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22005);
    EXPECT_EQ(StopObj->NodeID(),2584424865);
    StopObj = BusSystem.StopByID(22076);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22076);
    EXPECT_EQ(StopObj->NodeID(),9571742512);
    StopObj = BusSystem.StopByID(22033);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22033);
    EXPECT_EQ(StopObj->NodeID(),21873881);
    StopObj = BusSystem.StopByID(22073);
    ASSERT_NE(StopObj,nullptr);
    EXPECT_EQ(StopObj->ID(),22073);
    EXPECT_EQ(StopObj->NodeID(),432424344);


    EXPECT_EQ(BusSystem.RouteCount(), 4); // A, B, C, D
    auto RouteObj = BusSystem.RouteByIndex(0);
    EXPECT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->StopCount(), 2); // A should have 2
    RouteObj = BusSystem.RouteByIndex(1); 
    EXPECT_EQ(RouteObj->StopCount(), 3); // B should have 3
    RouteObj = BusSystem.RouteByIndex(2); 
    EXPECT_EQ(RouteObj->StopCount(), 1); // C should have 1
    RouteObj = BusSystem.RouteByIndex(3); 
    EXPECT_EQ(RouteObj->StopCount(), 1); // D should have 1

    RouteObj = BusSystem.RouteByName("A"); // our route "A" has two stop id's
    ASSERT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->Name(), "A"); 
    EXPECT_EQ(RouteObj->StopCount(), 2); // expect two stops
    EXPECT_EQ(RouteObj->GetStopID(0), 1); // expect first one to be 1
    EXPECT_EQ(RouteObj->GetStopID(1), 2); // expect second stop to be 2

    RouteObj = BusSystem.RouteByName("B"); // our route "B" has three stop id's
    ASSERT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->Name(), "B"); 
    EXPECT_EQ(RouteObj->StopCount(), 3); // expect 3 stops
    EXPECT_EQ(RouteObj->GetStopID(0), 22033); 
    EXPECT_EQ(RouteObj->GetStopID(1), 22076); 
    EXPECT_EQ(RouteObj->GetStopID(2), 22075); 

    RouteObj = BusSystem.RouteByName("C"); // our route "C" has three one id's
    ASSERT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->Name(), "C"); 
    EXPECT_EQ(RouteObj->StopCount(), 1); // expect 1 stops
    EXPECT_EQ(RouteObj->GetStopID(0), 22073); 

    RouteObj = BusSystem.RouteByName("D"); // our route "D" has one stop id's
    ASSERT_NE(RouteObj, nullptr);
    EXPECT_EQ(RouteObj->Name(), "D"); 
    EXPECT_EQ(RouteObj->StopCount(), 1); // expect 1 stops
    EXPECT_EQ(RouteObj->GetStopID(0), 22073); 
}

TEST(CSVBusSystem, InvalidHeader){
    auto StopDataSource = std::make_shared< CStringDataSource >("STOP_ID,NODEID\n"
                                                                "1,123\n"
                                                                "2,124");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource, ',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("ROUTES,STOP_ID\n"
                                                                "A,1\n"
                                                                "A,2");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource, ',');
    
    CCSVBusSystem BusSystem(StopReader,RouteReader);
    
    EXPECT_EQ(BusSystem.StopCount(),0);
    auto StopObj = BusSystem.StopByIndex(0);
    EXPECT_EQ(StopObj, nullptr);
    StopObj = BusSystem.StopByIndex(2);
    EXPECT_EQ(StopObj, nullptr);
    StopObj = BusSystem.StopByID(1);
    EXPECT_EQ(StopObj, nullptr);
    StopObj = BusSystem.StopByIndex(2);
    EXPECT_EQ(StopObj, nullptr);

    EXPECT_EQ(BusSystem.RouteCount(), 0);
    auto RouteObj = BusSystem.RouteByIndex(0);
    EXPECT_EQ(RouteObj, nullptr);
    RouteObj = BusSystem.RouteByIndex(2);
    EXPECT_EQ(RouteObj, nullptr);
    RouteObj = BusSystem.RouteByName("A");
    ASSERT_EQ(RouteObj, nullptr);
}

TEST(CSVBusSystem, MissingHeader){
    auto StopDataSource = std::make_shared< CStringDataSource >("1,123\n"
                                                                "2,124");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource, ',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("A,1\n"
                                                                "A,2");
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource, ',');
    
    CCSVBusSystem BusSystem(StopReader,RouteReader);
    
    EXPECT_EQ(BusSystem.StopCount(),0);
    auto StopObj = BusSystem.StopByIndex(0);
    EXPECT_EQ(StopObj, nullptr);
    StopObj = BusSystem.StopByIndex(2);
    EXPECT_EQ(StopObj, nullptr);
    StopObj = BusSystem.StopByID(1);
    EXPECT_EQ(StopObj, nullptr);
    StopObj = BusSystem.StopByIndex(2);
    EXPECT_EQ(StopObj, nullptr);

    EXPECT_EQ(BusSystem.RouteCount(), 0);
    auto RouteObj = BusSystem.RouteByIndex(0);
    EXPECT_EQ(RouteObj, nullptr);
    RouteObj = BusSystem.RouteByIndex(2);
    EXPECT_EQ(RouteObj, nullptr);
    RouteObj = BusSystem.RouteByName("A");
    ASSERT_EQ(RouteObj, nullptr);
}

TEST(CSVBusSystem, DuplicateRouteStopID){
    auto StopDataSource = std::make_shared< CStringDataSource >("stop_id,node_id\n"
                                                                "1,123\n"
                                                                "2,124");
    auto StopReader = std::make_shared< CDSVReader >(StopDataSource, ',');
    auto RouteDataSource = std::make_shared< CStringDataSource >("route,stop_id\n"
                                                                "A,1\n"
                                                                "A,1\n"
                                                                "A,1\n"
                                                                "A,1\n"
                                                                "A,1\n"
                                                                "A,1\n"
                                                                "B,2\n"
                                                                "C,3\n"
                                                                );
    auto RouteReader = std::make_shared< CDSVReader >(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 2);
    EXPECT_EQ(BusSystem.RouteCount(), 1);
    auto RouteObj = BusSystem.RouteByName("A");
    EXPECT_EQ(RouteObj->StopCount(), 1); 
}

TEST(CSVBusSystem, MissingColumns){
    auto StopDataSource = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                              ",322");
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              ",2");
    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 0);
    EXPECT_EQ(BusSystem.RouteCount(), 0);

    auto StopDataSource2 = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                              "1,");
    auto StopReader2 = std::make_shared<CDSVReader>(StopDataSource2, ',');

    auto RouteDataSource2 = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              "A,");
    auto RouteReader2 = std::make_shared<CDSVReader>(RouteDataSource2, ',');

    CCSVBusSystem BusSystem2(StopReader2, RouteReader2);

    EXPECT_EQ(BusSystem2.StopCount(), 0);
    EXPECT_EQ(BusSystem2.RouteCount(), 0);
}

TEST(CSVBusSystem, EmptyStopSystem){
    auto StopDataSource = std::make_shared<CStringDataSource>("\n"
                                                              "");
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              "A,2");
    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 0);
    EXPECT_EQ(BusSystem.RouteCount(), 0);
}


// adjust for duplicate stops
TEST(CSVBusSystem, NonexistentStopID){
    auto StopDataSource = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                              "2,4234732889\n"
                                                              "4,983019443"
                                                              );
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              "A,2\n"
                                                              "B,3"
                                                              );
    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 2);
    EXPECT_EQ(BusSystem.RouteCount(), 1);

    auto StopObj = BusSystem.StopByID(2);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 4234732889);
    StopObj = BusSystem.StopByID(4);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 983019443);
    StopObj = BusSystem.StopByIndex(0);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->ID(), 2);
    StopObj = BusSystem.StopByIndex(1);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->ID(), 4);

    auto RouteObj = BusSystem.RouteByName("A");
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->Name(), "A");
    EXPECT_EQ(RouteObj->GetStopID(0), 2);

    // stop at this route doesnt exist so it should not contain route at all 
    RouteObj = BusSystem.RouteByName("B");
    ASSERT_EQ(nullptr, RouteObj);
}

TEST(CSVBusSystem, DuplicateStopID){
    auto StopDataSource = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                              "1,318974983\n"
                                                              "2,34324980\n"
                                                              "3,4328749\n"
                                                              "3,432442432324\n"
                                                              "4,908432423\n"
                                                              "6,3849208409" 
                                                              );
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                              "A,1\n"
                                                              "B,2\n"
                                                              "C,3\n"
                                                              "D,4"
                                                              );
    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 3);
    EXPECT_EQ(BusSystem.RouteCount(), 3);

    auto StopObj = BusSystem.StopByID(1);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 318974983);
    EXPECT_EQ(StopObj->ID(), 1);
    StopObj = BusSystem.StopByIndex(0);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 318974983);
    EXPECT_EQ(StopObj->ID(), 1);
    StopObj = BusSystem.StopByID(2);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 34324980);
    EXPECT_EQ(StopObj->ID(), 2);
    StopObj = BusSystem.StopByIndex(1);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 34324980);
    EXPECT_EQ(StopObj->ID(), 2);
    StopObj = BusSystem.StopByID(3);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 4328749);
    EXPECT_EQ(StopObj->ID(), 3);
    StopObj = BusSystem.StopByIndex(2);
    ASSERT_NE(nullptr, StopObj);
    EXPECT_EQ(StopObj->NodeID(), 4328749);
    EXPECT_EQ(StopObj->ID(), 3);
    StopObj = BusSystem.StopByID(4);
    ASSERT_EQ(nullptr, StopObj);

    auto RouteObj = BusSystem.RouteByName("A");
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->GetStopID(0),1);
    EXPECT_EQ(RouteObj->GetStopID(1), std::numeric_limits<CBusSystem::TStopID>::max());
    RouteObj = BusSystem.RouteByIndex(0);
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->GetStopID(0),1);
    EXPECT_EQ(RouteObj->Name(), "A");
    RouteObj = BusSystem.RouteByName("B");
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->GetStopID(0),2);
    EXPECT_EQ(RouteObj->GetStopID(1), std::numeric_limits<CBusSystem::TStopID>::max());
    RouteObj = BusSystem.RouteByIndex(1);
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->GetStopID(0),2);
    EXPECT_EQ(RouteObj->Name(), "B");
    RouteObj = BusSystem.RouteByName("C");
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->GetStopID(0),3);
    EXPECT_EQ(RouteObj->GetStopID(1), std::numeric_limits<CBusSystem::TStopID>::max());
    RouteObj = BusSystem.RouteByIndex(2);
    ASSERT_NE(nullptr, RouteObj);
    EXPECT_EQ(RouteObj->GetStopID(0),3);
    EXPECT_EQ(RouteObj->Name(), "C");

    RouteObj = BusSystem.RouteByName("D");
    ASSERT_EQ(nullptr, RouteObj);
}

TEST(CSVBusSystem, DuplicateStopIDStopsParsing){
    auto StopDataSource = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                               "1,100\n"
                                                               "2,200\n"
                                                               "2,999\n"
                                                               "3,300"
    );
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id");

    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 2);
    EXPECT_EQ(nullptr, BusSystem.StopByID(3));
}

TEST(CSVBusSystem, NonvalidStopID){
    auto StopDataSource = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                               "abc,100\n"
                                                               "2,200"
    );
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                               "L,abc"
    );

    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 0);
    EXPECT_EQ(BusSystem.RouteCount(), 0);
    ASSERT_EQ(nullptr, BusSystem.StopByID(2));

    ASSERT_EQ(nullptr, BusSystem.RouteByName("L"));
}

TEST(CSVBusSystem, NonvalidNodeID){
    auto StopDataSource = std::make_shared<CStringDataSource>("stop_id,node_id\n"
                                                               "1,jason\n"
                                                               "2,200\n"
    );
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>("route,stop_id\n"
                                                               "A,1"
    );

    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.StopCount(), 0);
    EXPECT_EQ(BusSystem.RouteCount(), 0);
}

TEST(CSVBusSystem, EmptyRouteAndStopFile) {
    auto StopDataSource = std::make_shared<CStringDataSource>("");
    auto StopReader = std::make_shared<CDSVReader>(StopDataSource, ',');

    auto RouteDataSource = std::make_shared<CStringDataSource>(""); 
    auto RouteReader = std::make_shared<CDSVReader>(RouteDataSource, ',');

    CCSVBusSystem BusSystem(StopReader, RouteReader);

    EXPECT_EQ(BusSystem.RouteCount(), 0);
}