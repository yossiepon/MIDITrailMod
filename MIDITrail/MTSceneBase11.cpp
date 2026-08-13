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

#include "stdafx.h"
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "MTSceneBase11.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTColorPalette.h"
#include "SMMsgParser.h"

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
// Sequencer message: common handling
//******************************************************************************
int MTSceneBase11::OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	parser.Parse(param1, param2);

	if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		m_CurTickTime = parser.GetPlayTickTime();
		m_PlayTimeMSec = parser.GetPlayTimeMSec();
	}
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		SetNoteOnLive(parser.GetPortNo(), parser.GetChNo(),
		              parser.GetNoteNo(), parser.GetVelocity());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOff) {
		SetNoteOffLive(parser.GetPortNo(), parser.GetChNo(),
		               parser.GetNoteNo());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgAllNoteOff) {
		AllNoteOffOnChLive(parser.GetPortNo(), parser.GetChNo());
	}

	// Scene-specific handling
	result = _OnRecvSequencerMsg(param1, param2);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Per-frame update
//******************************************************************************
int MTSceneBase11::Update()
{
	int result = 0;

	// Build the context (from saved members)
	MTSceneUpdateContext ctx;
	ctx.curTickTime = m_CurTickTime;
	ctx.playTimeMSec = m_PlayTimeMSec;
	ctx.liveTimeMSec = m_IsLive ? timeGetTime() : 0;

	// Update camera
	result = m_Camera.Update(ctx);
	if (result != 0) goto EXIT;

	// Reflect camera position/rotation into the context
	m_Camera.GetPosition(&ctx.camPos);
	ctx.rollAngle = m_Camera.GetRollAngle();

	// Update registered components
	for (auto* pComp : m_ManagedComponents) {
		result = pComp->Update(ctx);
		if (result != 0) goto EXIT;
	}

	// Additional scene-specific update
	result = _UpdateComponents(ctx);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release shared resources
//******************************************************************************
void MTSceneBase11::Release()
{
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

	// Save for Rewind restoration
	if (pParamMap != &m_ViewParamMap) {
		m_ViewParamMap = *pParamMap;
	}

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

int MTSceneBase11::Rewind()
{
	_Reset();
	SetViewParam(&m_ViewParamMap);
	return 0;
}

//******************************************************************************
// Reset (subclasses extend by calling base then resetting own components)
//******************************************************************************
void MTSceneBase11::SetBGColor(unsigned long argb)
{
	m_BGColor[0] = ((argb >> 16) & 0xFF) / 255.0f;
	m_BGColor[1] = ((argb >>  8) & 0xFF) / 255.0f;
	m_BGColor[2] = ((argb >>  0) & 0xFF) / 255.0f;
	m_BGColor[3] = ((argb >> 24) & 0xFF) / 255.0f;
}

void MTSceneBase11::_Reset()
{
	m_CurTickTime = 0;
	m_PlayTimeMSec = 0;
	m_IsSkipping = false;
	m_Camera.Reset();

	for (auto* pComp : m_ManagedComponents) {
		pComp->Reset();
	}
}

//******************************************************************************
// Register managed component
//******************************************************************************
void MTSceneBase11::_RegisterComponent(IMTSceneManagedComponent* pComponent)
{
	m_ManagedComponents.push_back(pComponent);
}

//******************************************************************************
// Key-value parameters
//******************************************************************************
int MTSceneBase11::SetParam(const char* pKey, const char* pValue)
{
	if (pKey == nullptr || pValue == nullptr) return 0;
	m_Params[std::string(pKey)] = std::string(pValue);
	return 0;
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

	// Load background color
	{
		MTColorConf colorConf;
		MTColorPalette colorPalette;
		result = colorConf.Initialize(GetName());
		if (result == 0) {
			colorConf.GetSelectedColorPalette(&colorPalette);
			Color bgColor;
			colorPalette.GetBackgroundColor(&bgColor);
			m_BGColor[0] = bgColor.R();
			m_BGColor[1] = bgColor.G();
			m_BGColor[2] = bgColor.B();
			m_BGColor[3] = bgColor.A();
		}
	}
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
