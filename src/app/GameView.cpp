#include "GameView.h"
#include "Emitter.h"

GameView::GameView()
{

}

GameView::~GameView()
{

}

void GameView::OnShow()
{
    m_fireFX = AddEmitter("FireFX.pex");
    m_fireFX->SetPosition(OScreenCenterXf,OScreen.y);
    m_fireFX->Start();
}

void GameView::OnHide()
{
}
