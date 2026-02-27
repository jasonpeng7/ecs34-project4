#include "OpenStreetMap.h"

#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct COpenStreetMap::SImplementation
{
    struct SNodeImpl : public CStreetMap::SNode
    {
        TNodeID DID;
        TLocation DLocation;
        std::vector<TAttribute> DAttributes;

        SNodeImpl(TNodeID id, double lat, double lon)
            : DID(id), DLocation(std::make_pair(lat, lon))
        {
        }

        TNodeID ID() const noexcept override
        {
            return DID;
        }

        TLocation Location() const noexcept override
        {
            return DLocation;
        }

        std::size_t AttributeCount() const noexcept override
        {
            return DAttributes.size();
        }

        std::string GetAttributeKey(std::size_t index) const noexcept override
        {
            if (index >= DAttributes.size())
            {
                return std::string();
            }
            return DAttributes[index].first;
        }

        bool HasAttribute(const std::string &key) const noexcept override
        {
            for (const auto &Attr : DAttributes)
            {
                if (Attr.first == key)
                {
                    return true;
                }
            }
            return false;
        }

        std::string GetAttribute(const std::string &key) const noexcept override
        {
            for (const auto &Attr : DAttributes)
            {
                if (Attr.first == key)
                {
                    return Attr.second;
                }
            }
            return std::string();
        }
    };

    struct SWayImpl : public CStreetMap::SWay
    {
        TWayID DID;
        std::vector<TNodeID> DNodeIDs;
        std::vector<TAttribute> DAttributes;

        explicit SWayImpl(TWayID id)
            : DID(id)
        {
        }

        TWayID ID() const noexcept override
        {
            return DID;
        }

        std::size_t NodeCount() const noexcept override
        {
            return DNodeIDs.size();
        }

        TNodeID GetNodeID(std::size_t index) const noexcept override
        {
            if (index >= DNodeIDs.size())
            {
                return std::numeric_limits<CStreetMap::TNodeID>::max();
            }
            return DNodeIDs[index];
        }

        std::size_t AttributeCount() const noexcept override
        {
            return DAttributes.size();
        }

        std::string GetAttributeKey(std::size_t index) const noexcept override
        {
            if (index >= DAttributes.size())
            {
                return std::string();
            }
            return DAttributes[index].first;
        }

        bool HasAttribute(const std::string &key) const noexcept override
        {
            for (const auto &Attr : DAttributes)
            {
                if (Attr.first == key)
                {
                    return true;
                }
            }
            return false;
        }

        std::string GetAttribute(const std::string &key) const noexcept override
        {
            for (const auto &Attr : DAttributes)
            {
                if (Attr.first == key)
                {
                    return Attr.second;
                }
            }
            return std::string();
        }
    };

    std::vector<std::shared_ptr<SNodeImpl>> DNodesByIndex;
    std::unordered_map<TNodeID, std::shared_ptr<SNodeImpl>> DNodesByID;

    std::vector<std::shared_ptr<SWayImpl>> DWaysByIndex;
    std::unordered_map<TWayID, std::shared_ptr<SWayImpl>> DWaysByID;

    SImplementation(std::shared_ptr<CXMLReader> src)
    {
        SXMLEntity Entity;
        std::shared_ptr<SNodeImpl> CurrentNode = nullptr;
        std::shared_ptr<SWayImpl> CurrentWay = nullptr;

        while (src->ReadEntity(Entity, true))
        {
            if (Entity.DType == SXMLEntity::EType::StartElement)
            {
                if (Entity.DNameData == "node")
                {
                    if (!Entity.AttributeExists("id") ||
                        !Entity.AttributeExists("lat") ||
                        !Entity.AttributeExists("lon"))
                    {
                        CurrentNode = nullptr;
                        continue;
                    }

                    try
                    {
                        TNodeID ID = std::stoull(Entity.AttributeValue("id"));
                        double Lat = std::stod(Entity.AttributeValue("lat"));
                        double Lon = std::stod(Entity.AttributeValue("lon"));

                        if (DNodesByID.find(ID) != DNodesByID.end())
                        {
                            CurrentNode = nullptr;
                            continue;
                        }

                        auto Node = std::make_shared<SNodeImpl>(ID, Lat, Lon);
                        DNodesByIndex.push_back(Node);
                        DNodesByID[ID] = Node;
                        CurrentNode = Node;
                    }
                    catch (...)
                    {
                        CurrentNode = nullptr;
                    }
                }
                else if (Entity.DNameData == "way")
                {
                    if (!Entity.AttributeExists("id"))
                    {
                        CurrentWay = nullptr;
                        continue;
                    }

                    try
                    {
                        TWayID ID = std::stoull(Entity.AttributeValue("id"));

                        if (DWaysByID.find(ID) != DWaysByID.end())
                        {
                            CurrentWay = nullptr;
                            continue;
                        }

                        auto Way = std::make_shared<SWayImpl>(ID);
                        DWaysByIndex.push_back(Way);
                        DWaysByID[ID] = Way;
                        CurrentWay = Way;
                    }
                    catch (...)
                    {
                        CurrentWay = nullptr;
                    }
                }
                else if (Entity.DNameData == "nd" && CurrentWay != nullptr)
                {
                    if (Entity.AttributeExists("ref"))
                    {
                        try
                        {
                            TNodeID Ref = std::stoull(Entity.AttributeValue("ref"));
                            CurrentWay->DNodeIDs.push_back(Ref);
                        }
                        catch (...)
                        {
                            // ignore malformed nd ref
                        }
                    }
                }
                else if (Entity.DNameData == "tag")
                {
                    if (Entity.AttributeExists("k") && Entity.AttributeExists("v"))
                    {
                        auto KV = std::make_pair(Entity.AttributeValue("k"),
                                                 Entity.AttributeValue("v"));

                        // prefer way tags if inside a way
                        if (CurrentWay != nullptr)
                        {
                            CurrentWay->DAttributes.push_back(KV);
                        }
                        else if (CurrentNode != nullptr)
                        {
                            CurrentNode->DAttributes.push_back(KV);
                        }
                    }
                }
            }
            else if (Entity.DType == SXMLEntity::EType::EndElement)
            {
                if (Entity.DNameData == "node")
                {
                    CurrentNode = nullptr;
                }
                else if (Entity.DNameData == "way")
                {
                    CurrentWay = nullptr;
                }
            }
        }
    }
};

