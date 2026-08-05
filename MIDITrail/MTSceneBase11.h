//******************************************************************************
//
// MIDITrail / MTSceneBase11
//
// DX11 scene common base class.
// Owns components shared by all scenes: camera, dashboard, stars,
// background image, pitch bend. Absorbs the code that was duplicated
// across 7 independent DX9 scene classes.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "IMTScene11.h"
#include "MTSceneComponent11.h"
#include "MTFirstPersonCam.h"
class MTConfFile;
// Phase 2: DX11 component headers
// #include "MTDashboard11.h"
// #include "MTStars11.h"
// #include "MTBackgroundImage11.h"
// #include "MTNotePitchBend.h"

//******************************************************************************
// Camera progress direction
//******************************************************************************
enum MTCameraProgressDir {
	MTCameraDirX,       // PianoRoll 3D/2D: horizontal scroll
	MTCameraDirY,       // Rain: vertical (notes fall past stationary camera)
	MTCameraDirNone,    // Ring: no axis-aligned progress
};

//******************************************************************************
// Scene traits (set by subclass constructors)
//******************************************************************************
struct MTSceneTraits {
	MTCameraProgressDir cameraDir              = MTCameraDirX;
	bool                cameraTracksPlayback   = true;
	bool                lightEnabled           = true;
	int                 lightCount             = 1;
	bool                viewpointCompensation  = false;
};

//******************************************************************************
// DX11 scene common base
//******************************************************************************
class MTSceneBase11 : public IMTScene11 {
public:

	MTSceneBase11();
	virtual ~MTSceneBase11();

	//----------------------------------------------------------------------
	// IMTScene11: common implementations
	//----------------------------------------------------------------------

	bool IsLive() const override { return m_IsLive; }

	// Per-frame update: camera + context + components
	int Update() override;

	// Sequencer message: common handling (MsgPlayTime etc.)
	int OnRecvSequencerMsg(unsigned long param1, unsigned long param2) override;

	// Release shared resources
	void Release() override;

	// Input (identical across all DX9 scenes)
	int  OnWindowClicked(
				unsigned int button,
				WPARAM wParam,
				LPARAM lParam
			) override;

	// Viewpoint management (shared logic, compensation via virtual hook)
	void GetDefaultViewParam(MTViewParamMap* pParamMap) override;
	void GetViewParam(MTViewParamMap* pParamMap) override;
	void SetViewParam(MTViewParamMap* pParamMap) override;
	void MoveToStaticViewpoint(unsigned long viewpointNo) override;
	void ResetViewpoint() override;
	void Rewind() override;

	// Key-value parameters
	void SetParam(const char* pKey, const char* pValue) override;
	const char* GetParam(const char* pKey) override;

	// Camera accessor (for DXRenderer11)
	MTFirstPersonCam* GetCamera() { return &m_Camera; }

	// Background color
	const float* GetBGColor() const override { return m_BGColor; }
	void SetBGColor(unsigned long argb);

	// Live input: default no-ops (overridden by scenes that support live)
	void SetNoteOnLive(unsigned char, unsigned char,
	                   unsigned char, unsigned char) override {}
	void SetNoteOffLive(unsigned char, unsigned char,
	                    unsigned char) override {}
	void AllNoteOffLive() override {}
	void AllNoteOffOnChLive(unsigned char, unsigned char) override {}

protected:

	//----------------------------------------------------------------------
	// Scene properties (set by subclass constructors)
	//----------------------------------------------------------------------

	MTSceneTraits m_Traits;
	bool          m_IsLive = false;

	//----------------------------------------------------------------------
	// Shared components
	//----------------------------------------------------------------------

	MTFirstPersonCam  m_Camera;
	// Phase 2: DX11 components
	// MTDashboard11     m_Dashboard;
	// MTStars11         m_Stars;
	// MTBackgroundImage11 m_BackgroundImage;
	// MTNotePitchBend   m_PitchBend;

	//----------------------------------------------------------------------
	// Shared state
	//----------------------------------------------------------------------

	unsigned long    m_CurTickTime    = 0;
	unsigned long    m_PlayTimeMSec   = 0;
	bool             m_IsMouseCamMode = false;
	bool             m_IsAutoRollMode = false;
	bool             m_IsSkipping     = false;
	MTViewParamMap   m_ViewParamMap;
	MTViewParamMap   m_Viewpoint2;
	MTViewParamMap   m_Viewpoint3;

	// Device references (not owned)
	ID3D11Device*        m_pDevice  = nullptr;
	ID3D11DeviceContext* m_pContext = nullptr;

	// Background color (RGBA float[4])
	float m_BGColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//----------------------------------------------------------------------
	// Virtual hooks for subclass customization
	//----------------------------------------------------------------------

	// Compute scene-specific default viewpoint.
	virtual void _ComputeDefaultViewParam(MTViewParamMap* pParamMap) = 0;

	// Viewpoint compensation amount (Ring/3D use TimeIndicator position).
	virtual float _GetViewpointCompensation() const { return 0.0f; }

	// Scene-specific component update (called after camera update with full context).
	virtual int _UpdateComponents(const MTSceneUpdateContext& ctx) = 0;

	// Scene-specific component reset. Subclass calls base, then resets own.
	virtual void _Reset();

	// Draw scene-specific components (called between BackgroundImage and Dashboard).
	// Lighting is managed internally by the scene (via SceneTraits).
	virtual int _DrawSceneComponents(
					ID3D11DeviceContext* pContext,
					const DirectX::SimpleMath::Matrix& viewProj,
					float rollAngle,
					const DirectX::SimpleMath::Vector3& camPos
				) = 0;

	//----------------------------------------------------------------------
	// Shared utilities
	//----------------------------------------------------------------------

	void _LoadConf();
	void _LoadConfViewpoint(MTConfFile* pConfFile,
	                        unsigned long viewpointNo,
	                        MTViewParamMap* pParamMap);

private:

	// Key-value parameter store
	typedef std::map<std::string, std::string> ParamDictionary;
	ParamDictionary m_Params;
};
