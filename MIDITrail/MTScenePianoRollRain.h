//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain
//
// �s�A�m���[�����C���V�[���`��N���X
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
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
#include "MTMeshCtrl.h"
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
			UINT button,
			WPARAM wParam,
			LPARAM lParam
		);

	//���t�J�n�C�x���g��M
	int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);

	//���t�I���C�x���g��M
	int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);

	//�V�[�P���T���b�Z�[�W��M
	int OnRecvSequencerMsg(
			unsigned long param1,
			unsigned long param2
		);

	//�����߂�
	int Rewind();

	//���_�擾�^�o�^
	virtual void GetDefaultViewParam(MTViewParamMap* pParamMap);
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

	//�V���O���L�[�{�[�h�t���O
	bool m_IsSingleKeyboard;

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
	MTMeshCtrl m_MeshCtrl;

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
	int _LoadConf();

};