COpenStreetMap::COpenStreetMap(std::shared_ptr<CXMLReader> src)
    : DImplementation(std::make_unique<SImplementation>(src))
{
}

COpenStreetMap::~COpenStreetMap() = default;

std::size_t COpenStreetMap::NodeCount() const noexcept
{
    return DImplementation->DNodesByIndex.size();
}

std::size_t COpenStreetMap::WayCount() const noexcept
{
    return DImplementation->DWaysByIndex.size();
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByIndex(std::size_t index) const noexcept
{
    if (index >= DImplementation->DNodesByIndex.size())
    {
        return nullptr;
    }
    return DImplementation->DNodesByIndex[index];
}

std::shared_ptr<CStreetMap::SNode> COpenStreetMap::NodeByID(TNodeID id) const noexcept
{
    auto It = DImplementation->DNodesByID.find(id);
    if (It == DImplementation->DNodesByID.end())
    {
        return nullptr;
    }
    return It->second;
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByIndex(std::size_t index) const noexcept
{
    if (index >= DImplementation->DWaysByIndex.size())
    {
        return nullptr;
    }
    return DImplementation->DWaysByIndex[index];
}

std::shared_ptr<CStreetMap::SWay> COpenStreetMap::WayByID(TWayID id) const noexcept
{
    auto It = DImplementation->DWaysByID.find(id);
    if (It == DImplementation->DWaysByID.end())
    {
        return nullptr;
    }
    return It->second;
}
