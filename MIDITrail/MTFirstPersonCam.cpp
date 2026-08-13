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

#include "StdAfx.h"
#include "mmsystem.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTFirstPersonCam.h"
#include <cmath>

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTFirstPersonCam::MTFirstPersonCam()
{
	m_CamVector = Vector3(0.0f, 0.0f, 0.0f);
	m_CamDirPhi = 0.0f;
	m_CamDirTheta = 0.0f;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_hWnd = NULL;

	m_VelocityFB = 15.0f;
	m_VelocityLR = 15.0f;
	m_VelocityUD = 10.0f;
	m_VelocityPT =  6.0f;
	m_AcceleRate =  2.0f;
	m_PrevTime = 0;
	m_DeltaTime = 0;

	m_RollAngle = 0.0f;
	m_VelocityAutoRoll = 6.0f;
	m_VelocityManualRoll = 1.0f;

	m_PrevTickTime = 0;
	m_CurTickTime = 0;
	m_ProgressDirection = DirX;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTFirstPersonCam::~MTFirstPersonCam()
{
	m_DIKeyCtrl.Terminate();
	m_DIMouseCtrl.Terminate();
	_ClipCursor(false);
}

//******************************************************************************
// Clear
//******************************************************************************
int MTFirstPersonCam::Clear()
{
	m_CamVector = Vector3(0.0f, 0.0f, 0.0f);
	m_CamDirPhi = 0.0f;
	m_CamDirTheta = 0.0f;
	return 0;
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTFirstPersonCam::Initialize(
		HWND hWnd,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	m_hWnd = hWnd;

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = m_DIKeyCtrl.Initialize(hWnd);
	if (result != 0) goto EXIT;

	result = m_DIMouseCtrl.Initialize(hWnd);
	if (result != 0) goto EXIT;

	m_DIKeyCtrl.Acquire();
	m_DIMouseCtrl.Acquire();

	result = m_GamePadCtrl.Initialize(0);
	if (result != 0) goto EXIT;

	result = m_Camera.Initialize();
	if (result != 0) goto EXIT;

	m_Camera.SetBaseParam(45.0f, 1.0f, 1000.0f);

	m_Camera.SetPosition(
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, 1.0f),
			Vector3(0.0f, 1.0f, 0.0f)
		);

EXIT:;
	return result;
}

//******************************************************************************
// Position
//******************************************************************************
void MTFirstPersonCam::SetPosition(Vector3 camVector)
{
	m_CamVector = camVector;
}

void MTFirstPersonCam::GetPosition(Vector3* pCamVector)
{
	*pCamVector = m_CamVector;
}

//******************************************************************************
// Direction
//******************************************************************************
void MTFirstPersonCam::SetDirection(float phi, float theta)
{
	m_CamDirPhi = phi;
	m_CamDirTheta = theta;
}

void MTFirstPersonCam::GetDirection(float* pPhi, float* pTheta)
{
	*pPhi = m_CamDirPhi;
	*pTheta = m_CamDirTheta;
}

//******************************************************************************
// Mouse camera mode
//******************************************************************************
void MTFirstPersonCam::SetMouseCamMode(bool isEnable)
{
	m_IsMouseCamMode = isEnable;

	if (m_IsMouseCamMode) {
		ShowCursor(FALSE);
		_ClipCursor(true);
	}
	else {
		ShowCursor(TRUE);
		_ClipCursor(false);
	}
}

//******************************************************************************
// Auto-roll mode
//******************************************************************************
void MTFirstPersonCam::SetAutoRollMode(bool isEnable)
{
	m_IsAutoRollMode = isEnable;
}

void MTFirstPersonCam::SwitchAutoRollDir()
{
	m_VelocityAutoRoll *= -1.0f;
}

//******************************************************************************
// Per-frame input processing (replaces Transform(LPDIRECT3DDEVICE9))
//******************************************************************************
int MTFirstPersonCam::Update(const MTSceneUpdateContext& ctx)
{
	m_CurTickTime = ctx.curTickTime;
	int result = 0;
	float dt = 0.0f;
	int dX = 0;
	int dY = 0;
	int dW = 0;

	dt = (float)m_DeltaTime / 1000.0f;

	m_DIKeyCtrl.Acquire();
	m_DIMouseCtrl.Acquire();

	result = m_DIKeyCtrl.GetKeyStatus();
	if (result != 0) goto EXIT;

	result = m_DIMouseCtrl.GetMouseStatus();
	if (result != 0) goto EXIT;

	result = m_GamePadCtrl.UpdateState();
	if (result != 0) goto EXIT;

	dX = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisX);
	dY = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisY);
	dW = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisWheel);

	if (!m_IsMouseCamMode) {
		dX = 0;
		dY = 0;
	}

	dX += (int)(m_VelocityPT * dt * m_GamePadCtrl.GetState_ThumbRX() * 100.0f);
	dY += (int)(m_VelocityPT * dt * m_GamePadCtrl.GetState_ThumbRY() * (-100.0f));

	if (m_DIKeyCtrl.IsKeyDown(DIK_LCONTROL) || m_DIKeyCtrl.IsKeyDown(DIK_RCONTROL)) {
		if (m_DIKeyCtrl.IsKeyDown(DIK_W) || m_DIKeyCtrl.IsKeyDown(DIK_UP)) {
			dY -= (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_S) || m_DIKeyCtrl.IsKeyDown(DIK_DOWN)) {
			dY += (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_A) || m_DIKeyCtrl.IsKeyDown(DIK_LEFT)) {
			dX -= (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_D) || m_DIKeyCtrl.IsKeyDown(DIK_RIGHT)) {
			dX += (int)m_VelocityPT;
		}
	}

	_CalcDeltaTime();

	result = _TransformEyeDirection(dX, dY);
	if (result != 0) goto EXIT;

	result = _TransformCamPosition();
	if (result != 0) goto EXIT;

	result = _SetCamPosition();
	if (result != 0) goto EXIT;

	result = _TransformRolling(dW);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Get view/projection matrices
