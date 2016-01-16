#pragma once

#include "onut.h"
#include "SeedGlobals.h"

namespace seed
{
    class Node
    {
    public:

        Node();
        virtual ~Node();

        virtual void                    Update();
        virtual void                    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f);
        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*                   Duplicate() const;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void            Attach(Node* in_newChild, int in_zIndex = INT_MAX);
        void            AttachBefore(Node* in_newChild, Node* in_beforeChild);
        void            AttachAfter(Node* in_newChild, Node* in_afterChild);
        void            Detach(Node* in_child);
        Node*           GetParent() const;
        void            SetParent(Node* in_parent);
        NodeVect&       GetFgChildren();
        NodeVect&       GetBgChildren();
        const NodeVect& GetFgChildren() const;
        const NodeVect& GetBgChildren() const;
        void            SetZindex(int in_zIndex);
        int             GetZindex() const;
        void            SetPosition(const Vector2& in_position);
        const Vector2&  GetPosition() const;
        OAnim<Vector2>& GetPositionAnim();
        Vector2         GetAbsolutePosition() const;
        void            SetScale(const Vector2& in_scale);
        const Vector2&  GetScale() const;
        OAnim<Vector2>& GetScaleAnim();
        void            SetAngle(float in_angle);
        float           GetAngle() const;
        OAnim<float>&   GetAngleAnim();
        void            SetColor(const Color& in_color);
        const Color&    GetColor() const;
        OAnim<Color>&   GetColorAnim();
        Matrix          GetTransform() const;
        void            SetVisible(bool in_visible);
        bool            GetVisible() const;
        bool            GetReallyVisible() const;
        const string&   GetName() const;
        void            SetName(const string& in_name);

        virtual float   GetWidth() const { return 0; }
        virtual float   GetHeight() const { return 0; }

        // Visit all children in order.
        // Return true from the callback to interrupt searching.
        // VisitBackward to start with last child and their last children first
        using VisitCallback = std::function<bool(Node*)>;
        bool VisitBackgroundChildren(const VisitCallback& callback);
        bool VisitBackgroundChildrenBackward(const VisitCallback& callback);
        bool VisitForegroundChildren(const VisitCallback& callback);
        bool VisitForegroundChildrenBackward(const VisitCallback& callback);

        Node*           FindNode(const string& in_name);

        virtual void            Copy(Node* in_copy) const;

    protected:

        int                     m_zIndex;
        Node*                   m_parent;
        NodeVect                m_bgChildren;
        NodeVect                m_fgChildren;
        OAnim<Vector2>          m_position;
        OAnim<Vector2>          m_scale;
        OAnim<float>            m_angle;
        OAnim<Color>            m_color;
        bool                    m_visible;
        string                  m_name;

        void        DuplicateChildren(Node* parent, onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        void        DuplicateChildren(Node* parent) const;
        void        RenderChildren(NodeVect& in_children, Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f);
        void        UpdateChildren(NodeVect& in_children);

    private:

        void        InsertNode(NodeVect& in_vect, Node* in_node, int in_zIndex);
        void        InsertBefore(NodeVect& in_vect, Node* in_newChild, Node* in_beforeChild);
        void        InsertAfter(NodeVect& in_vect, Node* in_newChild, Node* in_afterChild);
        void        DetachChild(NodeVect& in_vect, Node* in_node);
    };
}
