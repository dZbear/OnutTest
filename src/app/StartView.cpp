#include "StartView.h"
#include "Sprite.h"
#include "SpriteString.h"

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
}

void StartView::OnHide()
{
}
