#include "GameView.h"
#include "Emitter.h"
#include "Sprite.h"
#include "SpriteString.h"

GameView::GameView()
{
}

GameView::~GameView()
{

}

void GameView::OnShow()
{
    SetSize(Vector2(OScreenWf, OScreenHf));

    m_dude = AddSpriteWithSpriteAnim("baltAnims.xml", "idle_down");
    m_idleAnim = "idle_down";
    m_dude->SetPosition(OScreenCenterXf, OScreenCenterYf);
    m_dude->SetScale(Vector2(5, 5));
    m_dude->SetFilter(onut::SpriteBatch::eFiltering::Nearest);


    m_testFX = AddEmitter("test2.pex", m_dude, -1);
    m_testFX->Start();
    m_testFX->SetPosition(0, -10);

    seed::SpriteString* testString = AddSpriteString("cartoon.fnt", m_dude);
    testString->SetCaption("BALT GUY");
    testString->SetScale(Vector2(.05f, .05f));
    testString->SetPosition(0, 4);
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
        m_dude->SetSpriteAnim(newAnim);
        m_dude->SetFlipped(flipped, false);
    }
    else
    {
        m_dude->SetSpriteAnim(m_idleAnim);
    }
    
    const float speed = 5.f;
    Vector2 pos = m_dude->GetPosition();
    pos += vel * speed;
    m_dude->SetPosition(pos);

    if (OJustPressed(OINPUT_SPACE))
    {
        // duplicate our dude
        seed::Node* newDude = DuplicateNode(m_dude);
        AddNode(newDude);

    }
}
