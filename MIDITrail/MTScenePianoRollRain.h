//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain
//
// �s�A�m���[�����C���V�[���`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTStars.h"
#include "MTPianoKeyboardCtrl.h"
#include "MTNoteRain.h"
#include "MTDashboard.h"
#include "MTNotePitchBend.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m���[�����C���V�[���`��N���X
//******************************************************************************
class MTScenePianoRollRain : public MTScene
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTScenePianoRollRain(void);
	virtual ~MTScenePianoRollRain(void);

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

	//���t���x�ݒ�
	void SetPlaySpeedRatio(unsigned long ratio);

protected:

	//���C�g�L��
	BOOL m_IsEnableLight;

private:

	//���C�g
	DXDirLight m_DirLight;

	//��l�̃J����
	MTFirstPersonCam m_FirstPersonCam;

	//�`��I�u�W�F�N�g
	MTStars m_Stars;
	MTPianoKeyboardCtrl m_PianoKeyboardCtrl;
	MTNoteRain m_NoteRain;
	MTNotePitchBend m_NotePitchBend;
	MTDashboard m_Dashboard;

	//�}�E�X�����ړ����[�h
	bool m_IsMouseCamMode;

	//������]���[�h
	bool m_IsAutoRollMode;

	//���_���
	MTViewParamMap m_ViewParamMap;

	//���t�ʒu
	unsigned long m_CurTickTime;

	//�X�L�b�v���
	bool m_IsSkipping;

	void _Reset();
	void _SetLightColor(DXDirLight* pLight);

};


