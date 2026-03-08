#include <gtest/gtest.h>
#include "DijkstraPathRouter.h"

TEST(DijkstraPathRouter, SimpleTest){
    CDijkstraPathRouter PathRouter;

    EXPECT_EQ(PathRouter.VertexCount(), 0);
    auto VertexID = PathRouter.AddVertex(std::string("foo"));
    EXPECT_EQ(PathRouter.VertexCount(), 1);
    EXPECT_EQ(std::any_cast<std::string>(PathRouter.GetVertexTag(VertexID)), "foo");

    auto VertexID2 = PathRouter.AddVertex(22);
    EXPECT_EQ(PathRouter.VertexCount(), 2);
    EXPECT_EQ(std::any_cast<int>(PathRouter.GetVertexTag(VertexID2)), 22);

    auto invalidVert = PathRouter.AddVertex(std::string("A"));
    auto invalid_get_tag = PathRouter.GetVertexTag(invalidVert + 1);
    EXPECT_FALSE(invalid_get_tag.has_value());
}

TEST(DijkstraPathRouter, ShortestPath){
    CDijkstraPathRouter PathRouter;
    /*
        A ---> B ---> C 
               |   _ / 
               v /
               D

        A->B : 4
        B->C : 5
        B->D: 2
        D->C: 1
    
    */

    auto vertexA = PathRouter.AddVertex(std::string("A"));
    auto vertexB = PathRouter.AddVertex(std::string("B"));
    auto vertexC = PathRouter.AddVertex(std::string("C"));
    auto vertexD = PathRouter.AddVertex(std::string("D"));
    EXPECT_EQ(PathRouter.VertexCount(), 4);
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexB, 4.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexB, vertexC, 5.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexB, vertexD, 2.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexD, vertexC, 1.0));
    // try to add invalid edge
    EXPECT_FALSE(PathRouter.AddEdge(vertexA, 5, 3.0));

    std::vector<CPathRouter::TVertexID> Regular_Path;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexC, Regular_Path), 7.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath1{vertexA, vertexB, vertexD, vertexC};
    EXPECT_EQ(Regular_Path, ExpectedPath1);

    std::vector<CPathRouter::TVertexID> Regular_Path2;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexB, vertexC, Regular_Path2), 3.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath2{vertexB, vertexD, vertexC};
    EXPECT_EQ(Regular_Path2, ExpectedPath2);

    std::vector<CPathRouter::TVertexID> invalid_Path3;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexB, vertexA, invalid_Path3), CPathRouter::NoPathExists);
}

TEST(DijkstraPathRouter, MyCustomGraphTest){
    CDijkstraPathRouter PathRouter;

    auto vertexA = PathRouter.AddVertex(std::string("A"));
    auto vertexB = PathRouter.AddVertex(std::string("B"));
    auto vertexC = PathRouter.AddVertex(std::string("C"));
    auto vertexD = PathRouter.AddVertex(std::string("D"));
    auto vertexE = PathRouter.AddVertex(std::string("E"));
    auto vertexF = PathRouter.AddVertex(std::string("F"));
    auto vertexG = PathRouter.AddVertex(std::string("G"));

    EXPECT_EQ(PathRouter.VertexCount(), 7);
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexB, 2.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexC, 9.0));

    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexB, 2.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexF, 6.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexD, 3.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexE, 3.0));

    EXPECT_TRUE(PathRouter.AddEdge(vertexE, vertexG, 1.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexE, vertexA, 10.0));

    EXPECT_TRUE(PathRouter.AddEdge(vertexF, vertexB, 1.0));
    EXPECT_TRUE(PathRouter.AddEdge(vertexF, vertexD, 4.0));

    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexF, 6.0));

    EXPECT_TRUE(PathRouter.AddEdge(vertexG, vertexC, 3.0));

    std::vector<CPathRouter::TVertexID> RegularPath1;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexG, RegularPath1), 13.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath1{vertexA, vertexC, vertexE, vertexG};
    EXPECT_EQ(RegularPath1, ExpectedPath1);

    std::vector<CPathRouter::TVertexID> RegularPath2;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexG, vertexA, RegularPath2), 16.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath2{vertexG, vertexC, vertexE, vertexA};
    EXPECT_EQ(RegularPath2, ExpectedPath2);
}

TEST(DijkstraPathRouter, BiredictionalEdges){
    CDijkstraPathRouter PathRouter;

    auto vertexA = PathRouter.AddVertex(std::string("A"));
    auto vertexB = PathRouter.AddVertex(std::string("B"));
    auto vertexC = PathRouter.AddVertex(std::string("C"));
    auto vertexD = PathRouter.AddVertex(std::string("D"));
    auto vertexE = PathRouter.AddVertex(std::string("E"));
    auto vertexF = PathRouter.AddVertex(std::string("F"));
    auto vertexG = PathRouter.AddVertex(std::string("G"));

    EXPECT_EQ(PathRouter.VertexCount(), 7);
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexB, 1.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexG, 10.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexD, 3.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexA, vertexC, 2.0, true));

    EXPECT_TRUE(PathRouter.AddEdge(vertexB, vertexE, 7.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexB, vertexC, 4.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexB, vertexD, 2.0, true));

    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexE, 2.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexC, vertexD, 3.0, true));

    EXPECT_TRUE(PathRouter.AddEdge(vertexE, vertexG, 8.0, true));
    EXPECT_TRUE(PathRouter.AddEdge(vertexE, vertexF, 3.0, true));

    EXPECT_TRUE(PathRouter.AddEdge(vertexF, vertexG, 1.0, true));

    EXPECT_TRUE(PathRouter.AddEdge(vertexG, vertexD, 6.0, true));

    std::vector<CPathRouter::TVertexID> RegularPath1;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexG, RegularPath1), 8.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath1{vertexA, vertexC, vertexE, vertexF, vertexG};
    EXPECT_EQ(RegularPath1, ExpectedPath1);

    std::vector<CPathRouter::TVertexID> RegularPath2;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexF, RegularPath2), 7.0);
    std::vector<CPathRouter::TVertexID> Exp2{vertexA, vertexC, vertexE, vertexF};
    EXPECT_EQ(RegularPath2, Exp2);

    std::vector<CPathRouter::TVertexID> RegularPath3;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexB, vertexG, RegularPath3), 8.0);
    std::vector<CPathRouter::TVertexID> Exp3{vertexB, vertexD, vertexG};
    EXPECT_EQ(RegularPath3, Exp3);

    std::vector<CPathRouter::TVertexID> RegularPath4;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexE, RegularPath4), 4.0);
    std::vector<CPathRouter::TVertexID> Exp4{vertexA, vertexC, vertexE};
    EXPECT_EQ(RegularPath4, Exp4);

    std::vector<CPathRouter::TVertexID> RegularPath5;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexB, RegularPath5), 1.0);
    std::vector<CPathRouter::TVertexID> Exp5{vertexA, vertexB};
    EXPECT_EQ(RegularPath5, Exp5);

    std::vector<CPathRouter::TVertexID> RegularPath6;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexC, RegularPath6), 2.0);
    std::vector<CPathRouter::TVertexID> Exp6{vertexA, vertexC};
    EXPECT_EQ(RegularPath6, Exp6);
}