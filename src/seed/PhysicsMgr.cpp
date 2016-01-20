#include "App.h"
//#include "Node.h"
#include "PhysicsMgr.h"
#include "PhysicsBody.h"
#include "Node.h"

namespace seed
{
    PhysicsMgr::PhysicsMgr()
        : m_world(nullptr)
    {
    }

    PhysicsMgr::~PhysicsMgr()
    {
    }

    void PhysicsMgr::Init(const Vector2& in_gravity, float in_pixelToMetersRatio )
    {
        b2Vec2 gravity(in_gravity.x, in_gravity.y);
        m_world = new b2World(gravity);
        m_pixelToMetersRatio = in_pixelToMetersRatio;
    }

    void PhysicsMgr::Update()
    {
        if (!m_world)
            return;
        
        m_world->Step(ODT, 6, 2);

        // update all nodes positions/angles based on their physic body
        BodyMap::const_iterator it = m_bodies.begin();
        BodyMap::const_iterator itEnd = m_bodies.end();
        for (; it != itEnd; ++it)
        {
            seed::Node* node = it->first;
            node->SetPosition(it->second->GetPosition() * m_pixelToMetersRatio);
            node->SetAngle(it->second->GetAngle() * m_pixelToMetersRatio);
        }
    }

    PhysicsBody* PhysicsMgr::CreateBoxPhysicsForNode(Node* in_node, bool in_static)
    {
        if (!m_world)
        {
            OLogE("PhysicsMgr::CreatePhysicsForNode called before PhysicsMgr::Init");
            return nullptr;
        }

        PhysicsBody* newBody = new PhysicsBody();
        newBody->SetPixelToMetersRatio(m_pixelToMetersRatio);
        newBody->InitAsBox(in_node->GetPosition() / m_pixelToMetersRatio, Vector2(in_node->GetWidth() / in_node->GetScale().x, in_node->GetHeight() / in_node->GetScale().y) / m_pixelToMetersRatio, m_world, in_static);
        m_bodies[in_node] = newBody;
        return newBody;
    }

    PhysicsBody* PhysicsMgr::CreateCirclePhysicsForNode(Node* in_node, float in_radius, bool in_static)
    {
        if (!m_world)
        {
            OLogE("PhysicsMgr::CreatePhysicsForNode called before PhysicsMgr::Init");
            return nullptr;
        }

        PhysicsBody* newBody = new PhysicsBody();
        newBody->SetPixelToMetersRatio(m_pixelToMetersRatio);
        newBody->InitAsCircle(in_node->GetPosition() / m_pixelToMetersRatio, in_radius, m_world, in_static);
        m_bodies[in_node] = newBody;
        return newBody;
    }

    PhysicsBody* PhysicsMgr::GetBodyForNode(Node* in_node)
    {
        BodyMap::const_iterator it = m_bodies.find(in_node);
        if (it == m_bodies.end())
        {
            OLogE("Node specified to PhysicsMgr::GetPhysicsForNode has no physics created yet");
            return nullptr;
        }
        return it->second;
    }


}



