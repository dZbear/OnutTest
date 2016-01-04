#pragma once

#include "Node.h"

namespace seed
{
    class Video : public Node
    {
    public:

        Video();
        virtual ~Video();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes);
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void        Play( const string& in_source );
        void        Pause();
        void        Resume();
        void        SetPlayRate(double in_rate);
        double      GetPlayRate();
        void        SetLoops(bool in_loops);
        bool        GetLoops();
        void        SetDimensions(const Vector2& in_dimensions);
        Vector2&    GetDimensions();

        const string&       GetSource() const;

        // only to be used by the seed sdk
        virtual void        Update() override;
        virtual void        Render(Matrix* in_parentMatrix = nullptr, float in_parentAlpha = 1.f) override;

    protected:

        virtual void        Copy(Node* in_copy);

        string              m_source;
        Vector2             m_dimensions;
        double              m_playRate;
        bool                m_loops;

        OTexture*           m_videoTarget;
        OPlayer*            m_videoPlayer;
    };
}