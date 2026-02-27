#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "DataSink.h"
#include "XMLReader.h"
#include "XMLWriter.h"
#include "StringDataSink.h"
#include "StringDataSource.h"

namespace
{
    std::vector<SXMLEntity> ReadAllEntities(CXMLReader &reader, bool skipcdata = false)
    {
        std::vector<SXMLEntity> Entities;
        SXMLEntity Temp;
        while (reader.ReadEntity(Temp, skipcdata))
        {
            Entities.push_back(Temp);
        }
        return Entities;
    }

    class CFailingDataSink : public CDataSink
    {
    public:
        explicit CFailingDataSink(int fail_after) : DFailAfter(fail_after), DCalls(0)
        {
        }

        bool Put(const char &ch) noexcept override
        {
            if (DFailAfter >= 0 && DCalls >= DFailAfter)
            {
                return false;
            }
            DCalls++;
            DString.push_back(ch);
            return true;
        }

        bool Write(const std::vector<char> &buf) noexcept override
        {
            if (DFailAfter >= 0 && DCalls >= DFailAfter)
            {
                return false;
            }
            DCalls++;
            DString.append(buf.data(), buf.size());
            return true;
        }

        const std::string &String() const
        {
            return DString;
        }

    private:
        int DFailAfter;
        int DCalls;
        std::string DString;
    };
}

// just see if it can recognize individual tags
TEST(XMLReader, SimpleElements)
{
    std::string XMLString = "<tag></tag>";
    auto DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);

    auto Entities = ReadAllEntities(Reader);
    ASSERT_EQ(Entities.size(), 2u);
    EXPECT_EQ(Entities[0].DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entities[0].DNameData, "tag");
    EXPECT_EQ(Entities[1].DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entities[1].DNameData, "tag");
    EXPECT_TRUE(Reader.End());
}

// see if it can read attributes
TEST(XMLReader, Attributes)
{

    std::string XMLString = "<tag a=\"1\" b=\"two\"></tag>";
    // using auto because it's a kind of complex type
    auto DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);

    SXMLEntity Entity;
    ASSERT_TRUE(Reader.ReadEntity(Entity));
    EXPECT_EQ(Entity.DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entity.DNameData, "tag");
    EXPECT_EQ(Entity.DAttributes.size(), 2u);
    EXPECT_TRUE(Entity.AttributeExists("a"));
    EXPECT_TRUE(Entity.AttributeExists("b"));
    EXPECT_EQ(Entity.AttributeValue("a"), "1");
    EXPECT_EQ(Entity.AttributeValue("b"), "two");
}

// nested
TEST(XMLReader, NestedCharData)
{
    std::string XMLString = "<root><child>hi</child></root>";
    // auto bc complicated types
    auto DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);

    auto Entities = ReadAllEntities(Reader);
    ASSERT_EQ(Entities.size(), 5u);
    EXPECT_EQ(Entities[0].DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entities[0].DNameData, "root");
    EXPECT_EQ(Entities[1].DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entities[1].DNameData, "child");
    EXPECT_EQ(Entities[2].DType, SXMLEntity::EType::CharData);
    EXPECT_EQ(Entities[2].DNameData, "hi");
    EXPECT_EQ(Entities[3].DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entities[3].DNameData, "child");
    EXPECT_EQ(Entities[4].DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entities[4].DNameData, "root");
}

// see if it can skip the data inside of a nested child
TEST(XMLReader, SkipCharData)
{
    std::string XMLString = "<root><child>hi</child></root>";
    auto DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);

    auto Entities = ReadAllEntities(Reader, true);
    ASSERT_EQ(Entities.size(), 4u);
    EXPECT_EQ(Entities[0].DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entities[1].DType, SXMLEntity::EType::StartElement);
    EXPECT_EQ(Entities[2].DType, SXMLEntity::EType::EndElement);
    EXPECT_EQ(Entities[3].DType, SXMLEntity::EType::EndElement);
}