//******************************************************************************
void MTFirstPersonCam::GetViewProjection(
		float aspect,
		Matrix* pView,
		Matrix* pProj
	)
{
	m_Camera.GetMatrices(aspect, pView, pProj);
}

//******************************************************************************
// Get roll angle
//******************************************************************************
float MTFirstPersonCam::GetRollAngle()
{
	return m_RollAngle;
}

//******************************************************************************
// Eye direction update
//******************************************************************************
int MTFirstPersonCam::_TransformEyeDirection(int dX, int dY)
{
	float dt = (float)m_DeltaTime / 1000.0f;

	float dPhi   = (float)-dX * m_VelocityPT * dt;
	float dTheta = (float) dY * m_VelocityPT * dt;

	if (fabsf(dPhi) > 45.0f) dPhi = 0.0f;
	if (fabsf(dTheta) > 45.0f) dTheta = 0.0f;

	m_CamDirPhi += dPhi;
	m_CamDirTheta += dTheta;

	if (m_CamDirPhi >= 360.0f) m_CamDirPhi -= 360.0f;
	else if (m_CamDirPhi <= -360.0f) m_CamDirPhi += 360.0f;

	if (m_CamDirTheta <= 1.0f) m_CamDirTheta = 1.0f;
	else if (m_CamDirTheta >= 179.0f) m_CamDirTheta = 179.0f;

	return 0;
}

