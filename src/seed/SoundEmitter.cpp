#include "SoundEmitter.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    SoundEmitter::SoundEmitter()
        : m_balance(0.f)
        , m_volume(1.f)
        , m_volumeFactor(1.f)
        , m_pitch(1.f)
        , m_positionBasedBalance(false)
        , m_positionBasedVolume(false)
        , m_soundInstance(nullptr)
        , m_isCue(false)
        , m_looping(false)
    {
    }

    SoundEmitter::~SoundEmitter()
    {

    }

    Node* SoundEmitter::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes)
    {
        SoundEmitter* newNode = in_pool.alloc<SoundEmitter>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        return newNode;
    }

    void SoundEmitter::Copy(Node* in_copy)
    {
        Node::Copy(in_copy);
        SoundEmitter* copy = (SoundEmitter*)in_copy;
        
        copy->Init(GetFile());
        copy->SetVolume(GetVolume());
        copy->SetPitch(GetPitch());
        copy->SetBalance(GetBalance());
        copy->SetPositionBasedBalance(GetPositionBasedBalance());
        copy->SetPositionBasedVolume(GetPositionBasedVolume());

        if (m_soundInstance)
        {
            if (m_soundInstance->isPlaying())
            {
                copy->Play(m_looping);
            }
        }
    }

    void SoundEmitter::Init(const string& in_file)
    {
        m_file = in_file;
        string extension = onut::toLower(onut::getExtension(m_file));
        if (extension == "xml")
        {
            m_isCue = true;
        }
        else
        {
            m_isCue = false;
            m_soundInstance = OCreateSoundInstance(m_file.c_str());
        }
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


    void SoundEmitter::Play(bool in_loop)
    {
        if (m_isCue)
        {
            OPlaySoundCue(m_file.c_str(), m_volume * m_volumeFactor, m_balance);
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
            Init(m_file);
            if (!m_soundInstance)
            {
                OLogE("Error calling SoundEmitter::Play for file named '" + m_file + "'");
                return;
            }
        }

        if (m_soundInstance->isPlaying())
        {
            m_soundInstance->stop();
        }

        m_soundInstance->setLoop(in_loop);
        m_looping = in_loop;
        UpdateSoundParams();
        m_soundInstance->play();
    }

    void SoundEmitter::Stop()
    {
        if (m_soundInstance)
        {
            m_soundInstance->stop();
        }
        m_looping = false;
    }

    const string& SoundEmitter::GetFile() const
    {
        return m_file;
    }

    tinyxml2::XMLElement* SoundEmitter::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("SoundEmitter");
        xmlNode->SetAttribute("file", GetFile().c_str());
        xmlNode->SetAttribute("volume", m_volume);
        xmlNode->SetAttribute("pitch", m_pitch);
        xmlNode->SetAttribute("balance", m_balance);
        xmlNode->SetAttribute("positionBasedBalance", m_positionBasedBalance);
        xmlNode->SetAttribute("positionBasedVolume", m_positionBasedVolume);

        return xmlNode;
    }

    void SoundEmitter::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        m_file = in_xmlNode->Attribute("file");
        in_xmlNode->QueryFloatAttribute("volume", &m_volume);
        in_xmlNode->QueryFloatAttribute("pitch", &m_pitch);
        in_xmlNode->QueryFloatAttribute("balance", &m_balance);
        in_xmlNode->QueryBoolAttribute("positionBasedBalance", &m_positionBasedBalance);
        in_xmlNode->QueryBoolAttribute("positionBasedVolume", &m_positionBasedVolume);
    }

    void SoundEmitter::SetVolume(float in_volume)
    {
        m_volume = in_volume;
        UpdateSoundParams();
    }

    float SoundEmitter::GetVolume()
    {
        return m_volume;
    }

    void SoundEmitter::SetBalance(float in_balance)
    {
        m_balance = in_balance;
        UpdateSoundParams();
    }

    float SoundEmitter::GetBalance()
    {
        return m_balance;
    }

    void SoundEmitter::SetPitch(float in_pitch)
    {
        m_pitch = in_pitch;
        UpdateSoundParams();
    }

    float SoundEmitter::GetPitch()
    {
        return m_pitch;
    }

    void SoundEmitter::SetPositionBasedBalance(bool in_positionBased)
    {
        m_positionBasedBalance = in_positionBased;
    }

    bool SoundEmitter::GetPositionBasedBalance()
    {
        return m_positionBasedBalance;
    }

    void SoundEmitter::SetPositionBasedVolume(bool in_positionBased)
    {
        m_positionBasedVolume = in_positionBased;
    }

    bool SoundEmitter::GetPositionBasedVolume()
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