// ensure reader handles invalid XML without crashing
TEST(XMLReader, InvalidXML)
{
    std::string XMLString = "<root><child></root>";
    auto DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);

    SXMLEntity Entity;
    while (Reader.ReadEntity(Entity))
    {
    }

    EXPECT_TRUE(true); // reached here w/o crashing
}

TEST(XMLReader, EmptyInput)
{
    std::string XMLString = "";
    auto DataSource = std::make_shared<CStringDataSource>(XMLString);
    CXMLReader Reader(DataSource);

    SXMLEntity Entity;
    EXPECT_FALSE(Reader.ReadEntity(Entity));
    EXPECT_TRUE(Reader.End());
}

// test writing out elements to check that it can open/close a tag
TEST(XMLWriter, SimpleElements)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity Start;
    Start.DType = SXMLEntity::EType::StartElement;
    Start.DNameData = "tag";

    SXMLEntity End;
    End.DType = SXMLEntity::EType::EndElement;
    End.DNameData = "tag";

    EXPECT_TRUE(Writer.WriteEntity(Start));
    EXPECT_TRUE(Writer.WriteEntity(End));
    EXPECT_EQ(DataSink->String(), "<tag></tag>");
}

// see if it can add in attributes without bugs
TEST(XMLWriter, CompleteElement)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity Complete;
    Complete.DType = SXMLEntity::EType::CompleteElement;
    Complete.DNameData = "tag";
    Complete.SetAttribute("a", "1");

    EXPECT_TRUE(Writer.WriteEntity(Complete));
    EXPECT_EQ(DataSink->String(), "<tag a=\"1\"/>");
}

// writing with escape cases
TEST(XMLWriter, Escaping)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity Start;
    Start.DType = SXMLEntity::EType::StartElement;
    Start.DNameData = "tag";
    Start.SetAttribute("a", "1&2\"3\'<>");

    SXMLEntity Text;
    Text.DType = SXMLEntity::EType::CharData;
    Text.DNameData = "x<y & z>!";

    SXMLEntity End;
    End.DType = SXMLEntity::EType::EndElement;
    End.DNameData = "tag";

    EXPECT_TRUE(Writer.WriteEntity(Start));
    EXPECT_TRUE(Writer.WriteEntity(Text));
    EXPECT_TRUE(Writer.WriteEntity(End));
    EXPECT_EQ(DataSink->String(), "<tag a=\"1&amp;2&quot;3&apos;&lt;&gt;\">x&lt;y &amp; z&gt;!</tag>");
}

TEST(XMLWriter, StartTagFailurePaths)
{
    for (int FailAfter = 0; FailAfter <= 7; FailAfter++)
    {
        auto DataSink = std::make_shared<CFailingDataSink>(FailAfter);
        CXMLWriter Writer(DataSink);

        SXMLEntity Start;
        Start.DType = SXMLEntity::EType::StartElement;
        Start.DNameData = "tag";
        Start.SetAttribute("a", "b");

        EXPECT_FALSE(Writer.WriteEntity(Start));
    }
}

TEST(XMLWriter, CompleteTagFailurePaths)
{
    for (int FailAfter = 0; FailAfter <= 7; FailAfter++)
    {
        auto DataSink = std::make_shared<CFailingDataSink>(FailAfter);
        CXMLWriter Writer(DataSink);

        SXMLEntity Complete;
        Complete.DType = SXMLEntity::EType::CompleteElement;
        Complete.DNameData = "tag";
        Complete.SetAttribute("a", "b");

        EXPECT_FALSE(Writer.WriteEntity(Complete));
    }
}

TEST(XMLWriter, EndTagFailurePaths)
{
    for (int FailAfter = 3; FailAfter <= 5; FailAfter++)
    {
        auto DataSink = std::make_shared<CFailingDataSink>(FailAfter);
        CXMLWriter Writer(DataSink);

        SXMLEntity Start;
        Start.DType = SXMLEntity::EType::StartElement;
        Start.DNameData = "tag";

        SXMLEntity End;
        End.DType = SXMLEntity::EType::EndElement;
        End.DNameData = "tag";

        ASSERT_TRUE(Writer.WriteEntity(Start));
        EXPECT_FALSE(Writer.WriteEntity(End));
    }
}

