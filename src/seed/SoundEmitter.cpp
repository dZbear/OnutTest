#include "SoundEmitter.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    SoundEmitter::SoundEmitter()
        : m_isCue(false)
        , m_loops(false)
        , m_volume(1.f)
        , m_volumeFactor(1.f)
        , m_balance(0.f)
        , m_pitch(1.f)
        , m_positionBasedBalance(false)
        , m_positionBasedVolume(false)
        , m_soundInstance(nullptr)
    {
    }

    SoundEmitter::~SoundEmitter()
    {
        delete m_soundInstance;
    }

    Node* SoundEmitter::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        SoundEmitter* newNode = in_pool.alloc<SoundEmitter>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* SoundEmitter::Duplicate() const
    {
        SoundEmitter* newNode = new SoundEmitter();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void SoundEmitter::Copy(Node* in_copy) const
    {
        Node::Copy(in_copy);
        SoundEmitter* copy = (SoundEmitter*)in_copy;
        
        copy->Init(GetSource());
        copy->m_loops = m_loops;
        copy->m_volume = m_volume;
        copy->m_balance = m_balance;
        copy->m_pitch = m_pitch;
        copy->m_positionBasedBalance = m_positionBasedBalance;
        copy->m_positionBasedVolume = m_positionBasedVolume;
        copy->m_randomFiles = m_randomFiles;

        if (m_soundInstance)
        {
            if (m_soundInstance->isPlaying())
            {
                copy->Play();
            }
        }
    }

    void SoundEmitter::Init(const string& in_file)
    {
        if (m_soundInstance)
        {
            delete m_soundInstance;
            m_soundInstance = nullptr;
        }
        m_source = in_file;
        string extension = onut::toLower(onut::getExtension(m_source));
        if (extension == "cue")
        {
            m_isCue = true;
        }
        else
        {
            m_isCue = false;
            m_soundInstance = OCreateSoundInstance(m_source.c_str());
        }
        UpdateSoundParams();
    }

    void SoundEmitter::Init(const vector<string>& in_randomFiles)
    {
        m_randomFiles = in_randomFiles;
        m_isCue = false;
        int randomSound = onut::randi(0, (int)in_randomFiles.size() - 1);
        m_soundInstance = OCreateSoundInstance(m_randomFiles[randomSound].c_str());
    }

    void SoundEmitter::Update()
    {
        Node::Update();

        if (m_positionBasedBalance)
        {
            const float absX = GetAbsolutePosition().x;
            const float percent = absX / OScreenWf;

            SetBalance(2.f * percent - 1.f);
        }

        if (m_positionBasedVolume)
        {
            Vector2 pos = GetAbsolutePosition();
            const Vector2 screen = Vector2(OScreenWf, OScreenHf);
            const float maxScreenDim = screen.y;

            pos.Clamp(Vector2(-screen.x, -screen.y), Vector2(screen.x * 2.f, screen.y * 2.f));


            float horizontalAtt = 0;
            float verticalAtt = 0;

            if (pos.x < 0)
            {
                horizontalAtt = std::abs(pos.x) / maxScreenDim;
            }
            else if (pos.x > screen.x)
            {
                horizontalAtt = (std::abs(pos.x) - screen.x) / maxScreenDim;
            }

            if (pos.y < 0)
            {
                verticalAtt = std::abs(pos.y) / maxScreenDim;
            }
            else if (pos.y > screen.y)
            {
                horizontalAtt = (std::abs(pos.y) - screen.y) / maxScreenDim;
            }

            m_volumeFactor = 1.f - onut::max(horizontalAtt, verticalAtt);
            m_volumeFactor = onut::max(0.f, m_volumeFactor);
            UpdateSoundParams();
        }
    }


    void SoundEmitter::Play()
    {
        if (m_isCue)
        {
            OPlaySoundCue(m_source.c_str(), m_volume * m_volumeFactor, m_balance);
            return;
        }
        else if (m_randomFiles.size())
        {
            int randomSound = onut::randi(0, (int)m_randomFiles.size() - 1);
            delete m_soundInstance;
            m_soundInstance = OCreateSoundInstance(m_randomFiles[randomSound].c_str());
            UpdateSoundParams();
        }

        if (!m_soundInstance && !m_isCue)
        {
            Init(m_source);
            if (!m_soundInstance)
            {
                OLogE("Error calling SoundEmitter::Play for file named '" + m_source + "'");
                return;
            }
        }

        if (m_soundInstance->isPlaying())
        {
            m_soundInstance->stop();
        }

        m_soundInstance->setLoop(m_loops);
        UpdateSoundParams();
        m_soundInstance->play();
    }

    void SoundEmitter::Stop()
    {
        if (m_soundInstance)
        {
            m_soundInstance->stop();
        }
        m_loops = false;
    }

    const string& SoundEmitter::GetSource() const
    {
        return m_source;
    }

    vector<string>& SoundEmitter::GetRandomFiles()
    {
        return m_randomFiles;
    }

    const vector<string>& SoundEmitter::GetRandomFiles() const
    {
        return m_randomFiles;
    }

    tinyxml2::XMLElement* SoundEmitter::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("SoundEmitter");
        xmlNode->SetAttribute("source", GetSource().c_str());
        xmlNode->SetAttribute("volume", m_volume.get());
        xmlNode->SetAttribute("pitch", m_pitch.get());
        xmlNode->SetAttribute("balance", m_balance);
        xmlNode->SetAttribute("loops", m_loops);
        xmlNode->SetAttribute("positionBasedBalance", m_positionBasedBalance);
        xmlNode->SetAttribute("positionBasedVolume", m_positionBasedVolume);

        return xmlNode;
    }

    void SoundEmitter::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        const char* szSource = in_xmlNode->Attribute("source");
        if (szSource)
        {
            m_source = szSource;
        }

        float volume = GetVolume();
        in_xmlNode->QueryFloatAttribute("volume", &volume);
        SetVolume(volume);

        float pitch = GetPitch();
        in_xmlNode->QueryFloatAttribute("pitch", &pitch);
        SetPitch(pitch);

        float balance = GetBalance();
        in_xmlNode->QueryFloatAttribute("balance", &balance);
        SetBalance(balance);

        bool loops = GetLoops();
        in_xmlNode->QueryBoolAttribute("loops", &loops);
        SetLoops(loops);

        bool positionBasedBalance = GetPositionBasedBalance();
        in_xmlNode->QueryBoolAttribute("positionBasedBalance", &positionBasedBalance);
        SetPositionBasedBalance(positionBasedBalance);

        bool positionBasedVolume = GetPositionBasedVolume();
        in_xmlNode->QueryBoolAttribute("positionBasedVolume", &positionBasedVolume);
        SetPositionBasedVolume(positionBasedVolume);
    }

    void SoundEmitter::SetVolume(float in_volume)
    {
        m_volume = in_volume;
        UpdateSoundParams();
    }

    float SoundEmitter::GetVolume() const
    {
        return m_volume;
    }

    OAnimf& SoundEmitter::GetVolumeAnim()
    {
        return m_volume;
    }

    void SoundEmitter::SetLoops(bool in_loops)
    {
        m_loops = in_loops;
    }

    bool SoundEmitter::GetLoops() const
    {
        return m_loops;
    }

    void SoundEmitter::SetBalance(float in_balance)
    {
        m_balance = in_balance;
        UpdateSoundParams();
    }

    float SoundEmitter::GetBalance() const
    {
        return m_balance;
    }

    void SoundEmitter::SetPitch(float in_pitch)
    {
        m_pitch = in_pitch;
        UpdateSoundParams();
    }

    float SoundEmitter::GetPitch() const
    {
        return m_pitch;
    }

    void SoundEmitter::SetPositionBased(bool in_positionBased)
    {
        SetPositionBasedBalance(in_positionBased);
        SetPositionBasedVolume(in_positionBased);
    }

    bool SoundEmitter::GetPositionBased() const
    {
        return GetPositionBasedBalance() || GetPositionBasedVolume();
    }

    void SoundEmitter::SetPositionBasedBalance(bool in_positionBased)
    {
        m_positionBasedBalance = in_positionBased;
    }

    bool SoundEmitter::GetPositionBasedBalance() const
    {
        return m_positionBasedBalance;
    }

    void SoundEmitter::SetPositionBasedVolume(bool in_positionBased)
    {
        m_positionBasedVolume = in_positionBased;
    }

    bool SoundEmitter::GetPositionBasedVolume() const
    {
        return m_positionBasedVolume;
    }

    void SoundEmitter::UpdateSoundParams()
    {
        if (!m_isCue)
        {
            if (m_soundInstance)
            {
                m_soundInstance->setPitch(m_pitch);
                m_soundInstance->setBalance(m_balance);
                m_soundInstance->setVolume(m_volume * m_volumeFactor);
            }
        }
    }
}
