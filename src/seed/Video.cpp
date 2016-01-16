#include "Video.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    Video::Video()
        : m_dimensions(Vector2(320.f, 240.f))
        , m_playRate(1.0)
        , m_loops(false)
        , m_volume(1.f)
        , m_videoTarget(nullptr)
        , m_videoPlayer(nullptr)
    {
    }

    Video::~Video()
    {

    }

    Node* Video::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const
    {
        Video* newNode = in_pool.alloc<Video>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        DuplicateChildren(newNode, in_pool, in_pooledNodes);
        return newNode;
    }

    Node* Video::Duplicate() const
    {
        Video* newNode = new Video();
        Copy(newNode);
        DuplicateChildren(newNode);
        return newNode;
    }

    void Video::Copy(Node* in_copy) const
    {
        Node::Copy(in_copy);
        Video* copy = (Video*)in_copy;

        copy->m_source = m_source;
        copy->m_dimensions = m_dimensions;
        copy->m_playRate = m_playRate;
        copy->m_loops = m_loops;
        copy->m_volume = m_volume;

        if (IsPlaying())
        {
            copy->Play();
        }
    }
    
    void Video::Update()
    {
        Node::Update();
        if (m_videoPlayer)
        {
            m_videoPlayer->update();
        }
    }

    void Video::Render(Matrix* in_parentMatrix, float in_parentAlpha)
    {
        if (!m_visible)
        {
            return;
        }

        // generate our matrix
        const Vector2& dimensions = m_dimensions;
        Matrix transform = Matrix::Identity;
        if (m_videoTarget)
        {
            Rect rect;
            rect.x = transform.Translation().x - dimensions.x * .5f;
            rect.y = transform.Translation().y - dimensions.y * .5f;
            rect.z = dimensions.x;
            rect.w = dimensions.y;
            rect = ORectFit(rect, m_videoTarget->getSize());
            auto scale = onut::min(rect.z / m_videoTarget->getSizef().x, rect.w / m_videoTarget->getSizef().y);
            transform *= Matrix::CreateScale(scale, scale, 1.f);
        }
        transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (in_parentMatrix)
        {
            transform = transform * *in_parentMatrix;
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // render the video
        if (m_videoTarget)
        {
            OSpriteBatch->drawSprite(m_videoTarget, transform, GetColor(), Vector2(.5f));
        }

        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
    }

    void Video::SetSource(const string& source)
    {
        if (m_source == source) return;
        m_source = source;
    }

    void Video::Play()
    {
        // Create a render target that the video will render to
        if (!m_videoTarget)
        {
            m_videoTarget = OTexture::createRenderTarget();
        }

        // Initialize our video using our rendering target
        if (m_videoPlayer)
        {
            delete m_videoPlayer;
            m_videoPlayer = nullptr;
        }
        m_videoPlayer = OPlayer::Create();
        m_videoPlayer->init(m_videoTarget);

        std::string sourceFile = OContentManager->find(m_source);
        if (sourceFile.empty()) sourceFile = m_source;
        m_videoPlayer->setSource(sourceFile);
        m_videoPlayer->setLoop(m_loops);
        m_videoPlayer->setVolume(m_volume);
        m_videoPlayer->setPlayRate(m_playRate);
        m_videoPlayer->play();
    }

    void Video::Stop()
    {
        if (m_videoPlayer)
        {
            delete m_videoPlayer;
            m_videoPlayer = nullptr;
        }
    }

    void Video::Pause()
    {
        if (m_videoPlayer)
        {
            m_videoPlayer->pause();
        }
    }

    void Video::Resume()
    {
        if (m_videoPlayer)
        {
            m_videoPlayer->play();
        }
    }

    bool Video::IsPlaying() const
    {
        if (!m_videoPlayer) return false;
        return m_videoPlayer->isPlaying();
    }

    const string& Video::GetSource() const
    {
        return m_source;
    }

    float Video::GetWidth() const
    {
        return m_dimensions.get().x;
    }

    float Video::GetHeight() const
    {
        return m_dimensions.get().y;
    }

    void Video::SetPlayRate(double in_rate)
    {
        m_playRate = in_rate;
        if (m_videoPlayer)
        {
            m_videoPlayer->setPlayRate(in_rate);
        }
    }

    double Video::GetPlayRate() const
    {
        return m_playRate;
    }

    void Video::SetLoops(bool in_loops)
    {
        m_loops = in_loops;
        if (m_videoPlayer)
        {
            m_videoPlayer->setLoop(m_loops);
        }
    }

    bool Video::GetLoops() const
    {
        return m_loops;
    }

    void Video::SetDimensions(const Vector2& in_dimensions)
    {
        m_dimensions = in_dimensions;
    }

    const Vector2& Video::GetDimensions() const
    {
        return m_dimensions;
    }

    OAnim2& Video::GetDimensionsAnim()
    {
        return m_dimensions;
    }

    void Video::SetVolume(float in_volume)
    {
        m_volume = in_volume;
        if (m_videoPlayer)
        {
            m_videoPlayer->setVolume(m_volume);
        }
    }

    float Video::GetVolume() const
    {
        return m_volume;
    }

    OAnimf& Video::GetVolumeAnim()
    {
        return m_volume;
    }

    tinyxml2::XMLElement* Video::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        xmlNode->SetName("Video");
        xmlNode->SetAttribute("source", GetSource().c_str());
        xmlNode->SetAttribute("loops", m_loops);
        xmlNode->SetAttribute("width", m_dimensions.get().x);
        xmlNode->SetAttribute("height", m_dimensions.get().y);
        xmlNode->SetAttribute("volume", m_volume.get());
        xmlNode->SetAttribute("playRate", m_playRate);

        return xmlNode;
    }

    void Video::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        const char* szSource = in_xmlNode->Attribute("source");
        if (szSource)
        {
            m_source = szSource;
        }

        bool loops = GetLoops();
        in_xmlNode->QueryBoolAttribute("loops", &loops);
        SetLoops(loops);

        Vector2 dimensions = GetDimensions();
        in_xmlNode->QueryFloatAttribute("width", &dimensions.x);
        in_xmlNode->QueryFloatAttribute("height", &dimensions.y);
        SetDimensions(dimensions);

        float volume = GetVolume();
        in_xmlNode->QueryFloatAttribute("volume", &volume);
        SetVolume(volume);

        double rate = GetPlayRate();
        in_xmlNode->QueryDoubleAttribute("playRate", &rate);
        SetPlayRate(rate);
    }
}