//******************************************************************************
// Camera position update
//******************************************************************************
int MTFirstPersonCam::_TransformCamPosition()
{
	float dt = (float)m_DeltaTime / 1000.0f;
	float phi = m_CamDirPhi;
	float distance = 0.0f;
	float dFB = 0.0f;
	float dLR = 0.0f;

	if (m_DIKeyCtrl.IsKeyDown(DIK_LCONTROL) || m_DIKeyCtrl.IsKeyDown(DIK_RCONTROL)) {
		// CTRL held: direction keys handled in TransformInput
	}
	else {
		float rate = 1.0f;
		if (m_DIKeyCtrl.IsKeyDown(DIK_LSHIFT) || m_DIKeyCtrl.IsKeyDown(DIK_RSHIFT)) {
			rate = m_AcceleRate;
		}

		if (m_DIKeyCtrl.IsKeyDown(DIK_W) || m_DIKeyCtrl.IsKeyDown(DIK_UP)) {
			distance = m_VelocityFB * dt * rate;
			phi += 0.0f;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_S) || m_DIKeyCtrl.IsKeyDown(DIK_DOWN)) {
			distance = m_VelocityFB * dt * rate;
			phi += 180.0f;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_A) || m_DIKeyCtrl.IsKeyDown(DIK_LEFT)) {
			distance = m_VelocityLR * dt * rate;
			phi += 90.0f;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_D) || m_DIKeyCtrl.IsKeyDown(DIK_RIGHT)) {
			distance = m_VelocityLR * dt * rate;
			phi += -90.0f;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_Q) || m_DIKeyCtrl.IsKeyDown(DIK_PRIOR)) {
			m_CamVector.y += +(m_VelocityUD * dt * rate);
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_E) || m_DIKeyCtrl.IsKeyDown(DIK_NEXT)) {
			m_CamVector.y += -(m_VelocityUD * dt * rate);
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_Z) || m_DIKeyCtrl.IsKeyDown(DIK_COMMA)) {
			m_CamVector.x += -(m_VelocityFB * dt * rate);
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_C) || m_DIKeyCtrl.IsKeyDown(DIK_PERIOD)) {
			m_CamVector.x += +(m_VelocityFB * dt * rate);
		}
	}

	// Gamepad: D-pad movement
	if (distance == 0.0f) {
		if (m_GamePadCtrl.GetState_DPadUp())    dFB = m_VelocityFB * dt * (1.0f);
		if (m_GamePadCtrl.GetState_DPadDown())   dFB = m_VelocityFB * dt * (-1.0f);
		if (m_GamePadCtrl.GetState_DPadRight())  dLR = m_VelocityLR * dt * (-1.0f);
		if (m_GamePadCtrl.GetState_DPadLeft())   dLR = m_VelocityLR * dt * (1.0f);
		distance = sqrtf(dFB * dFB + dLR * dLR);
		phi += XMConvertToDegrees(atan2f(dLR, dFB));
	}
	// Gamepad: left stick movement
	if (distance == 0.0f) {
		dFB += m_VelocityFB * dt * m_GamePadCtrl.GetState_ThumbLX() * -1.0f;
		dLR += m_VelocityLR * dt * m_GamePadCtrl.GetState_ThumbLY();
		distance = sqrtf(dFB * dFB + dLR * dLR);
		phi += XMConvertToDegrees(atan2f(dFB, dLR));
	}
	// Gamepad: X/Y buttons for vertical
	if (m_GamePadCtrl.GetState_X()) m_CamVector.y -= m_VelocityUD * dt;
	if (m_GamePadCtrl.GetState_Y()) m_CamVector.y += m_VelocityUD * dt;

	if (phi >= 360.0f) phi -= 360.0f;
	else if (phi <= -360.0f) phi += 360.0f;

	// Move vector (spherical to cartesian)
	float phiRad = XMConvertToRadians(phi);
	m_CamVector.x += distance * cosf(phiRad);
	m_CamVector.z += distance * sinf(phiRad);

	// Playback tracking
	float progress = m_NoteDesign.GetPlayPosX(m_CurTickTime)
	               - m_NoteDesign.GetPlayPosX(m_PrevTickTime);
	switch (m_ProgressDirection) {
		case DirX: m_CamVector.x += progress; break;
		case DirY: m_CamVector.y += progress; break;
		case DirZ: m_CamVector.z += progress; break;
	}

	_ClipCamVector(&m_CamVector);
	m_PrevTickTime = m_CurTickTime;

	return 0;
}

//******************************************************************************
// Rolling
//******************************************************************************
int MTFirstPersonCam::_TransformRolling(int dW)
{
	float dt = (float)m_DeltaTime / 1000.0f;

	float domega = (float)dW * m_VelocityManualRoll * dt;
	if (fabsf(domega) > 45.0f) domega = 0.0f;

	if (m_IsAutoRollMode) {
		domega += m_VelocityAutoRoll * dt;
	}

	m_RollAngle += domega;

	if (m_RollAngle >= 360.0f) m_RollAngle -= 360.0f;
	else if (m_RollAngle <= -360.0f) m_RollAngle += 360.0f;

	return 0;
}

//******************************************************************************
// Roll angle accessors
//******************************************************************************
float MTFirstPersonCam::GetManualRollAngle()
{
	return m_RollAngle;
}

void MTFirstPersonCam::SetManualRollAngle(float rollAngle)
{
	m_RollAngle = rollAngle;
}

float MTFirstPersonCam::GetAutoRollVelocity()
{
	return m_VelocityAutoRoll;
}

void MTFirstPersonCam::SetAutoRollVelocity(float rollVelocity)
{
	m_VelocityAutoRoll = rollVelocity;
}

//******************************************************************************
// Set camera position on DXCamera (internal)
//******************************************************************************
int MTFirstPersonCam::_SetCamPosition()
{
	float phiRad   = XMConvertToRadians(m_CamDirPhi);
	float thetaRad = XMConvertToRadians(m_CamDirTheta);

	Vector3 lookVector;
	lookVector.x = 10.0f * sinf(thetaRad) * cosf(phiRad);
	lookVector.y = 10.0f * cosf(thetaRad);
	lookVector.z = 10.0f * sinf(thetaRad) * sinf(phiRad);

	Vector3 camLookAt;
	camLookAt.x = m_CamVector.x + lookVector.x;
	camLookAt.y = m_CamVector.y + lookVector.y;
	camLookAt.z = m_CamVector.z + lookVector.z;

	Vector3 camUp(0.0f, 1.0f, 0.0f);

	m_Camera.SetPosition(m_CamVector, camLookAt, camUp);

	return 0;
}

