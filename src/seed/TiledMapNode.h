#pragma once

#include "Node.h"

namespace onut
{
    class TiledMap;
};

namespace seed
{
    class TiledMapNode : public Node
    {
    public:

        TiledMapNode();
        virtual ~TiledMapNode();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const override;
        virtual Node*                   Duplicate() const override;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const override;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode) override;

        void            Init(const string& in_file);
        const string&   GetFile() const;
        onut::TiledMap* GetTiledMap() const;

        // only to be used by the seed sdk
        virtual void    Update() override;
        virtual void    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;

    protected:

        virtual void    Copy(Node* in_copy) const;

    private:

        string          m_file;
        onut::TiledMap* m_tiledMap = nullptr;
    };
}
