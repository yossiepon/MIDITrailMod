//******************************************************************************
//
// MIDITrail / MTScenePianoRollRainLive
//
// ���C�u���j�^�p�s�A�m���[�����C���V�[���`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTStars.h"
#include "MTPianoKeyboardCtrlLive.h"
#include "MTNoteRainLive.h"
#include "MTDashboardLive.h"
#include "MTNotePitchBend.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m���[�����C���V�[���`��N���X
//******************************************************************************
class MTScenePianoRollRainLive : public MTScene
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTScenePianoRollRainLive(void);
	virtual ~MTScenePianoRollRainLive(void);
	
	//���̎擾
	const TCHAR* GetName();
	
	//����
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);
	
	//�ϊ�
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//�j��
	void Release();
	
	//�E�B���h�E�N���b�N�C�x���g��M
	int OnWindowClicked(
			unsigned long button,
			unsigned long wParam,
			unsigned long lParam
		);
	
	//���t�J�n�C�x���g��M
	int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//���t�I���C�x���g��M
	int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//�V�[�P���T���b�Z�[�W��M
	int OnRecvSequencerMsg(
			unsigned long wParam,
			unsigned long lParam
		);
	
	//�����߂�
	int Rewind();
	
	//���_�擾�^�o�^
	void GetDefaultViewParam(MTViewParamMap* pParamMap);
	void GetViewParam(MTViewParamMap* pParamMap);
	void SetViewParam(MTViewParamMap* pParamMap);
	
	//���_���Z�b�g
	void ResetViewpoint();
	
	//�G�t�F�N�g�ݒ�
	void SetEffect(MTScene::EffectType type, bool isEnable);
	
protected:
	
	//���C�g�L��
	bool m_IsEnableLight;
	
private:
	
	//���C�g
	DXDirLight m_DirLight;
	
	//��l�̃J����
	MTFirstPersonCam m_FirstPersonCam;
	
	//�`��I�u�W�F�N�g
	MTStars m_Stars;
	MTPianoKeyboardCtrlLive m_PianoKeyboardCtrlLive;
	MTNoteRainLive m_NoteRainLive;
	MTNotePitchBend m_NotePitchBend;
	MTDashboardLive m_DashboardLive;
	
	//�}�E�X�����ړ����[�h
	bool m_IsMouseCamMode;
	
	//������]���[�h
	bool m_IsAutoRollMode;
	
	//���_���
	MTViewParamMap m_ViewParamMap;
	
	void _Reset();
	
};

