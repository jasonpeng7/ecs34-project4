#include <gtest/gtest.h>
#include "FileDataFactory.h"
#include "FileDataSink.h"
#include "FileDataSource.h"
#include <cstdio>
#include <filesystem>

// Assume being run from Makefile so testtmp is subdirectory

const std::string BaseDirectory = "./testtmp/";

TEST(FileDataSourceSink, EmptyTest){
    CFileDataFactory DataFactory(BaseDirectory);
    std::string Filename = "empty.txt";
    std::remove((BaseDirectory + Filename).c_str());
    {
        auto Sink = DataFactory.CreateSink(Filename);
    }
    auto Source = DataFactory.CreateSource(Filename);
    EXPECT_TRUE(Source->End());
}

TEST(FileDataSourceSink, PutTest){
    CFileDataFactory DataFactory(BaseDirectory);
    std::string Filename = "put.txt";
    std::remove((BaseDirectory + Filename).c_str());
    {
        auto Sink = DataFactory.CreateSink(Filename);
        for(char Ch = ' '; Ch < '~'; Ch++){
            EXPECT_TRUE(Sink->Put(Ch));
        }
    }
    auto Source = DataFactory.CreateSource(Filename);
    for(char Ch = ' '; Ch < '~'; Ch++){
        char TempCh;
        EXPECT_TRUE(Source->Get(TempCh));
        EXPECT_EQ(Ch,TempCh);
    }
    EXPECT_TRUE(Source->End());
}

TEST(FileDataSourceSink, WriteTest){
    CFileDataFactory DataFactory(BaseDirectory);
    std::string Filename = "write.txt";
    std::remove((BaseDirectory + Filename).c_str());
    std::vector<char> OutBuffer, InBuffer;
    for(char Ch = ' '; Ch < '~'; Ch++){
        OutBuffer.push_back(Ch);
    }
    {
        auto Sink = DataFactory.CreateSink(Filename);
        EXPECT_TRUE(Sink->Write(OutBuffer));
    }
    auto Source = DataFactory.CreateSource(Filename);
    EXPECT_TRUE(Source->Read(InBuffer,OutBuffer.size()));
    EXPECT_EQ(InBuffer,OutBuffer);
    EXPECT_TRUE(Source->End());
}

TEST(FileDataSourceSink, PeekTest){
    CFileDataFactory DataFactory(BaseDirectory);
    std::string Filename = "peek.txt";
    std::remove((BaseDirectory + Filename).c_str());
    {
        auto Sink = DataFactory.CreateSink(Filename);
        EXPECT_TRUE(Sink->Put('A'));
        EXPECT_TRUE(Sink->Put('B'));
    }

    auto Source = DataFactory.CreateSource(Filename);
    char TempCh = '\0';

    EXPECT_TRUE(Source->Peek(TempCh));
    EXPECT_EQ(TempCh, 'A');
    EXPECT_TRUE(Source->Get(TempCh));
    EXPECT_EQ(TempCh, 'A');
    EXPECT_TRUE(Source->Peek(TempCh));
    EXPECT_EQ(TempCh, 'B');
    EXPECT_TRUE(Source->Get(TempCh));
    EXPECT_EQ(TempCh, 'B');
    EXPECT_TRUE(Source->End());
}

TEST(FileDataSourceSink, EmptySourceFailureStateTest){
    CFileDataFactory DataFactory(BaseDirectory);
    std::string Filename = "empty_failure.txt";
    std::remove((BaseDirectory + Filename).c_str());
    {
        auto Sink = DataFactory.CreateSink(Filename);
    }

    auto Source = DataFactory.CreateSource(Filename);
    char TempCh = '\0';
    std::vector<char> Buffer = {'x'};

    EXPECT_TRUE(Source->End());
    EXPECT_FALSE(Source->Get(TempCh));
    EXPECT_FALSE(Source->Peek(TempCh));
    EXPECT_FALSE(Source->Read(Buffer, 4));
    EXPECT_EQ(Buffer, std::vector<char>({'x'}));
}

TEST(FileDataSourceSink, MissingSourceFailureStateTest){
    CFileDataFactory DataFactory(BaseDirectory);
    std::string Filename = "missing.txt";
    std::remove((BaseDirectory + Filename).c_str());

    auto Source = DataFactory.CreateSource(Filename);
    char TempCh = '\0';
    std::vector<char> Buffer = {'x'};

    EXPECT_FALSE(Source->Get(TempCh));
    EXPECT_FALSE(Source->Peek(TempCh));
    EXPECT_FALSE(Source->Read(Buffer, 2));
    EXPECT_EQ(Buffer, std::vector<char>({'x'}));
}

TEST(FileDataSourceSink, EmptyBasePathDefaultsToCurrentDirectory){
    CFileDataFactory DataFactory("");
    std::string Filename = "filedatafactory_empty_base.txt";
    std::remove(Filename.c_str());

    {
        auto Sink = DataFactory.CreateSink(Filename);
        ASSERT_NE(Sink, nullptr);
        EXPECT_TRUE(Sink->Put('Z'));
    }

    auto Source = DataFactory.CreateSource(Filename);
    char TempCh = '\0';
    EXPECT_TRUE(Source->Get(TempCh));
    EXPECT_EQ(TempCh, 'Z');
    EXPECT_TRUE(Source->End());

    std::remove(Filename.c_str());
}

TEST(FileDataSourceSink, BasePathWithoutTrailingSlashIsNormalized){
    std::string LocalBaseDirectory = "./testtmp/factory_normalized";
    std::filesystem::remove_all(LocalBaseDirectory);

    CFileDataFactory DataFactory(LocalBaseDirectory);
    std::string Filename = "normalized.txt";

    {
        auto Sink = DataFactory.CreateSink(Filename);
        ASSERT_NE(Sink, nullptr);
        EXPECT_TRUE(Sink->Write(std::vector<char>({'O', 'K'})));
    }

    EXPECT_TRUE(std::filesystem::exists(LocalBaseDirectory + "/" + Filename));

    auto Source = DataFactory.CreateSource(Filename);
    std::vector<char> Buffer;
    EXPECT_TRUE(Source->Read(Buffer, 2));
    EXPECT_EQ(Buffer, std::vector<char>({'O', 'K'}));
    EXPECT_TRUE(Source->End());

    std::filesystem::remove_all(LocalBaseDirectory);
}

TEST(FileDataSourceSink, CreateSinkFailureReturnsNullptr){
    std::string BlockingPath = BaseDirectory + "factory_blocker";
    std::filesystem::remove_all(BlockingPath);
    std::remove(BlockingPath.c_str());

    {
        FILE *BlockerFile = std::fopen(BlockingPath.c_str(), "w");
        ASSERT_NE(BlockerFile, nullptr);
        std::fputs("block", BlockerFile);
        std::fclose(BlockerFile);
    }

    CFileDataFactory DataFactory(BlockingPath);
    EXPECT_EQ(DataFactory.CreateSink("blocked.txt"), nullptr);

    std::remove(BlockingPath.c_str());
}