//******************************************************************************
// Cursor clipping
//******************************************************************************
int MTFirstPersonCam::_ClipCursor(bool isClip)
{
	int result = 0;

	if (isClip) {
		RECT wrect, crect, clip;
		if (!GetWindowRect(m_hWnd, &wrect)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
		if (!GetClientRect(m_hWnd, &crect)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
		int wh = wrect.bottom - wrect.top;
		int ww = wrect.right  - wrect.left;
		int ch = crect.bottom - crect.top;
		int cw = crect.right  - crect.left;
		clip = wrect;
		clip.left   += (ww - cw);
		clip.right  -= (ww - cw);
		clip.top    += (wh - ch);
		clip.bottom -= (wh - ch);
		if (!::ClipCursor(&clip)) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		::ClipCursor(NULL);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Delta time
//******************************************************************************
void MTFirstPersonCam::_CalcDeltaTime()
{
	unsigned long curTime = timeGetTime();

	if (m_PrevTime == 0) {
		m_DeltaTime = 0;
	}
	else {
		m_DeltaTime = curTime - m_PrevTime;
	}

	m_PrevTime = curTime;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTFirstPersonCam::Reset()
{
	m_PrevTime = 0;
	m_DeltaTime = 0;
	m_PrevTickTime = 0;
	m_CurTickTime = 0;
}

//******************************************************************************
// Load config
//******************************************************************************
int MTFirstPersonCam::_LoadConfFile(const TCHAR* pSceneName)
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("FirstPersonCam"));
	if (result != 0) goto EXIT;
	confFile.GetFloat(_T("VelocityFB"), &m_VelocityFB, 15.0f);
	confFile.GetFloat(_T("VelocityLR"), &m_VelocityLR, 15.0f);
	confFile.GetFloat(_T("VelocityUD"), &m_VelocityUD, 10.0f);
	confFile.GetFloat(_T("VelocityPT"), &m_VelocityPT, 6.0f);
	confFile.GetFloat(_T("AcceleRate"), &m_AcceleRate, 2.0f);
	confFile.GetFloat(_T("VelocityAutoRoll"), &m_VelocityAutoRoll, 6.0f);
	confFile.GetFloat(_T("VelocityManualRoll"), &m_VelocityManualRoll, 1.0f);

EXIT:;
	return result;
}

//******************************************************************************
// Camera position clipping
//******************************************************************************
void MTFirstPersonCam::_ClipCamVector(Vector3* pVector)
{
	const float limit = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	if (pVector->x < -limit) pVector->x = -limit;
	if (pVector->x >  limit) pVector->x =  limit;
	if (pVector->y < -limit) pVector->y = -limit;
	if (pVector->y >  limit) pVector->y =  limit;
	if (pVector->z < -limit) pVector->z = -limit;
	if (pVector->z >  limit) pVector->z =  limit;
}

//******************************************************************************
// Progress direction
//******************************************************************************
void MTFirstPersonCam::SetProgressDirection(MTProgressDirection dir)
{
	m_ProgressDirection = dir;
}

//******************************************************************************
// ViewParam support
//******************************************************************************
void MTFirstPersonCam::GetViewParam(MTViewParamMap* pParamMap)
{
	(*pParamMap)["X"] = m_CamVector.x;
	(*pParamMap)["Y"] = m_CamVector.y;
	(*pParamMap)["Z"] = m_CamVector.z;
	(*pParamMap)["Phi"] = m_CamDirPhi;
	(*pParamMap)["Theta"] = m_CamDirTheta;
	(*pParamMap)["ManualRollAngle"] = m_RollAngle;
	(*pParamMap)["AutoRollVelocity"] = m_VelocityAutoRoll;
}

void MTFirstPersonCam::SetViewParam(MTViewParamMap* pParamMap)
{
	auto it = pParamMap->find("X");
	if (it != pParamMap->end()) m_CamVector.x = it->second;
	it = pParamMap->find("Y");
	if (it != pParamMap->end()) m_CamVector.y = it->second;
	it = pParamMap->find("Z");
	if (it != pParamMap->end()) m_CamVector.z = it->second;
	it = pParamMap->find("Phi");
	if (it != pParamMap->end()) m_CamDirPhi = it->second;
	it = pParamMap->find("Theta");
	if (it != pParamMap->end()) m_CamDirTheta = it->second;
	it = pParamMap->find("ManualRollAngle");
	if (it != pParamMap->end()) m_RollAngle = it->second;
	it = pParamMap->find("AutoRollVelocity");
	if (it != pParamMap->end()) m_VelocityAutoRoll = it->second;

	_SetCamPosition();
}
