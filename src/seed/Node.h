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

        virtual void    Update();
        virtual void    Render(Matrix* in_parentMatrix = nullptr) {};

        void            Attach(Node* in_newChild, int in_zIndex = INT_MAX);
        void            Detach(Node* in_child);
        Node*           GetParent() const;
        void            SetParent(Node* in_parent);
        void            SetZindex(int in_zIndex);
        int             GetZindex();
        void            SetPosition(const Vector2& in_position);
        void            SetPosition(float in_x, float in_y);
        Vector2         GetPosition();
        OAnim<Vector2>& GetPositionAnim();
        Vector2         GetAbsolutePosition();
        void            SetScale(const Vector2& in_scale);
        Vector2         GetScale();
        OAnim<Vector2>& GetScaleAnim();
        void            SetAngle(float in_angle);
        float           GetAngle();
        OAnim<float>&   GetAngleAnim();
        void            SetColor(const Color& in_color);
        Color           GetColor();
        OAnim<Color>&   GetColorAnim();
        Matrix          GetTransform() const;
        void            SetVisible(bool in_visible);
        bool            GetVisible();

        virtual float   GetWidth() { return 0; }
        virtual float   GetHeight() { return 0; }

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

        void        RenderChildren(NodeVect& in_children, Matrix* in_parentMatrix = nullptr);
        void        UpdateChildren(NodeVect& in_children);

    private:

        void        InsertNode(NodeVect& in_vect, Node* in_node, int in_zIndex);
        void        DetachChild(NodeVect& in_vect, Node* in_node);
    };
}