#pragma once
#include "View.h"

class MusicEmitter;
class SoundEmitter;
class Emitter;
class Video;
class GameView : public seed::View
{
public:

    GameView();
    virtual ~GameView();

	virtual void OnShow();
	virtual void OnHide();
    virtual void OnUpdate();
	
private:

    seed::Sprite*       m_dude;
    seed::Emitter*      m_testFX;
    string              m_idleAnim;

    seed::SoundEmitter* m_sndEmitter;
    seed::MusicEmitter* m_musicEmitter;
    seed::Video*        m_video;

};