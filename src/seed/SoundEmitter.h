#pragma once

#include "Node.h"
#include <memory>

namespace seed
{
    class SoundEmitter : public Node
    {
    public:

        SoundEmitter();
        virtual ~SoundEmitter();

        virtual Node*                   Duplicate(onut::Pool<true>& in_pool, NodeVect& in_pooledNodes) const;
        virtual Node*                   Duplicate() const;
        virtual tinyxml2::XMLElement*   Serialize(tinyxml2::XMLDocument* in_xmlDoc) const;
        virtual void                    Deserialize(View* view, tinyxml2::XMLElement* in_xmlNode);

        void    Init(const string& in_file);    // can be .wav or .xml for sound cues
        void    Init(const vector<string>& in_randomFiles);    // vector of .wav to randomize from
        void    Play();
        void    Stop();
        void    SetVolume(float in_volume);
        float   GetVolume() const;
        OAnimf& GetVolumeAnim();
        void    SetBalance(float in_balance);
        float   GetBalance() const;
        void    SetPitch(float in_pitch);
        float   GetPitch() const;
        void    SetLoops(bool in_loops);
        bool    GetLoops() const;
        void    SetPositionBased(bool in_positionBased);
        bool    GetPositionBased() const;
        void    SetPositionBasedBalance(bool in_positionBased);
        bool    GetPositionBasedBalance() const;
        void    SetPositionBasedVolume(bool in_positionBased);
        bool    GetPositionBasedVolume() const;

        const string&           GetSource() const;
        vector<string>&         GetRandomFiles();
        const vector<string>&   GetRandomFiles() const;

        // only to be used by the seed sdk
        virtual void        Update() override;

    protected:

        virtual void        Copy(Node* in_copy) const;

        string              m_source;
        bool                m_isCue;
        bool                m_loops;
        OAnimf              m_volume;
        float               m_volumeFactor;
        float               m_balance;
        OAnimf              m_pitch;
        bool                m_positionBasedBalance;
        bool                m_positionBasedVolume;
        OSoundInstance*     m_soundInstance;
        vector<string>      m_randomFiles;

        void                UpdateSoundParams();
    };
}