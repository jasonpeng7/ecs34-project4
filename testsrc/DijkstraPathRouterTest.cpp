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

}