//******************************************************************************
//
// MIDITrail / MTFirstPersonCam
//
// First-person camera class.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include "MTSceneComponent11.h"
#include "MTViewParamMap.h"
#include "DIKeyCtrl.h"
#include "DIMouseCtrl.h"
#include "DXCamera.h"
#include "SMIDILib.h"
#include "MTNoteDesign11.h"
#include "MTGamePadCtrl.h"

using namespace SMIDILib;


//******************************************************************************
// Parameter definitions
//******************************************************************************
#define MTFIRSTPERSONCAM_CAMVECTOR_LIMIT  (1000000.0f)


//******************************************************************************
// First-person camera class
//******************************************************************************
class MTFirstPersonCam
{
public:

	enum MTProgressDirection {
		DirNone,
		DirX,
		DirY,
		DirZ
	};

public:

	MTFirstPersonCam();
	virtual ~MTFirstPersonCam();

	int Clear();

	int Initialize(HWND hWnd, const TCHAR* pSceneName, SMSeqData* pSeqData);

	// Position
	void SetPosition(DirectX::SimpleMath::Vector3 camVector);
	void GetPosition(DirectX::SimpleMath::Vector3* pCamVector);

	// Direction (spherical coordinates)
	//   phi:   azimuthal angle on XZ plane (+X=0, +Z=90)
	//   theta: polar angle from +Y axis (+Y=0, XZ plane=90)
	void SetDirection(float phi, float theta);
	void GetDirection(float* pPhi, float* pTheta);

	// Mouse camera mode
	void SetMouseCamMode(bool isEnable);

	// Auto-roll mode
	void SetAutoRollMode(bool isEnable);
	void SwitchAutoRollDir();

	// Per-frame update: input processing, position tracking, view matrix
	int Update(const MTSceneUpdateContext& ctx);

	// Get view/projection matrices for rendering
	void GetViewProjection(float aspect,
	                       DirectX::SimpleMath::Matrix* pView,
	                       DirectX::SimpleMath::Matrix* pProj);

	// Get current roll angle (manual roll + auto roll combined)
	float GetRollAngle();


	// Reset
	void Reset();

	// Roll angle get/set
	float GetManualRollAngle();
	float GetAutoRollVelocity();
	void SetManualRollAngle(float rollAngle);
	void SetAutoRollVelocity(float rollVelocity);

	// Progress direction
	void SetProgressDirection(MTProgressDirection dir);

	// ViewParam support (used by MTSceneBase11)
	void GetViewParam(MTViewParamMap* pParamMap);
	void SetViewParam(MTViewParamMap* pParamMap);

private:

	DXCamera m_Camera;
	DirectX::SimpleMath::Vector3 m_CamVector;
	float m_CamDirPhi;
	float m_CamDirTheta;
	MTProgressDirection m_ProgressDirection;

	DIKeyCtrl m_DIKeyCtrl;
	DIMouseCtrl m_DIMouseCtrl;
	MTGamePadCtrl m_GamePadCtrl;
	bool m_IsMouseCamMode;
	bool m_IsAutoRollMode;
	HWND m_hWnd;
	MTNoteDesign11 m_NoteDesign;

	float m_VelocityFB;
	float m_VelocityLR;
	float m_VelocityUD;
	float m_VelocityPT;
	float m_AcceleRate;

	float m_RollAngle;
	float m_VelocityAutoRoll;
	float m_VelocityManualRoll;

	unsigned long m_PrevTime;
	unsigned long m_DeltaTime;

	unsigned long m_PrevTickTime;
	unsigned long m_CurTickTime;

	int _TransformEyeDirection(int dX, int dY);
	int _TransformCamPosition();
	int _TransformRolling(int dW);
	int _SetCamPosition();
	int _ClipCursor(bool isClip);
	void _CalcDeltaTime();
	int _LoadConfFile(const TCHAR* pSceneName);
	void _ClipCamVector(DirectX::SimpleMath::Vector3* pVector);
};
