#include "App.h"
#include "PhysicsBody.h"



namespace seed
{
    PhysicsBody::PhysicsBody()
        : m_body(nullptr)
        , m_fixture(nullptr)
        , m_pixelToMeterRatio(64.f)
    {
    }

    PhysicsBody::~PhysicsBody()
    {
    }

    void PhysicsBody::InitAsBox(const Vector2& in_position, const Vector2& in_dimensions, b2World* in_world, bool in_static)
    {
        b2BodyDef bodyDef;
        bodyDef.type = in_static ? b2_staticBody : b2_dynamicBody;
        bodyDef.position.Set(in_position.x, in_position.y);
        bodyDef.gravityScale = 200.f;
        
        m_body = in_world->CreateBody(&bodyDef);

        b2PolygonShape box;
        box.SetAsBox(in_dimensions.x / 2.f, in_dimensions.y / 2.f);

        if (in_static)
        {
            m_fixture = m_body->CreateFixture(&box, 0.0f);
        }
        else
        {
            // Define the dynamic body fixture.
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &box;

            // Set the box density to be non-zero, so it will be dynamic.
            fixtureDef.density = 100.0f;

            // Override the default friction.
            fixtureDef.friction = 0.3f;

            // Add the shape to the body.
            m_fixture = m_body->CreateFixture(&fixtureDef);
        }
        
    }

    void PhysicsBody::InitAsCircle(const Vector2& in_position, float in_radius, b2World* in_world, bool in_static)
    {
        b2BodyDef bodyDef;
        bodyDef.type = in_static ? b2_staticBody : b2_dynamicBody;
        bodyDef.position.Set(in_position.x, in_position.y);

        m_body = in_world->CreateBody(&bodyDef);

        b2CircleShape circle;
        circle.m_radius = in_radius;

        m_fixture = m_body->CreateFixture(&circle, in_static ? 0.0f : 1.0f);
    }

    void PhysicsBody::SetPixelToMetersRatio(float in_ratio)
    {
        m_pixelToMeterRatio = in_ratio;
    }

    void PhysicsBody::SetRestitution(float in_restitution)
    {
        m_fixture->SetRestitution(in_restitution);
    }

    Vector2 PhysicsBody::GetPosition()
    {
        return Vector2(m_body->GetPosition().x, m_body->GetPosition().y);
    }

    float PhysicsBody::GetAngle()
    {
        return m_body->GetAngle();
    }

    Vector2 PhysicsBody::GetLinearVel()
    {
        return Vector2(m_body->GetLinearVelocity().x, m_body->GetLinearVelocity().y);
    }

    void PhysicsBody::SetLinearVel(const Vector2& in_vel)
    {
        m_body->SetLinearVelocity(b2Vec2(in_vel.x, in_vel.y));
    }

    void PhysicsBody::SetAngularVel(float in_vel)
    {
        m_body->SetAngularVelocity(in_vel);
    }

    void PhysicsBody::ApplyForce(const Vector2& in_force)
    {
        m_body->ApplyForceToCenter(b2Vec2(in_force.x, in_force.y), true);
    }

    void PhysicsBody::ApplyLinearImpulse(const Vector2& in_impulse)
    {
        m_body->ApplyLinearImpulse(b2Vec2(in_impulse.x, in_impulse.y), m_body->GetWorldCenter(), true);
    }

    void PhysicsBody::SetTransform(const Vector2& in_transform, float in_angle)
    {
        m_body->SetTransform(b2Vec2(in_transform.x / m_pixelToMeterRatio, in_transform.y / m_pixelToMeterRatio), in_angle);
    }

    void PhysicsBody::SetFriction(float in_friction)
    {
        m_fixture->SetFriction(in_friction);
    }

}



