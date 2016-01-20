#include "StartView.h"
#include "Sprite.h"
#include "SpriteString.h"
#include "Button.h"

StartView::StartView()
{

}

StartView::~StartView()
{

}

void StartView::OnShow()
{
    seed::Sprite* spriteButton = CreateSprite("button.png");
    spriteButton->SetPosition(Vector2(OScreenCenterXf, OScreenCenterYf - 70.f));
    AddNode(spriteButton);

    seed::SpriteString* label = CreateSpriteString("cartoon.fnt");
    AddNode(label, spriteButton);
    label->SetCaption("START GAME");
    label->SetColor(Color(1.f,.5f,0.f));
    label->SetScale( Vector2(.5f, .5f) );
    seed::Button* startButton = AddButton(spriteButton, "start");
    
    spriteButton = CreateSprite("button.png");
    spriteButton->SetPosition(Vector2(OScreenCenterXf, OScreenCenterYf + 70.f));
    AddNode(spriteButton);

    label = CreateSpriteString("cartoon.fnt");
    label->SetCaption("PHYSICS TEST");
    label->SetColor(Color(1.f, .5f, 0.f));
    label->SetScale(Vector2(.5f, .5f));
    AddNode(label, spriteButton);
    AddButton(spriteButton, "physics");


    spriteButton = CreateSprite("button.png");
    spriteButton->SetPosition(Vector2(OScreenCenterXf, OScreenCenterYf + 220.f));
    AddNode(spriteButton);
    
    label = CreateSpriteString("cartoon.fnt");
    label->SetCaption("QUIT");
    label->SetColor(Color(1.f, .5f, 0.f));
    label->SetScale(Vector2(.5f, .5f));
    AddNode(label, spriteButton);
    AddButton(spriteButton, "quit");

    FocusButton(startButton);
    SetDefaultFocusedButton(startButton);
}

void StartView::OnHide()
{
}

void StartView::OnButtonDown(seed::Button* in_button)
{
    seed::Sprite* buttonSprite = in_button->GetSprite();
    buttonSprite->GetScaleAnim().startKeyframed(
        Vector2(1, 1),
        {
            { Vector2(1.2f, 1.2f), .2f, OSpringOut },
        });

}

void StartView::OnButtonUp(seed::Button* in_button)
{
    seed::Sprite* buttonSprite = in_button->GetSprite();
    buttonSprite->GetScaleAnim().startFromCurrent(Vector2(1, 1), .2f, OEaseOut);
}

void StartView::OnButtonFocused(seed::Button* in_button, int in_playerIndex)
{
    seed::Sprite* buttonSprite = in_button->GetSprite();
    buttonSprite->GetColorAnim().startFromCurrent(Color(1.0f, 0, 0), .2f);
}

void StartView::OnButtonFocusLost(seed::Button* in_button, int in_playerIndex)
{
    seed::Sprite* buttonSprite = in_button->GetSprite();
    buttonSprite->GetColorAnim().startFromCurrent(Color(1.0f, 1.0f, 1.0f), .2f);
}


bool StartView::OnCommand(const string& in_cmd)
{
    if (in_cmd == "start")
    {
        SendCommand(seed::eAppCommand::SWITCH_VIEW, "GameView");
        return true;
    }
    else if (in_cmd == "physics")
    {
        SendCommand(seed::eAppCommand::SWITCH_VIEW, "PhysicsView");
        return true;
    }
    else if (in_cmd == "quit")
    {
        exit(0);
        return true;
    }
    return false;
}
