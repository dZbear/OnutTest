#include "Video.h"
#include "onut.h"
#include "tinyxml2.h"

namespace seed
{
    Video::Video()
        : m_videoTarget(nullptr)
        , m_videoPlayer(nullptr)
        , m_playRate(1.0)
        , m_loops(false)
    {
    }

    Video::~Video()
    {

    }

    Node* Video::Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes)
    {
        Video* newNode = in_pool.alloc<Video>();
        Copy(newNode);
        in_pooledNodes.push_back(newNode);
        return newNode;
    }

    void Video::Copy(Node* in_copy)
    {
        Node::Copy(in_copy);
        Video* copy = (Video*)in_copy;
        
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
        Matrix transform = Matrix::Identity;
        transform *= Matrix::CreateScale(m_scale.get().x, m_scale.get().y, 1.f);
        transform *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle));
        transform *= Matrix::CreateTranslation(m_position.get().x, m_position.get().y, 0);

        if (in_parentMatrix)
        {
            transform = transform * *in_parentMatrix;
        }

        // render bg children
        RenderChildren(m_bgChildren, &transform, m_color.get().w * in_parentAlpha);

        // temp until i figure this out
        m_dimensions = m_videoTarget->getSizef();

        // render the video
        Rect rect;
        rect.x = transform.Translation().x;
        rect.y = transform.Translation().y;
        rect.z = m_dimensions.x;
        rect.w = m_dimensions.y;

        OSB->drawRect(m_videoTarget, rect);


        // render fg children
        RenderChildren(m_fgChildren, &transform, m_color.get().w * in_parentAlpha);
    }


    void Video::Play(const string& in_source)
    {
        if (m_source == in_source)
        {
            return; // video already playing
        }
        m_source = in_source;

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
        m_videoPlayer->setSource(m_source);
        m_videoPlayer->play();
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

    const string& Video::GetSource() const
    {
        return m_source;
    }

    void Video::SetPlayRate(double in_rate)
    {
        m_playRate = in_rate;
        if (m_videoPlayer)
        {
            m_videoPlayer->setPlayRate(in_rate);
        }
    }

    double Video::GetPlayRate()
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

    bool Video::GetLoops()
    {
        return m_loops;
    }

    void Video::SetDimensions(const Vector2& in_dimensions)
    {
        m_dimensions = in_dimensions;
    }

    Vector2& Video::GetDimensions()
    {
        return m_dimensions;
    }


    tinyxml2::XMLElement* Video::Serialize(tinyxml2::XMLDocument* in_xmlDoc) const
    {
        tinyxml2::XMLElement *xmlNode = Node::Serialize(in_xmlDoc);

        //xmlNode->SetName("Video");
        //xmlNode->SetAttribute("file", GetFile().c_str());
        //xmlNode->SetAttribute("volume", m_volume.get());
        //xmlNode->SetAttribute("loops", m_loops);

        return xmlNode;
    }

    void Video::Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode)
    {
        Node::Deserialize(view, in_xmlNode);

        //m_file = in_xmlNode->Attribute("file");
        //float volume;
        //in_xmlNode->QueryFloatAttribute("volume", &volume);
        //m_volume = volume;

        //in_xmlNode->QueryBoolAttribute("loops", &m_loops);

    }
}
