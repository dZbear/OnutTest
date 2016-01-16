#pragma once

#include "Node.h"

namespace seed
{
    class MusicEmitter : public Node
    {
    public:

        MusicEmitter();
        virtual ~MusicEmitter();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*                   Duplicate() const;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void    Play(const string& in_mp3File, float in_volume = 1.f, float in_fadeTime = 0.f);
        void    Play(float in_volume = 1.f, float in_fadeTime = 0.f);
        void    Stop(float in_fadeTime = 0.f);
        void    Pause();
        void    Resume();
        void    SetVolume(float in_volume);
        void    SetVolumeWithFade(float in_volume, float in_fadeTime = 0.f);
        float   GetVolume();
        void    SetLoops(bool in_loops);
        bool    GetLoops();

        void                SetSource(const std::string& source);
        const string&       GetSource() const;

        // only to be used by the seed sdk
        virtual void        Update() override;

    protected:

        virtual void        Copy(Node* in_copy) const;

        string              m_source;
        string              m_playingSource;
        OAnim<float>        m_volume;
        OAnim<float>        m_lastTrackVolume;
        bool                m_loops;

        onut::Music*        m_currentTrack;
        onut::Music*        m_lastTrack;


        void                UpdateVolume();
        void                UpdateLooping();
    };
}