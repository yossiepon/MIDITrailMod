//******************************************************************************
//
// MIDITrail / MTSceneBase11
//
// DX11 scene common base class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <vector>
#include "IMTScene11.h"
#include "MTSceneComponent11.h"
#include "MTFirstPersonCam.h"
class MTConfFile;

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

	// Per-frame update: camera + context + components
	int Update() override;

	// Sequencer message: common handling + scene-specific hook
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
	int  Rewind() override;

	// Key-value parameters
	int  SetParam(const char* pKey, const char* pValue) override;
	const char* GetParam(const char* pKey) override;

	// Camera accessor (for DXRenderer11)
	MTFirstPersonCam* GetCamera() override { return &m_Camera; }

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
	bool          m_isMonitoringActive = false;

	//----------------------------------------------------------------------
	// Shared components
	//----------------------------------------------------------------------

	MTFirstPersonCam  m_Camera;

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

	// Registered managed components (for automatic Update/Reset dispatch)
	std::vector<IMTSceneManagedComponent*> m_ManagedComponents;

	//----------------------------------------------------------------------
	// Virtual hooks for subclass customization
	//----------------------------------------------------------------------

	// Compute scene-specific default viewpoint.
	virtual void _ComputeDefaultViewParam(MTViewParamMap* pParamMap) = 0;

	// Viewpoint compensation amount (Ring/3D use TimeIndicator position).
	virtual float _GetViewpointCompensation() const { return 0.0f; }

	// Register a managed component for automatic Update/Reset dispatch.
	void _RegisterComponent(IMTSceneManagedComponent* pComponent);

	// Scene-specific update logic beyond registered components (optional).
	virtual int _UpdateComponents(const MTSceneUpdateContext& ctx) { return 0; }

	// Scene-specific sequencer message handling (called after common handling).
	virtual int _OnRecvSequencerMsg(unsigned long param1, unsigned long param2) { return 0; }

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
