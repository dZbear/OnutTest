#include "App.h"
#include "PhysicsListener.h"
#include "PhysicsBody.h"
#include "View.h"


namespace seed
{
    PhysicsListener::PhysicsListener()
    {
    }

    PhysicsListener::~PhysicsListener()
    {
    }

    void PhysicsListener::Init(View* in_view, BodyMap* in_bodyMap)
    {
        m_view = in_view;
        m_bodies = in_bodyMap;
    }

    void PhysicsListener::BeginContact(b2Contact* contact)
    {
        Node* nodeA = GetNodeFromPhysicsBody(GetPhysicsBodyFromB2Body(contact->GetFixtureA()->GetBody()));
        Node* nodeB = GetNodeFromPhysicsBody(GetPhysicsBodyFromB2Body(contact->GetFixtureB()->GetBody()));
        m_view->OnCollisionStart(nodeA, nodeB);
    }

    
    void PhysicsListener::EndContact(b2Contact* contact)
    {
        Node* nodeA = GetNodeFromPhysicsBody(GetPhysicsBodyFromB2Body(contact->GetFixtureA()->GetBody()));
        Node* nodeB = GetNodeFromPhysicsBody(GetPhysicsBodyFromB2Body(contact->GetFixtureB()->GetBody()));
        m_view->OnCollisionEnd(nodeA, nodeB);
    }

    Node* PhysicsListener::GetNodeFromPhysicsBody(PhysicsBody* in_body)
    {
        BodyMap::const_iterator it = m_bodies->begin();
        BodyMap::const_iterator itEnd = m_bodies->end();

        for (; it != itEnd; ++it)
        {
            if (it->second == in_body)
            {
                return it->first;
            }
        }
        return nullptr;
    }

    PhysicsBody* PhysicsListener::GetPhysicsBodyFromB2Body(b2Body* in_body)
    {
        BodyMap::const_iterator it = m_bodies->begin();
        BodyMap::const_iterator itEnd = m_bodies->end();

        for (; it != itEnd; ++it)
        {
            if (it->second->GetB2Body() == in_body)
            {
                return it->second;
            }
        }
        return nullptr;
    }

}



