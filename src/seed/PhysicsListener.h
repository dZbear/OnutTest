#pragma once
#ifdef WITH_BOX_2D
    #include <Box2D/Box2D.h>
#endif

namespace seed
{
    class PhysicsListener : public b2ContactListener
    {
    public:

        PhysicsListener();
        virtual ~PhysicsListener();

        void Init(View* in_view, BodyMap* in_bodyMap);

        //  b2ContactListener
        // Called when two fixtures begin to touch
        virtual void BeginContact(b2Contact* contact);

        // Called when two fixtures cease to touch
        virtual void EndContact(b2Contact* contact);

    private:

        // view we call for callbacks
        View*       m_view;

        // Map of bodies to match collisions with Nodes
        BodyMap*    m_bodies;

        Node*           GetNodeFromPhysicsBody(PhysicsBody* in_body);
        PhysicsBody*    GetPhysicsBodyFromB2Body(b2Body* in_body);
        
    };
}
