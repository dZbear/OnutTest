#include "StartView.h"

StartView::StartView()
{

}

StartView::~StartView()
{

}

void StartView::OnCreate()
{
	m_testSprite = AddSprite("dasLogo.png");
	m_testSprite->SetPosition(Vector2(OScreen.x / 2.f, OScreen.y / 2.f));

	seed::Sprite* fgChild = AddSprite("dasLogo.png");
	fgChild->SetPosition(-m_testSprite->GetWidth() / 4.f, m_testSprite->GetHeight() / 4.f);
	fgChild->SetScale(Vector2(.5f,.5f));
	fgChild->GetPositionAnim().start(fgChild->GetPosition(), fgChild->GetPosition() + Vector2(10, 0), 1.f, onut::TweenType::EASE_BOTH, onut::LoopType::PINGPONG_LOOP);
	m_testSprite->Attach(fgChild);

	fgChild = AddSprite("dasLogo.png");
	fgChild->SetPosition(m_testSprite->GetWidth() / 4.f, m_testSprite->GetHeight() / 4.f);
	fgChild->SetScale(Vector2(.5f, .5f));
	fgChild->GetPositionAnim().start(fgChild->GetPosition(), fgChild->GetPosition() + Vector2(-10, 0), 1.f, onut::TweenType::EASE_BOTH, onut::LoopType::PINGPONG_LOOP);
	m_testSprite->Attach(fgChild);

	seed::Sprite* bgChild = AddSprite("dasLogo.png");
	bgChild->SetPosition(-m_testSprite->GetWidth() / 4.f, -m_testSprite->GetHeight() / 4.f);
	bgChild->SetScale(Vector2(.5f, .5f));
	bgChild->GetPositionAnim().start(bgChild->GetPosition(), bgChild->GetPosition() + Vector2(0, 10), .5f, onut::TweenType::EASE_BOTH, onut::LoopType::PINGPONG_LOOP);
	m_testSprite->Attach(bgChild, -1);

	bgChild = AddSprite("dasLogo.png");
	bgChild->SetPosition(m_testSprite->GetWidth() / 4.f, -m_testSprite->GetHeight() / 4.f);
	bgChild->SetScale(Vector2(.5f, .5f));
	bgChild->GetPositionAnim().start(bgChild->GetPosition(), bgChild->GetPosition() + Vector2(0, -10), .5f, onut::TweenType::EASE_BOTH, onut::LoopType::PINGPONG_LOOP);
	m_testSprite->Attach(bgChild, -1);

	m_testSprite->GetAngleAnim().start(0.f, 359.f, 10.f, onut::TweenType::LINEAR, onut::LoopType::LOOP);
	m_testSprite->GetScaleAnim().start(Vector2(1, 1), Vector2(2, 2), 5.f, onut::TweenType::EASE_BOTH, onut::LoopType::PINGPONG_LOOP);
}

void StartView::OnShow()
{
}

void StartView::OnHide()
{
}
