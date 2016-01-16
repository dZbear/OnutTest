#pragma once

#include "Node.h"

namespace seed
{
    class Video : public Node
    {
    public:

        Video();
        virtual ~Video();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*                   Duplicate() const;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void            Play();
        void            Stop();
        void            Pause();
        void            Resume();
        void            SetPlayRate(double in_rate);
        double          GetPlayRate() const;
        void            SetLoops(bool in_loops);
        bool            GetLoops() const;
        void            SetDimensions(const Vector2& in_dimensions);
        const Vector2&  GetDimensions() const;
        OAnim2&         GetDimensionsAnim();
        void            SetVolume(float in_volume);
        float           GetVolume() const;
        OAnimf&         GetVolumeAnim();
        void            SetSource(const string& source);
        const string&   GetSource() const;
        bool            IsPlaying() const;

        virtual float   GetWidth() const override;
        virtual float   GetHeight() const override;

        // only to be used by the seed sdk
        virtual void    Update() override;
        virtual void    Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;

    protected:

        virtual void    Copy(Node* in_copy) const;

        string          m_source;
        OAnim2          m_dimensions;
        double          m_playRate;
        bool            m_loops;
        OAnimf          m_volume;

        OTexture*       m_videoTarget;
        OPlayer*        m_videoPlayer;
    };
}
