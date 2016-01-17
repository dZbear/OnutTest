#include "Effect.h"
#include "Emitter.h"
#include "Node.h"
#include "MusicEmitter.h"
#include "SoundEmitter.h"
#include "Sprite.h"
#include "SpriteString.h"
#include "TiledMapNode.h"
#include "tinyxml2.h"
#include "Video.h"
#include "View.h"

namespace seed
{
    Node::Node()
        : m_parent(nullptr)
    {
        m_scale = Vector2(1.f, 1.f);
        m_angle = 0;
        m_color = Color(1.f, 1.f, 1.f, 1.f);
        m_visible = true;
    }

    Node::~Node()
    {
        if (m_parent)
        {
            m_parent->Detach(this);
        }
        for (Node* pChild : m_bgChildren)
        {
            pChild->m_parent = nullptr;
            delete pChild;
        }
        for (Node* pChild : m_fgChildren)
        {
            pChild->m_parent = nullptr;
            delete pChild;
        }
    }

    Node* Node::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        Node* newNode = in_pool.alloc<Node>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* Node::Duplicate() const
    {
        Node* newNode = new Node();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void Node::DuplicateChildren(Node* parent, onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        for (Node* childNode : GetBgChildren())
        {
            Node* newChildNode = childNode->Duplicate(in_pool, in_pooledNodes);
            parent->Attach(newChildNode, newChildNode->GetZindex());
        }

        for (Node* childNode : GetFgChildren())
        {
            Node* newChildNode = childNode->Duplicate(in_pool, in_pooledNodes);
            parent->Attach(newChildNode, newChildNode->GetZindex());
        }
    }

    void Node::DuplicateChildren(Node* parent) const
    {
        for (Node* childNode : GetBgChildren())
        {
            Node* newChildNode = childNode->Duplicate();
            parent->Attach(newChildNode, newChildNode->GetZindex());
        }

        for (Node* childNode : GetFgChildren())
        {
            Node* newChildNode = childNode->Duplicate();
            parent->Attach(newChildNode, newChildNode->GetZindex());
        }
    }

    void Node::Copy(Node* in_copy) const
    {
        in_copy->m_zIndex = m_zIndex;
        in_copy->m_position = m_position;
        in_copy->m_scale = m_scale;
        in_copy->m_angle = m_angle;
        in_copy->m_color = m_color;
        in_copy->m_visible = m_visible;
        in_copy->m_name = m_name;
    }

    tinyxml2::XMLElement* Node::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement* xmlNode = in_xmlDoc->NewElement("Node");

        xmlNode->SetAttribute("name", GetName().c_str());
        xmlNode->SetAttribute("zIndex", GetZindex());
        xmlNode->SetAttribute("x", GetPosition().x);
        xmlNode->SetAttribute("y", GetPosition().y);
        xmlNode->SetAttribute("scaleX", GetScale().x);
        xmlNode->SetAttribute("scaleY", GetScale().y);
        xmlNode->SetAttribute("angle", GetAngle());
        xmlNode->SetAttribute("r", GetColor().x);
        xmlNode->SetAttribute("g", GetColor().y);
        xmlNode->SetAttribute("b", GetColor().z);
        xmlNode->SetAttribute("a", GetColor().w);
        xmlNode->SetAttribute("visible", GetVisible());

        for (auto pChild : m_bgChildren)
        {
            xmlNode->InsertEndChild(pChild->Serialize(in_xmlDoc));
        }
        for (auto pChild : m_fgChildren)
        {
            xmlNode->InsertEndChild(pChild->Serialize(in_xmlDoc));
        }

        return xmlNode;
    }

    void Node::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        const char* szName = in_xmlNode->Attribute("name");
        string name = GetName();
        if (szName)
        {
            name = szName;
        }
        SetName(name);

        Vector2 position = GetPosition();
        in_xmlNode->QueryAttribute("x", &position.x);
        in_xmlNode->QueryAttribute("y", &position.y);
        SetPosition(position);

