#include "MusicEmitter.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    MusicEmitter::MusicEmitter()
        : m_volume(1.f)
        , m_currentTrack(nullptr)
        , m_lastTrack(nullptr)
        , m_loops(true)
    {
    }

    MusicEmitter::~MusicEmitter()
    {

    }

    Node* MusicEmitter::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes)
    {
        MusicEmitter* newNode = in_pool.alloc<MusicEmitter>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        return newNode;
    }

    void MusicEmitter::Copy(Node* in_copy)
    {
        Node::Copy(in_copy);
        MusicEmitter* copy = (MusicEmitter*)in_copy;
        
        copy->SetLoops(GetLoops());
        if (m_currentTrack)
        {
            copy->Play(m_file, m_volume.get());
        }
    }
    
    void MusicEmitter::Update()
    {
        Node::Update();
        UpdateVolume();
        UpdateLooping();
    }


    void MusicEmitter::Play(const string& in_mp3File, float in_volume, float in_fadeTime)
    {
        if (m_file == in_mp3File)
        {
            return; // music already playing
        }
        m_file = in_mp3File;

        // check if music is already playing
        if (m_currentTrack)
        {
            if (m_lastTrack)
            {
                m_lastTrack->stop();
                delete m_lastTrack;
                m_lastTrack = nullptr;
            }
            m_lastTrack = m_currentTrack;
            m_currentTrack = nullptr;
               
            m_lastTrackVolume = m_volume.get();
            if (in_fadeTime > 0)
            {
                // cross fade happening
                m_lastTrackVolume.startFromCurrent(0, in_fadeTime);
            }
            else
            {
                // no cross fade, just shut it down
                m_lastTrack->stop();
                delete m_lastTrack;
                m_lastTrack = nullptr;
            }
        }

        m_volume = in_volume;
        if (in_fadeTime > 0.f)
        {
            m_volume.start(0.f, in_volume, in_fadeTime);
        }

        
        m_currentTrack = new onut::Music();
        UpdateVolume();
        m_currentTrack->play(in_mp3File);
    }

    void MusicEmitter::Pause()
    {
        if (m_currentTrack)
        {
            m_currentTrack->pause();
        }
    }

    void MusicEmitter::Resume()
    {
        if (m_currentTrack)
        {
            m_currentTrack->resume();
        }
    }

    void MusicEmitter::Stop(float in_fadeTime)
    {
        if (m_lastTrack)
        {
            m_lastTrack->stop();
            delete m_lastTrack;
            m_lastTrack = nullptr;
        }

        if (m_currentTrack)
        {
            m_lastTrack = m_currentTrack;
            m_currentTrack = nullptr;
        }

        if (in_fadeTime > 0.f)
        {
            m_lastTrackVolume = m_volume.get();
            m_lastTrackVolume.startFromCurrent(0, in_fadeTime);
        }
        else
        {
            m_lastTrack->stop();
            delete m_lastTrack;
            m_lastTrack = nullptr;
        }
    }

    const string& MusicEmitter::GetFile() const
    {
        return m_file;
    }

    tinyxml2::XMLElement* MusicEmitter::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("MusicEmitter");
        xmlNode->SetAttribute("file", GetFile().c_str());
        xmlNode->SetAttribute("volume", m_volume.get());
        xmlNode->SetAttribute("loops", m_loops);

        return xmlNode;
    }

    void MusicEmitter::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        m_file = in_xmlNode->Attribute("file");
        float volume;
        in_xmlNode->QueryFloatAttribute("volume", &volume);
        m_volume = volume;

        in_xmlNode->QueryBoolAttribute("loops", &m_loops);

    }

    void MusicEmitter::SetVolume(float in_volume, float in_fadeTime)
    {
        if (in_fadeTime > 0)
        {
            m_volume.startFromCurrent(in_volume, in_fadeTime);
        }
        else
        {
            m_volume = in_volume;
        }
        UpdateVolume();
    }

    float MusicEmitter::GetVolume()
    {
        return m_volume.get();
    }

    void MusicEmitter::UpdateVolume()
    {
        if (m_currentTrack)
        {
            m_currentTrack->setVolume(m_volume.get());
        }

        if (m_lastTrack)
        {
            m_lastTrack->setVolume(m_lastTrackVolume.get());
            if (m_lastTrackVolume.get() <= 0.f)
            {
                // done fading out
                m_lastTrack->stop();
                delete m_lastTrack;
                m_lastTrack = nullptr;
            }
        }
    }

    void MusicEmitter::SetLoops(bool in_loops)
    {
        m_loops = in_loops;
    }

    bool MusicEmitter::GetLoops()
    {
        return m_loops;
    }

    void MusicEmitter::UpdateLooping()
    {
        if (!m_loops)
        {
            return;
        }

        if (m_currentTrack)
        {
            if (m_currentTrack->isDone())
            {
                OLog("Looping music");
                m_currentTrack->play(m_file);
            }
        }
    }
}
