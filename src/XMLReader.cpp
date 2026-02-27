#include "XMLReader.h"

#include <expat.h>

#include <deque>
#include <vector>

namespace
{
    constexpr std::size_t kReadBufferSize = 4096;
}

struct CXMLReader::SImplementation
{
    std::shared_ptr<CDataSource> DSource;
    XML_Parser DParser;
    std::deque<SXMLEntity> DQueue;
    bool DDone;
    bool DParseError;

    SImplementation(std::shared_ptr<CDataSource> src)
        : DSource(src),
          DParser(nullptr),
          DDone(false),
          DParseError(false)
    {
        // init Expat
        DParser = XML_ParserCreate(nullptr);
        // if (!DParser)
        // {
        //     DParseError = true;
        //     return;
        // }
        XML_SetUserData(DParser, this);
        XML_SetElementHandler(DParser, StartElementHandler, EndElementHandler);
        XML_SetCharacterDataHandler(DParser, CharacterDataHandler);
    }

    ~SImplementation()
    {
        if (DParser)
        {
            XML_ParserFree(DParser);
        }
    }

    static void StartElementHandler(void *userData, const XML_Char *name, const XML_Char **atts)
    {
        auto *This = static_cast<SImplementation *>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::StartElement;
        Entity.DNameData = name ? std::string(name) : std::string();

        if (atts)
        {
            for (int Index = 0; atts[Index] != nullptr; Index += 2)
            {
                std::string AttrName = atts[Index] ? std::string(atts[Index]) : std::string();
                std::string AttrValue = atts[Index + 1] ? std::string(atts[Index + 1]) : std::string();
                Entity.DAttributes.push_back(std::make_pair(AttrName, AttrValue));
            }
        }
        This->DQueue.push_back(Entity);
    }

    static void EndElementHandler(void *userData, const XML_Char *name)
    {
        auto *This = static_cast<SImplementation *>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::EndElement;
        Entity.DNameData = name ? std::string(name) : std::string();
        This->DQueue.push_back(Entity);
    }

    static void CharacterDataHandler(void *userData, const XML_Char *s, int len)
    {
        auto *This = static_cast<SImplementation *>(userData);
        SXMLEntity Entity;
        Entity.DType = SXMLEntity::EType::CharData;
        Entity.DNameData.assign(s, s + len);
        This->DQueue.push_back(Entity);
    }

    bool ParseMore()
    {

        std::vector<char> Buffer;
        if (DSource->Read(Buffer, kReadBufferSize) && !Buffer.empty())
        {
            // feed next chunk into expat
            if (XML_Parse(DParser, Buffer.data(), static_cast<int>(Buffer.size()), XML_FALSE) == XML_STATUS_ERROR)
            {
                DParseError = true;
                DQueue.clear();
                return false;
            }
            return true;
        }

        // if we each an eof then end parsing
        if (XML_Parse(DParser, "", 0, XML_TRUE) == XML_STATUS_ERROR)
        {
            DParseError = true;
            DQueue.clear();
            return false;
        }
        DDone = true;
        return true;
    }
};

CXMLReader::CXMLReader(std::shared_ptr<CDataSource> src)
{
    DImplementation = std::make_unique<SImplementation>(src);
}

CXMLReader::~CXMLReader() = default;

bool CXMLReader::End() const
{
    // parsing is complete if 1) no more input OR 2) error OR 3) queue is empty
    return (DImplementation->DDone || DImplementation->DParseError) && DImplementation->DQueue.empty();
}

bool CXMLReader::ReadEntity(SXMLEntity &entity, bool skipcdata)
{
    while (true)
    {
        if (DImplementation->DQueue.empty())
        {
            if (!DImplementation->ParseMore())
            {
                break;
            }
        }
        if (DImplementation->DQueue.empty())
        {
            break;
        }
        if (skipcdata && DImplementation->DQueue.front().DType == SXMLEntity::EType::CharData)
        {
            DImplementation->DQueue.pop_front();
            continue;
        }
        entity = DImplementation->DQueue.front();
        DImplementation->DQueue.pop_front();
        return true;
    }
    return false;
}
