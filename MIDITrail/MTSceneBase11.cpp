//******************************************************************************
//
// MIDITrail / MTSceneBase11
//
// DX11 scene common base class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "MTSceneBase11.h"
#include "MTConfFile.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTSceneBase11::MTSceneBase11()
{
}

MTSceneBase11::~MTSceneBase11()
{
	MTSceneBase11::Release();
}

//******************************************************************************
// Release shared resources
//******************************************************************************
void MTSceneBase11::Release()
{
	// Phase 2: Release shared components
	// m_Dashboard.Release();
	// m_Stars.Release();
	// m_BackgroundImage.Release();
	m_pDevice  = nullptr;
	m_pContext = nullptr;
}

//******************************************************************************
// Window click (identical across all DX9 scenes)
//******************************************************************************
int MTSceneBase11::OnWindowClicked(
		unsigned int button,
		WPARAM wParam,
		LPARAM lParam
	)
{
	// Left button: toggle mouse camera mode
	if (button == WM_LBUTTONDOWN) {
		m_IsMouseCamMode = !m_IsMouseCamMode;
		m_Camera.SetMouseCamMode(m_IsMouseCamMode);
	}
	// Middle button: toggle auto-roll mode
	else if (button == WM_MBUTTONDOWN) {
		m_IsAutoRollMode = !m_IsAutoRollMode;
		m_Camera.SetAutoRollMode(m_IsAutoRollMode);
		if (m_IsAutoRollMode) {
			m_Camera.SwitchAutoRollDir();
		}
	}

	return 0;
}

//******************************************************************************
// Viewpoint management
//******************************************************************************
void MTSceneBase11::GetDefaultViewParam(MTViewParamMap* pParamMap)
{
	_ComputeDefaultViewParam(pParamMap);
}

void MTSceneBase11::GetViewParam(MTViewParamMap* pParamMap)
{
	m_Camera.GetViewParam(pParamMap);

	// Compensate for playback position so stored viewpoint is relative
	if (m_Traits.viewpointCompensation) {
		float compensation = _GetViewpointCompensation();
		(*pParamMap)["X"] -= compensation;
	}
}

void MTSceneBase11::SetViewParam(MTViewParamMap* pParamMap)
{
	MTViewParamMap paramMap = *pParamMap;

	// Apply playback position offset
	if (m_Traits.viewpointCompensation) {
		float compensation = _GetViewpointCompensation();
		paramMap["X"] += compensation;
	}

	m_Camera.SetViewParam(&paramMap);

	// Restore mode flags from the viewpoint
	m_IsMouseCamMode = false;
	m_Camera.SetMouseCamMode(m_IsMouseCamMode);

	if (paramMap.count("AutoRollVelocity") && paramMap["AutoRollVelocity"] != 0.0f) {
		m_IsAutoRollMode = true;
	}
	else {
		m_IsAutoRollMode = false;
	}
	m_Camera.SetAutoRollMode(m_IsAutoRollMode);
}

void MTSceneBase11::MoveToStaticViewpoint(unsigned long viewpointNo)
{
	MTViewParamMap paramMap;

	switch (viewpointNo) {
		case 1:
			GetDefaultViewParam(&paramMap);
			break;
		case 2:
			paramMap = m_Viewpoint2;
			break;
		case 3:
			paramMap = m_Viewpoint3;
			break;
		default:
			return;
	}

	SetViewParam(&paramMap);
}

void MTSceneBase11::ResetViewpoint()
{
	MTViewParamMap paramMap;
	GetDefaultViewParam(&paramMap);
	SetViewParam(&paramMap);
}

void MTSceneBase11::Rewind()
{
	_Reset();
	SetViewParam(&m_ViewParamMap);
}

//******************************************************************************
// Reset (subclasses extend by calling base then resetting own components)
//******************************************************************************
void MTSceneBase11::_Reset()
{
	// Phase 2: Reset shared components
	// m_Dashboard.Reset();
	m_Camera.Reset();
	// m_PitchBend.Reset();
}

//******************************************************************************
// Key-value parameters
//******************************************************************************
void MTSceneBase11::SetParam(const char* pKey, const char* pValue)
{
	if (pKey == nullptr || pValue == nullptr) return;
	m_Params[std::string(pKey)] = std::string(pValue);
}

const char* MTSceneBase11::GetParam(const char* pKey)
{
	if (pKey == nullptr) return "";
	auto it = m_Params.find(std::string(pKey));
	if (it == m_Params.end()) return "";
	return it->second.c_str();
}

//******************************************************************************
// Configuration loading
//******************************************************************************
void MTSceneBase11::_LoadConf()
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(GetName());
	if (result != 0) return;

	_LoadConfViewpoint(&confFile, 2, &m_Viewpoint2);
	_LoadConfViewpoint(&confFile, 3, &m_Viewpoint3);

	// Phase 2: Load background color from color palette
}

void MTSceneBase11::_LoadConfViewpoint(
		MTConfFile* pConfFile,
		unsigned long viewpointNo,
		MTViewParamMap* pParamMap
	)
{
	int result = 0;
	TCHAR section[32] = {0};

	_stprintf_s(section, 32, _T("Viewpoint-%d"), viewpointNo);

	result = pConfFile->SetCurSection(section);
	if (result != 0) return;

	float x = 0.0f, y = 0.0f, z = 0.0f;
	float phi = 0.0f, theta = 0.0f;
	float manualRoll = 0.0f, autoRollVel = 0.0f;

	pConfFile->GetFloat(_T("X"), &x, 0.0f);
	pConfFile->GetFloat(_T("Y"), &y, 0.0f);
	pConfFile->GetFloat(_T("Z"), &z, 0.0f);
	pConfFile->GetFloat(_T("Phi"), &phi, 0.0f);
	pConfFile->GetFloat(_T("Theta"), &theta, 0.0f);
	pConfFile->GetFloat(_T("ManualRollAngle"), &manualRoll, 0.0f);
	pConfFile->GetFloat(_T("AutoRollVelocity"), &autoRollVel, 0.0f);

	(*pParamMap)["X"] = x;
	(*pParamMap)["Y"] = y;
	(*pParamMap)["Z"] = z;
	(*pParamMap)["Phi"] = phi;
	(*pParamMap)["Theta"] = theta;
	(*pParamMap)["ManualRollAngle"] = manualRoll;
	(*pParamMap)["AutoRollVelocity"] = autoRollVel;
}