TEST(XMLWriter, EndElementWithoutStart)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity End;
    End.DType = SXMLEntity::EType::EndElement;
    End.DNameData = "tag";

    EXPECT_FALSE(Writer.WriteEntity(End));
}

TEST(XMLWriter, EndElementUsesStackWhenNameEmpty)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity Start;
    Start.DType = SXMLEntity::EType::StartElement;
    Start.DNameData = "root";

    SXMLEntity End;
    End.DType = SXMLEntity::EType::EndElement;
    End.DNameData = "";

    EXPECT_TRUE(Writer.WriteEntity(Start));
    EXPECT_TRUE(Writer.WriteEntity(End));
    EXPECT_EQ(DataSink->String(), "<root></root>");
}

TEST(XMLWriter, FlushClosesOpenElements)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity Root;
    Root.DType = SXMLEntity::EType::StartElement;
    Root.DNameData = "root";

    SXMLEntity Child;
    Child.DType = SXMLEntity::EType::StartElement;
    Child.DNameData = "child";

    EXPECT_TRUE(Writer.WriteEntity(Root));
    EXPECT_TRUE(Writer.WriteEntity(Child));
    EXPECT_TRUE(Writer.Flush());
    EXPECT_EQ(DataSink->String(), "<root><child></child></root>");
}

TEST(XMLWriter, UnknownType)
{
    auto DataSink = std::make_shared<CStringDataSink>();
    CXMLWriter Writer(DataSink);

    SXMLEntity Unknown;
    Unknown.DType = static_cast<SXMLEntity::EType>(99);

    EXPECT_FALSE(Writer.WriteEntity(Unknown));
}

TEST(XMLWriter, FailingSinkStartTag)
{
    auto DataSink = std::make_shared<CFailingDataSink>(1);
    CXMLWriter Writer(DataSink);

    SXMLEntity Start;
    Start.DType = SXMLEntity::EType::StartElement;
    Start.DNameData = "tag";

    EXPECT_FALSE(Writer.WriteEntity(Start));
}

TEST(XMLWriter, FailingSinkCompleteTag)
{
    auto DataSink = std::make_shared<CFailingDataSink>(1);
    CXMLWriter Writer(DataSink);

    SXMLEntity Complete;
    Complete.DType = SXMLEntity::EType::CompleteElement;
    Complete.DNameData = "tag";

    EXPECT_FALSE(Writer.WriteEntity(Complete));
}

TEST(XMLWriter, FailingSinkEndTag)
{
    auto DataSink = std::make_shared<CFailingDataSink>(3);
    CXMLWriter Writer(DataSink);

    SXMLEntity Start;
    Start.DType = SXMLEntity::EType::StartElement;
    Start.DNameData = "tag";

    SXMLEntity End;
    End.DType = SXMLEntity::EType::EndElement;
    End.DNameData = "tag";

    EXPECT_TRUE(Writer.WriteEntity(Start));
    EXPECT_FALSE(Writer.WriteEntity(End));
}

TEST(XMLEntity, AttributeHelpers)
{
    SXMLEntity Entity;

    EXPECT_FALSE(Entity.AttributeExists("missing"));
    EXPECT_EQ(Entity.AttributeValue("missing"), "");

    EXPECT_FALSE(Entity.SetAttribute("", "nope"));

    EXPECT_TRUE(Entity.SetAttribute("id", "1"));
    EXPECT_TRUE(Entity.AttributeExists("id"));
    EXPECT_EQ(Entity.AttributeValue("id"), "1");

    EXPECT_TRUE(Entity.SetAttribute("id", "2"));
    EXPECT_EQ(Entity.AttributeValue("id"), "2");
}
