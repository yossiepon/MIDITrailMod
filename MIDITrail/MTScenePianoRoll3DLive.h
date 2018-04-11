//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DLive
//
// ���C�u���j�^�p�s�A�m���[��3D�V�[���`��N���X
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTNoteBoxLive.h"
#include "MTNoteRipple.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "MTGridBoxLive.h"
#include "MTPictBoard.h"
#include "MTDashboardLive.h"
#include "MTStars.h"
#include "MTTimeIndicator.h"
#include "MTMeshCtrl.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// ���C�u���j�^�p�s�A�m���[��3D�V�[���`��N���X
//******************************************************************************
class MTScenePianoRoll3DLive : public MTScene
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^l
	MTScenePianoRoll3DLive();
	~MTScenePianoRoll3DLive();
	
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
	MTNoteBoxLive m_NoteBoxLive;
	MTNoteRipple m_NoteRipple;
	MTNotePitchBend m_NotePitchBend;
	MTGridBoxLive m_GridBoxLive;
	MTPictBoard m_PictBoard;
	MTDashboardLive m_DashboardLive;
	MTStars m_Stars;
	MTTimeIndicator m_TimeIndicator;
	MTMeshCtrl m_MeshCtrl;
	
	//�}�E�X�����ړ����[�h
	bool m_IsMouseCamMode;
	
	//������]���[�h
	bool m_IsAutoRollMode;
	
	//���_���
	MTViewParamMap m_ViewParamMap;
	
	//�m�[�g�f�U�C���I�u�W�F�N�g
	MTNoteDesign m_NoteDesign;
	
	//�X�L�b�v���
	bool m_IsSkipping;
	
	void _Reset();
	int _LoadConf();

};


