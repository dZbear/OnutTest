#include "PhysicsView.h"
#include "Sprite.h"
#include "PhysicsBody.h"

PhysicsView::PhysicsView()
    : m_dude(nullptr)
{

}

PhysicsView::~PhysicsView()
{

}

void PhysicsView::OnShow()
{
    // init a physical world

    GetPhysics().Init(Vector2(0, 1), 16.f);

    seed::Sprite* ground = CreateSprite("ground.png");
    ground->SetPosition(Vector2(OScreenCenterXf, OScreenHf - ground->GetHeight() / 2.f));
    AddNode(ground);

    CreateBoxPhysicsForNode(ground, true);

    m_box = CreateSprite("box.png");
    m_box->SetPosition(Vector2(OScreenCenterXf - 200.f, 100.f));
    AddNode(m_box);

    seed::PhysicsBody* phBox = CreateBoxPhysicsForNode(m_box, false);
    phBox->SetRestitution(.2f);
    phBox->SetFriction(0.0001f);

    m_dude = CreateSpriteWithSpriteAnim("baltAnims.xml", "idle_side");
    m_dude->SetPosition(Vector2(OScreenCenterXf, 100.f) + Vector2(100, 0));
    m_dude->SetScale(Vector2(5, 5));
    m_dude->SetFilter(onut::SpriteBatch::eFiltering::Nearest);
    AddNode(m_dude);

    seed::PhysicsBody* phDudeBox = CreateBoxPhysicsForNode(m_dude, false);
    phDudeBox->SetRestitution(.05f);
}

void PhysicsView::OnHide()
{

}

void PhysicsView::OnUpdate()
{
    const float MOVE_VEL = 500.f;
    if (OPressed(OINPUT_LEFT))
    {
        // move left!
        GetPhysicsForNode(m_dude)->ApplyLinearImpulse(Vector2(-MOVE_VEL, 0));
        m_dude->SetSpriteAnim("run_side");
        m_dude->SetFlippedH(false);
    }

    if (OPressed(OINPUT_RIGHT))
    {
        // move right!
        GetPhysicsForNode(m_dude)->ApplyLinearImpulse(Vector2(MOVE_VEL, 0));
        m_dude->SetSpriteAnim("run_side");
        m_dude->SetFlippedH(true);
    }

    if (std::abs(GetPhysicsForNode(m_dude)->GetLinearVel().x) < 0.0001f )
    {
        m_dude->SetSpriteAnim("idle_side");
    }

    if (m_dude->GetPosition().y > OScreenHf)
    {
        GetPhysicsForNode(m_dude)->SetTransform(Vector2(OScreenCenterXf, 100.f) + Vector2(100, 0), 0);
        GetPhysicsForNode(m_dude)->SetLinearVel(Vector2(0, 0));
        GetPhysicsForNode(m_dude)->SetAngularVel(0);
    }

    if (m_box->GetPosition().y > OScreenHf)
    {
        GetPhysicsForNode(m_box)->SetTransform(Vector2(OScreenCenterXf - 200.f, 100.f), 0);
        GetPhysicsForNode(m_box)->SetLinearVel(Vector2(0, 0));
        GetPhysicsForNode(m_box)->SetAngularVel(0);
    }
}