#include "XMLWriter.h"

#include <vector>

namespace
{
    std::string EscapeText(const std::string &value)
    {
        std::string Out;
        Out.reserve(value.size());
        for (char ch : value)
        {
            // TODO: i should probalby change this to switch statements over time
            if (ch == '&')
            {
                Out += "&amp;";
            }
            else if (ch == '<')
            {
                Out += "&lt;";
            }
            else if (ch == '>')
            {
                Out += "&gt;";
            }
            else
            {
                Out += ch;
            }
        }
        return Out;
    }

    std::string EscapeAttribute(const std::string &value)
    {
        std::string Out;
        Out.reserve(value.size());
        for (char ch : value)
        {
            // this maybe shoudl be switch statements over time
            if (ch == '&')
            {
                Out += "&amp;";
            }
            else if (ch == '<')
            {
                Out += "&lt;";
            }
            else if (ch == '>')
            {
                Out += "&gt;";
            }
            else if (ch == '\"')
            {
                Out += "&quot;";
            }
            else if (ch == '\'')
            {
                Out += "&apos;";
            }
            else
            {
                Out += ch;
            }
        }
        return Out;
    }
}

struct CXMLWriter::SImplementation
{
    std::shared_ptr<CDataSink> DSink;
    std::vector<std::string> DElementStack;

    SImplementation(std::shared_ptr<CDataSink> sink) : DSink(sink)
    {
    }

    bool WriteString(const std::string &value)
    {
        std::vector<char> Buffer(value.begin(), value.end());
        return DSink->Write(Buffer);
    }

    bool WriteStartTag(const SXMLEntity &entity)
    {
        // should validate DNameData isn't empty before starting
        if (!WriteString("<"))
        {
            return false;
        }
        if (!WriteString(entity.DNameData))
        {
            return false;
        }
        for (const auto &Attribute : entity.DAttributes)
        {
            if (!WriteString(" "))
            {
                return false;
            }
            if (!WriteString(Attribute.first))
            {
                return false;
            }
            if (!WriteString("=\""))
            {
                return false;
            }
            if (!WriteString(EscapeAttribute(Attribute.second)))
            {
                return false;
            }
            if (!WriteString("\""))
            {
                return false;
            }
        }
        if (!WriteString(">"))
        {
            return false;
        }
        return true;
    }

    bool WriteEndTag(const std::string &name)
    {
        // end is either terminated or empty
        if (!WriteString("</"))
        {
            return false;
        }
        if (!WriteString(name))
        {
            return false;
        }
        if (!WriteString(">"))
        {
            return false;
        }
        return true;
    }

    bool WriteCompleteTag(const SXMLEntity &entity)
    {
        // use <tag/> or <tag></tag>?
        if (!WriteString("<"))
        {
            return false;
        }
        if (!WriteString(entity.DNameData))
        {
            return false;
        }
        for (const auto &Attribute : entity.DAttributes)
        {
            if (!WriteString(" "))
            {
                return false;
            }
            if (!WriteString(Attribute.first))
            {
                return false;
            }
            if (!WriteString("=\""))
            {
                return false;
            }
            if (!WriteString(EscapeAttribute(Attribute.second)))
            {
                return false;
            }
            if (!WriteString("\""))
            {
                return false;
            }
        }
        if (!WriteString("/>"))
        {
            return false;
        }
        return true;
    }

    bool WriteEntity(const SXMLEntity &entity)
    {
        if (entity.DType == SXMLEntity::EType::StartElement)
        {
            if (!WriteStartTag(entity))
            {
                return false;
            }
            DElementStack.push_back(entity.DNameData);
            return true;
        }
        if (entity.DType == SXMLEntity::EType::EndElement)
        {
            // if name is empty, try to close the last opened element
            if (DElementStack.empty())
            {
                return false;
            }
            std::string Name = entity.DNameData.empty() ? DElementStack.back() : entity.DNameData;
            if (!WriteEndTag(Name))
            {
                return false;
            }
            if (!DElementStack.empty() && DElementStack.back() == Name)
            {
                DElementStack.pop_back();
            }
            return true;
        }
        if (entity.DType == SXMLEntity::EType::CompleteElement)
        {
            return WriteCompleteTag(entity);
        }
        if (entity.DType == SXMLEntity::EType::CharData)
        {
            return WriteString(EscapeText(entity.DNameData));
        }
        return false;
    }

    bool Flush()
    {
        while (!DElementStack.empty())
        {
            std::string Name = DElementStack.back();
            DElementStack.pop_back();

            if (!WriteEndTag(Name))
            {
                return false;
            }
        }
        return true;
    }
};

CXMLWriter::CXMLWriter(std::shared_ptr<CDataSink> sink)
{
    DImplementation = std::make_unique<SImplementation>(sink);
}

CXMLWriter::~CXMLWriter() = default;

bool CXMLWriter::Flush()
{
    return DImplementation->Flush();
}

bool CXMLWriter::WriteEntity(const SXMLEntity &entity)
{
    return DImplementation->WriteEntity(entity);
}
