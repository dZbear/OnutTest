#pragma once
#ifdef WITH_BOX_2D
    #include <Box2D/Box2D.h>
#endif

namespace seed
{
    class PhysicsBody
    {
    public:

        PhysicsBody();
        virtual ~PhysicsBody();

        void    InitAsBox(const Vector2& in_position, const Vector2& in_dimensions, b2World* in_world, bool in_static);
        void    InitAsCircle(const Vector2& in_position, float in_radius, b2World* in_world, bool in_static);
        void    SetRestitution(float in_restitution);
        void    SetFriction(float in_friction);
        void    SetPixelToMetersRatio(float in_ratio);

        // move things around
        void    SetTransform(const Vector2& in_transform, float in_angle);
        void    SetLinearVel(const Vector2& in_vel);
        void    SetAngularVel(float in_vel);
        void    ApplyForce(const Vector2& in_force);
        void    ApplyLinearImpulse(const Vector2& in_impulse);


        Vector2     GetPosition();
        float       GetAngle();
        Vector2     GetLinearVel();

    private:

        b2Body*     m_body;
        b2Fixture*  m_fixture;
        float       m_pixelToMeterRatio;
        
    };
}
