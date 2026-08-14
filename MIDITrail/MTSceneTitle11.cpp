//******************************************************************************
//
// MIDITrail / MTSceneTitle11
//
// Title scene.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTSceneTitle11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTSceneTitle11::MTSceneTitle11()
{
	m_CamPosZ = MTSCENETITLE_CAMERA_POSZ;
	m_pContext = NULL;
	m_BGColor[0] = 0.0f;
	m_BGColor[1] = 0.0f;
	m_BGColor[2] = 0.0f;
	m_BGColor[3] = 1.0f;
}

MTSceneTitle11::~MTSceneTitle11()
{
	Release();
}

//******************************************************************************
// Name
//******************************************************************************
const TCHAR* MTSceneTitle11::GetName() const
{
	return _T("Title");
}

//******************************************************************************
// Create
//******************************************************************************
int MTSceneTitle11::Create(
		HWND hWnd,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	m_hWnd = hWnd;
	m_pContext = pContext;

	m_Camera.SetBaseParam(45.0f, 1.0f, 1000.0f);
	m_Camera.SetPosition(
		Vector3(0.0f, 0.0f, m_CamPosZ),
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f)
	);

	result = m_Logo.Create(pDevice, pContext);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTSceneTitle11::Release()
{
	m_Logo.Release();
}

//******************************************************************************
// Update
//******************************************************************************
int MTSceneTitle11::Update()
{
	int result = 0;

	m_CamPosZ += MTSCENETITLE_CAMERA_POSZ_DELTA;
	m_Camera.SetPosition(
		Vector3(0.0f, 0.0f, m_CamPosZ),
		Vector3(0.0f, 0.0f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f)
	);

	result = m_Logo.Update(m_pContext);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTSceneTitle11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	float aspect = (float)(rc.right - rc.left) / (float)(rc.bottom - rc.top);

	Matrix view, proj;
	m_Camera.GetMatrices(aspect, &view, &proj);
	Matrix titleViewProj = view * proj;

	Vector4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);
	result = m_Logo.Draw(pContext, titleViewProj, lightDir);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}
