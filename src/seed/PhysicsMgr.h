#pragma once
#ifdef WITH_BOX_2D
    #include <Box2D/Box2D.h>
#endif

namespace seed
{
    class PhysicsListener;
    class PhysicsMgr
    {
    public:

        PhysicsMgr();
        ~PhysicsMgr();

        void            Init(View* in_container, const Vector2& in_gravity, float in_pixelToMetersRatio=64.f);
        void            Update();
        void            Reset();

        PhysicsBody*    CreateBoxPhysicsForNode(Node* in_node, bool in_static);
        PhysicsBody*    CreateCirclePhysicsForNode(Node* in_node, float in_radius, bool in_static);
        PhysicsBody*    GetBodyForNode(Node* in_node);

        
    private:

        b2World*            m_world;
        BodyMap             m_bodies;
        PhysicsListener*    m_listener;
        View*               m_container;

        float       m_pixelToMetersRatio;
    };
}
