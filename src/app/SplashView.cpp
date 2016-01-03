#include "seed/Node.h"
#include "SplashView.h"

void SplashView::OnShow()
{
    // Load from file
    Load("../../assets/views/SplashView.xml");

    // Start some animations
    auto pBackground = FindNode("Background");
    auto pBackgroundNuts = FindNode("BackgroundNuts");
    auto pNuts = FindNode("Nuts");
    auto pForeground = FindNode("Foreground");
    auto pNear = FindNode("Near");
    auto pTitle = FindNode("Title");

    // Zoom in and fade out layers
    pBackground->GetScaleAnim().startFromCurrent(Vector2(1.2f, 1.2f), 3.f, OEaseIn);
    pBackground->GetColorAnim().startFromCurrent(Color(pBackground->GetColor(), 0.f), 3.f, OEaseIn);

    pBackgroundNuts->GetScaleAnim().startFromCurrent(Vector2(1.25f, 1.25f), 3.f, OEaseIn);
    pBackgroundNuts->GetColorAnim().startFromCurrent(Color(pBackground->GetColor(), 0.f), 3.f, OEaseIn);

    pNuts->GetScaleAnim().startFromCurrent(Vector2(1.5f, 1.5f), 3.f, OEaseIn);
    pNuts->GetPositionAnim().startFromCurrentKeyframed(
    {
        OAnimWait(pNuts->GetPosition(), 3.f),
        {pNuts->GetPosition() + Vector2(0, 128.f), 1.f, OBounceOut}
    });

    pForeground->GetScaleAnim().startFromCurrent(Vector2(2.f, 2.f), 3.f, OEaseIn);
    pForeground->GetColorAnim().startFromCurrent(Color(pForeground->GetColor(), 0.f), 3.f, OEaseIn);

    pNear->GetScaleAnim().startFromCurrent(Vector2(3.f, 3.f), 3.f, OEaseIn);
    pNear->GetColorAnim().startFromCurrent(Color(pNear->GetColor(), 0.f), 3.f, OEaseIn);

    // Fade in title
    pTitle->GetColorAnim().startFromCurrentKeyframed(
    {
        OAnimWait(pTitle->GetColor(), 2.f),
        {Color(pTitle->GetColor(), 1.f), 1.5f, OEaseBoth}
    });

    // Start a timer then switch to main menu
    m_timer.start(5.f, [this]
    {
        SendCommand(seed::eAppCommand::SWITCH_VIEW, "StartView");
    });
}

void SplashView::OnUpdate()
{
    if (OJustPressed(OINPUT_MOUSEB1))
    {
        m_timer.stop();
        SendCommand(seed::eAppCommand::SWITCH_VIEW, "StartView");
    }
}
