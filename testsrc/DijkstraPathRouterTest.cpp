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

    std::vector<CPathRouter::TVertexID> Path;
    EXPECT_EQ(PathRouter.FindShortestPath(vertexA, vertexC, Path), 7.0);
    std::vector<CPathRouter::TVertexID> ExpectedPath{vertexA, vertexB, vertexD, vertexC};
    EXPECT_EQ(Path, ExpectedPath);
}
