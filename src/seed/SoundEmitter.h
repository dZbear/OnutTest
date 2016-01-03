#pragma once

#include "Node.h"

namespace seed
{
    class SoundEmitter : public Node
    {
    public:

        SoundEmitter();
        virtual ~SoundEmitter();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes);
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void    Init(const string& in_file);    // can be .wav or .xml for sound cues
        void    Init(const vector<string>& in_randomFiles);    // vector of .wav to randomize from
        void    Play(bool in_loop=false);
        void    Stop();
        void    SetVolume(float in_volume);
        float   GetVolume();
        void    SetBalance(float in_balance);
        float   GetBalance();
        void    SetPitch(float in_pitch);
        float   GetPitch();
        void    SetPositionBasedBalance(bool in_positionBased);
        bool    GetPositionBasedBalance();
        void    SetPositionBasedVolume(bool in_positionBased);
        bool    GetPositionBasedVolume();

        const string&       GetFile() const;

        // only to be used by the seed sdk
        virtual void        Update() override;

    protected:

        virtual void        Copy(Node* in_copy);

        string              m_file;
        bool                m_isCue;
        bool                m_looping;
        float               m_volume;
        float               m_volumeFactor;
        float               m_balance;
        float               m_pitch;
        bool                m_positionBasedBalance;
        bool                m_positionBasedVolume;
        OSoundInstance*     m_soundInstance;
        vector<string>      m_randomFiles;

        void                UpdateSoundParams();
    };
}