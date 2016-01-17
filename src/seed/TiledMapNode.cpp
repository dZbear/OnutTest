#include "TiledMapNode.h"
#include "TiledMap.h"
#include "tinyxml2.h"

namespace seed
{
    TiledMapNode::TiledMapNode()
    {
    }

    TiledMapNode::~TiledMapNode()
    {
        if (m_tiledMap)
        {
            delete m_tiledMap;
        }
    }

    Node* TiledMapNode::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        TiledMapNode* newNode = in_pool.alloc<TiledMapNode>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* TiledMapNode::Duplicate() const
    {
        TiledMapNode* newNode = new TiledMapNode();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void TiledMapNode::Copy(Node* in_copy) const
    {
        Node::Copy(in_copy);
        TiledMapNode* copy = (TiledMapNode*)in_copy;

        copy->Init(m_file);
    }

    tinyxml2::XMLElement* TiledMapNode::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement* xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("TiledMapNode");
        xmlNode->SetAttribute("map", m_file.c_str());

        return xmlNode;
    }

    void TiledMapNode::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);
    }

    void TiledMapNode::Init(const string& in_file)
    {
        if (m_file == in_file) return;

        if (m_tiledMap)
        {
            delete m_tiledMap;
            m_tiledMap = nullptr;
        }

        m_file = in_file;

        if (!m_file.empty())
        {
            std::string mapFile = OContentManager->find(m_file);
            if (mapFile.empty()) mapFile = m_file;
            if (!mapFile.empty())
            {
                m_tiledMap = new onut::TiledMap(mapFile);
            }
        }
    }

    const string& TiledMapNode::GetFile() const
    {
        return m_file;
    }

    onut::TiledMap* TiledMapNode::GetTiledMap() const
    {
        return m_tiledMap;
    }

    void TiledMapNode::Update()
    {
        Node::Update();
    }

    void TiledMapNode::Render(Matrix* in_parentMatrix, float in_parentAlpha)
    {
        if (!m_visible)
        {
            return;
        }

        // generate our matrix
        Matrix transform = Matrix::Identity;
        transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (in_parentMatrix)
        {
            transform = transform * *in_parentMatrix;
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // render the map
        if (m_tiledMap)
        {
            OSB->end();
            Matrix spriteBatchTransform = OSB->getTransform();
            OSB->changeFiltering(onut::SpriteBatch::eFiltering::Nearest);
            m_tiledMap->setTransform(transform * spriteBatchTransform);
            m_tiledMap->render();
            OSB->begin(spriteBatchTransform);
        }

        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
    }
}
