#include "GameView.h"
#include "Emitter.h"
#include "Sprite.h"
#include "SpriteString.h"
#include "SoundEmitter.h"
#include "MusicEmitter.h"
#include "Video.h"

GameView::GameView()
{
}

GameView::~GameView()
{

}

void GameView::OnShow()
{
    SetSize(Vector2(OScreenWf, OScreenHf));

    m_dude = CreateSpriteWithSpriteAnim("baltAnims.xml", "idle_down");
    AddNode(m_dude);
    m_idleAnim = "idle_down";
    m_dude->SetPosition(OScreenCenterXf, OScreenCenterYf);
    m_dude->SetScale(Vector2(5, 5));
    m_dude->SetFilter(onut::SpriteBatch::eFiltering::Nearest);

    m_sndEmitter = CreateRandomSoundEmitter({ "sound1.wav", "sound2.wav", "sound3.wav", "sound4.wav", "sound5.wav" });
    m_sndEmitter->SetPositionBasedBalance(true);
    m_sndEmitter->SetPositionBasedVolume(true);
    m_dude->Attach(m_sndEmitter);

    m_testFX = CreateEmitter("test2.pex");
    m_testFX->Start();
    m_testFX->SetPosition(0, -10);
    AddNode(m_testFX,m_dude, -1);

    seed::SpriteString* testString = CreateSpriteString("cartoon.fnt");
    testString->SetCaption("BALT GUY");
    testString->SetScale(Vector2(.05f, .05f));
    testString->SetPosition(0, 4);
    AddNode(testString, m_dude);

    m_musicEmitter = CreateMusicEmitter();
    AddNode(m_musicEmitter);
    m_musicEmitter->Play("music.mp3", .7f, 3.f);

    m_video = CreateVideo();
    m_video->SetVolume(0.f);
    m_video->SetSource("https://download.blender.org/durian/trailer/sintel_trailer-720p.mp4");
    m_video->Play();
    AddNode(m_video, m_dude, -2);
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

    if (OJustPressed(OINPUT_0))
    {
        m_sndEmitter->Play();
    }

    if (OJustPressed(OINPUT_1))
    {
        static string s_toPlay = "fight.mp3";
        m_musicEmitter->Play(s_toPlay, .7f, 3.f);

        s_toPlay = s_toPlay == "fight.mp3" ? "music.mp3" : "fight.mp3";
    }
}