        Vector2 scale = GetScale();
        in_xmlNode->QueryAttribute("scaleX", &scale.x);
        in_xmlNode->QueryAttribute("scaleY", &scale.y);
        SetScale(scale);

        float angle = GetAngle();
        in_xmlNode->QueryAttribute("angle", &angle);
        SetAngle(angle);

        Color color = GetColor();
        in_xmlNode->QueryAttribute("r", &color.x);
        in_xmlNode->QueryAttribute("g", &color.y);
        in_xmlNode->QueryAttribute("b", &color.z);
        in_xmlNode->QueryAttribute("a", &color.w);
        SetColor(color);

        bool visible = GetVisible();
        in_xmlNode->QueryAttribute("visible", &visible);
        SetVisible(visible);

        for (tinyxml2::XMLElement* xmlChild = in_xmlNode->FirstChildElement(); xmlChild; xmlChild = xmlChild->NextSiblingElement())
        {
            Node* pChild = nullptr;
            int childZIndex = GetZindex();

            string childType = xmlChild->Name();
            xmlChild->QueryAttribute("zIndex", &childZIndex);

            if (childType == "Node")
            {
                pChild = view->CreateNode();
                Attach(pChild, childZIndex);
            }
            else if (childType == "Sprite")
            {
                const char* szTextureName = xmlChild->Attribute("texture");
                string textureName;
                if (szTextureName)
                {
                    textureName = szTextureName;
                }
                pChild = view->CreateSprite(textureName);
                Attach(pChild, childZIndex);
            }
            else if (childType == "SpriteString")
            {
                const char* szFontName = xmlChild->Attribute("font");
                string fontName;
                if (szFontName)
                {
                    fontName = szFontName;
                }
                pChild = view->CreateSpriteString(fontName);
                Attach(pChild, childZIndex);
            }
            else if (childType == "Emitter")
            {
                const char* szFxName = xmlChild->Attribute("fx");
                string fxName;
                if (szFxName)
                {
                    fxName = szFxName;
                }
                pChild = view->CreateEmitter(fxName);
                Attach(pChild, childZIndex);
            }
            else if (childType == "SoundEmitter")
            {
                const char* szSource = xmlChild->Attribute("source");
                string source;
                if (szSource)
                {
                    source = szSource;
                }
                pChild = view->CreateSoundEmitter(source);
                Attach(pChild, childZIndex);
            }
            else if (childType == "MusicEmitter")
            {
                pChild = view->CreateMusicEmitter();
                Attach(pChild, childZIndex);
            }
            else if (childType == "Video")
            {
                pChild = view->CreateVideo();
                Attach(pChild, childZIndex);
            }
            else if (childType == "Effect")
            {
                pChild = view->CreateEffect();
                Attach(pChild, childZIndex);
            }
            else if (childType == "TiledMapNode")
            {
                const char* szMap = xmlChild->Attribute("map");
                string map;
                if (szMap)
                {
                    map = szMap;
                }
                pChild = view->CreateTiledMapNode(map);
                Attach(pChild, childZIndex);
            }

            if (pChild)
            {
                pChild->Deserialize(view, xmlChild);
            }
            else
            {
                OLogE("Unrecognized Node Type: " + childType);
            }
        }
    }

    void Node::Update()
    {
        UpdateChildren(m_bgChildren);
        UpdateChildren(m_fgChildren);
    }

    void Node::Render(Matrix* in_parentMatrix, float in_parentAlpha)
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

        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
    }

    void Node::SetZindex(int in_zIndex)
    {
        m_zIndex = in_zIndex;
    }

    int Node::GetZindex() const
    {
        return m_zIndex;
    }

    void Node::Attach(Node* in_newChild, int in_zIndex)
    {
        if (in_zIndex < 0)
        {
            InsertNode(m_bgChildren, in_newChild, in_zIndex);
        }
        else
        {
            InsertNode(m_fgChildren, in_newChild, in_zIndex);
        }
        in_newChild->SetParent(this);
        in_newChild->SetZindex(in_zIndex);
    }

    void Node::AttachBefore(Node* in_newChild, Node* in_beforeChild)
    {
        if (in_beforeChild->GetZindex() < 0)
        {
            InsertBefore(m_bgChildren, in_newChild, in_beforeChild);
        }
        else
        {
            InsertBefore(m_fgChildren, in_newChild, in_beforeChild);
        }
    }

    void Node::AttachAfter(Node* in_newChild, Node* in_afterChild)
    {
        if (in_afterChild->GetZindex() < 0)
        {
            InsertAfter(m_bgChildren, in_newChild, in_afterChild);
        }
        else
        {
            InsertAfter(m_fgChildren, in_newChild, in_afterChild);
        }
    }

    void Node::Detach(Node* in_child)
    {
        if (in_child->GetZindex() < 0)
        {
            DetachChild(m_bgChildren, in_child);
        }
        else
        {
            DetachChild(m_fgChildren, in_child);
        }
        in_child->SetParent(nullptr);
    }

    void Node::InsertNode(NodeVect& in_vect, Node* in_node, int in_zIndex)
    {
        if (in_zIndex < 0)
        {
            // inserted in background children
            for (size_t i = 0, size = in_vect.size(); i < size; ++i)
            {
                if (in_vect[i]->GetZindex() < in_zIndex)
                {
                    // let's insert before this one
                    in_vect.insert(in_vect.begin() + i, in_node);
                    return;
                }
            }
        }
        else
        {
            // inserted in foreground children
            for (size_t i = 0, size = in_vect.size(); i < size; ++i)
            {
                if (in_vect[i]->GetZindex() > in_zIndex)
                {
                    // let's insert before this one
                    in_vect.insert(in_vect.begin() + i, in_node);
                    return;
                }
            }
        }
        // if we're here it means we didnt find any suitable place for it, just insert at the end
        in_vect.push_back(in_node);
    }

    void Node::InsertBefore(NodeVect& in_vect, Node* in_newChild, Node* in_beforeChild)
    {
        for (size_t i = 0, size = in_vect.size(); i < size; ++i)
        {
            Node* child = in_vect[i];
            if (child == in_beforeChild)
            {
                in_vect.insert(in_vect.begin() + i, in_newChild);
                in_newChild->SetParent(this);
                in_newChild->SetZindex(child->GetZindex());
                return;
            }
        }
    }

    void Node::InsertAfter(NodeVect& in_vect, Node* in_newChild, Node* in_afterChild)
    {
        for (size_t i = 0, size = in_vect.size(); i < size; ++i)
        {
            Node* child = in_vect[i];
            if (child == in_afterChild)
            {
                in_vect.insert(in_vect.begin() + i + 1, in_newChild);
                in_newChild->SetParent(this);
                in_newChild->SetZindex(child->GetZindex());
                return;
            }
        }
    }

    void Node::DetachChild(NodeVect& in_vect, Node* in_node)
    {
        // inserted in background children
        for (size_t i = 0, size = in_vect.size(); i < size; ++i)
        {
            if (in_vect[i] == in_node)
            {
                // let's insert before this one
                in_vect.erase(in_vect.begin() + i);
                return;
            }
        }
    }

    void Node::SetParent(Node* in_parent)
    {
        m_parent = in_parent;
    }

    Node* Node::GetParent() const
    {
        return m_parent;
    }

    void Node::RenderChildren(NodeVect& in_children, Matrix* in_parentMatrix, float in_parentAlpha)
    {
        for (Node* node : in_children)
        {
            node->Render(in_parentMatrix, in_parentAlpha);
        }
    }

    void Node::UpdateChildren(NodeVect& in_children)
    {
        for (Node* node : in_children)
        {
            node->Update();
        }
    }

    void Node::SetPosition(const Vector2& in_position)
    {
        m_position = in_position;
    }

    const Vector2& Node::GetPosition() const
    {
        return m_position.get();
    }

    OAnim<Vector2>& Node::GetPositionAnim()
    {
        return m_position;
    }

    Vector2 Node::GetAbsolutePosition() const
    {
        Vector2 pos = GetPosition();
        Node* parent = GetParent();
        while (parent)
        {
            pos += parent->GetPosition();
            parent = parent->GetParent();
        }
        return pos;
    }

    void Node::SetScale(const Vector2& in_scale)
    {
        m_scale = in_scale;
    }

    const Vector2& Node::GetScale() const
    {
        return m_scale.get();
    }

    OAnim<Vector2>& Node::GetScaleAnim()
    {
        return m_scale;
    }

    void Node::SetAngle(float in_angle)
    {
        m_angle = in_angle;
    }

    float Node::GetAngle() const
    {
        return m_angle.get();
    }

    OAnim<float>& Node::GetAngleAnim()
    {
        return m_angle;
    }

    void Node::SetColor(const Color& in_color)
    {
        m_color = in_color;
    }

    const Color& Node::GetColor() const
    {
        return m_color.get();
    }

    OAnim<Color>& Node::GetColorAnim()
    {
        return m_color;
    }

    Matrix Node::GetTransform() const
    {
        Matrix transform = Matrix::Identity;
        transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (GetParent())
        {
            transform = transform * GetParent()->GetTransform();
        }

        return std::move(transform);
    }

    void Node::SetVisible(bool in_visible)
    {
        m_visible = in_visible;
    }

    bool Node::GetVisible() const
    {
        return m_visible;
    }

    bool Node::GetReallyVisible() const
    {
        if (!m_visible) return false;

        if (GetParent())
        {
            return m_visible && GetParent()->GetReallyVisible();
        }

        return m_visible;
    }

    NodeVect& Node::GetFgChildren()
    {
        return m_fgChildren;
    }

    NodeVect& Node::GetBgChildren()
    {
        return m_bgChildren;
    }

    const NodeVect& Node::GetFgChildren() const
    {
        return m_fgChildren;
    }

    const NodeVect& Node::GetBgChildren() const
    {
        return m_bgChildren;
    }

    bool Node::VisitBackgroundChildren(const VisitCallback& callback)
    {
        for (Node* node : m_bgChildren)
        {
            if (node->VisitBackgroundChildren(callback)) return true;
            if (callback(node)) return true;
        }
        return false;
    }

    bool Node::VisitBackgroundChildrenBackward(const VisitCallback& callback)
    {
        NodeVect::const_reverse_iterator end = m_bgChildren.rend();
        for (NodeVect::const_reverse_iterator it = m_bgChildren.rbegin(); it != end; ++it)
        {
            Node* node = *it;
            if (callback(node)) return true;
            if (node->VisitBackgroundChildrenBackward(callback)) return true;
        }
        return false;
    }

    bool Node::VisitForegroundChildren(const VisitCallback& callback)
    {
        for (Node* node : m_fgChildren)
        {
            if (callback(node)) return true;
            if (node->VisitForegroundChildren(callback)) return true;
        }
        return false;
    }

    bool Node::VisitForegroundChildrenBackward(const VisitCallback& callback)
    {
        NodeVect::const_reverse_iterator end = m_fgChildren.rend();
        for (NodeVect::const_reverse_iterator it = m_fgChildren.rbegin(); it != end; ++it)
        {
            Node* node = *it;
            if (node->VisitForegroundChildrenBackward(callback)) return true;
            if (callback(node)) return true;
        }
        return false;
    }

    const string& Node::GetName() const
    {
        return m_name;
    }

    void Node::SetName(const string& in_name)
    {
        m_name = in_name;
    }

    Node* Node::FindNode(const string& in_name)
    {
        if (in_name == m_name) return this;
        Node* foundChild = nullptr;

        if (VisitBackgroundChildren([&in_name, &foundChild](Node* child)
            {
                if (child->GetName() == in_name)
                {
                    foundChild = child;
                    return true;
                }
                return false;
            }))
        {
            return foundChild;
        }

        VisitForegroundChildren([&in_name, &foundChild](Node* child)
        {
            if (child->GetName() == in_name)
            {
                foundChild = child;
                return true;
            }
            return false;
        });

        return foundChild;
    }
}





