#include "GameView.h"
#include "Emitter.h"
#include "Sprite.h"

GameView::GameView()
{

}

GameView::~GameView()
{

}

void GameView::OnShow()
{
    m_dude = AddSpriteWithAnim("baltAnims.xml", "idle_down");
    m_idleAnim = "idle_down";
    m_dude->SetPosition(OScreenCenterXf, OScreenCenterYf);
    m_dude->SetScale(Vector2(5, 5));
    m_dude->SetFilter(onut::SpriteBatch::eFiltering::Nearest);

    m_fireFX = AddEmitter("FireFX.pex");
    m_fireFX->SetPosition(OScreenCenterXf,(float)OScreen.y);
    //m_fireFX->Start();
}

void GameView::OnHide()
{
}

void GameView::OnUpdate()
{
    Vector2 vel;
    string newAnim;
    bool flipped = false;
    if (OPressed(OINPUT_LEFT))
    {
        // move left!
        newAnim = "run_side";
        m_idleAnim = "idle_side";
        vel.x = -1;
    }

    if (OPressed(OINPUT_RIGHT))
    {
        // move right!
        newAnim = "run_side";
        m_idleAnim = "idle_side";
        flipped = true;
        vel.x = 1;
    }

    if (OPressed(OINPUT_UP))
    {
        // move up!
        newAnim = "run_up";
        m_idleAnim = "idle_up";
        vel.y = -1;
    }

    if (OPressed(OINPUT_DOWN))
    {
        // move down!
        newAnim = "run_down";
        m_idleAnim = "idle_down";
        vel.y = 1;
    }

    if (newAnim.length())
    {
        m_dude->SetAnim(newAnim);
        m_dude->SetFlipped(flipped, false);
    }
    else
    {
        m_dude->SetAnim(m_idleAnim);
    }
    
    const float speed = 5.f;
    Vector2 pos = m_dude->GetPosition();
    pos += vel * speed;
    m_dude->SetPosition(pos);
}
