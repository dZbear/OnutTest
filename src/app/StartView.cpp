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
    seed::Sprite* spriteButton = AddSprite("button.png");
    spriteButton->SetPosition(OScreenCenterXf, OScreenCenterYf);

    seed::SpriteString* label = AddSpriteString("cartoon.fnt", spriteButton);
    label->SetCaption("Start Game");
    label->SetColor(Color(1.f,.5f,0.f));
    label->SetScale( Vector2(.5f, .5f) );

    AddButton(spriteButton, "start");
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

void StartView::OnCommand(const string& in_cmd)
{
    if (in_cmd == "start")
    {
        OLog("start");
    }
}
