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
    m_dude = AddSpriteWithAnim("baltAnims.xml", "idle");
    m_dude->SetPosition(OScreenCenterXf, OScreenCenterYf);
    m_dude->SetScale(Vector2(5, 5));
    m_dude->SetFilter(onut::SpriteBatch::eFiltering::Nearest);

    m_fireFX = AddEmitter("FireFX.pex");
    m_fireFX->SetPosition(OScreenCenterXf,(float)OScreen.y);
    m_fireFX->Start();
}

void GameView::OnHide()
{
}
