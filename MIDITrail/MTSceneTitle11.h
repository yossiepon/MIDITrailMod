//******************************************************************************
//
// MIDITrail / MTSceneTitle11
//
// DX11 title scene.
// Displays the MIDITrail logo with gradation animation.
// Standalone IMTScene11 implementation (no MTSceneBase11 dependency).
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "IMTScene11.h"
#include "DXCamera.h"
#include "MTLogo11.h"

#define MTSCENETITLE_CAMERA_POSZ        (-80.0f)
#define MTSCENETITLE_CAMERA_POSZ_DELTA  (0.05f)


//******************************************************************************
// DX11 title scene
//******************************************************************************
class MTSceneTitle11 : public IMTScene11
{
public:

	MTSceneTitle11();
	virtual ~MTSceneTitle11();

	const TCHAR* GetName() const override;
	bool IsLive() const override { return false; }

	int  Create(HWND hWnd, ID3D11Device* pDevice,
	            ID3D11DeviceContext* pContext,
	            SMIDILib::SMSeqData* pSeqData) override;
	void Release() override;

	int  Update() override;
	int  Draw(ID3D11DeviceContext* pContext,
	          const DirectX::SimpleMath::Matrix& viewProj,
	          float rollAngle,
	          const DirectX::SimpleMath::Vector3& camPos) override;

	int  OnPlayStart() override { return 0; }
	int  OnPlayEnd() override { return 0; }
	int  OnRecvSequencerMsg(unsigned long param1, unsigned long param2) override { return 0; }

	void SetEffect(MTEffectType type, bool isEnable) override {}
	void GetDefaultViewParam(MTViewParamMap* pParamMap) override {}
	void GetViewParam(MTViewParamMap* pParamMap) override {}
	void SetViewParam(MTViewParamMap* pParamMap) override {}
	void MoveToStaticViewpoint(unsigned long viewpointNo) override {}
	void ResetViewpoint() override {}

	int  OnWindowClicked(unsigned int button, WPARAM wParam, LPARAM lParam) override { return 0; }

	int  Rewind() override { return 0; }
	void SetPlaySpeedRatio(unsigned long ratio) override {}

	int  SetParam(const char* pKey, const char* pValue) override { return 0; }
	const char* GetParam(const char* pKey) override { return ""; }

	void SetNoteOnLive(unsigned char, unsigned char,
	                   unsigned char, unsigned char) override {}
	void SetNoteOffLive(unsigned char, unsigned char, unsigned char) override {}
	void AllNoteOffLive() override {}
	void AllNoteOffOnChLive(unsigned char, unsigned char) override {}

	unsigned long GetNoteCount() const override { return 0; }
	MTFirstPersonCam* GetCamera() override { return NULL; }
	const float* GetBGColor() const override { return m_BGColor; }

private:

	DXCamera m_Camera;
	MTLogo11 m_Logo;
	HWND m_hWnd;
	ID3D11DeviceContext* m_pContext;
	float m_CamPosZ;
	float m_BGColor[4];
};
