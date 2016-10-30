//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D
//
// �s�A�m���[��3D�V�[���`��N���X
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
#include "MTNoteBox.h"
#include "MTNoteRipple.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"
#include "MTGridBox.h"
#include "MTPictBoard.h"
#include "MTDashboard.h"
#include "MTStars.h"
#include "MTTimeIndicator.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m���[��3D�V�[���`��N���X
//******************************************************************************
class MTScenePianoRoll3D : public MTScene
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTScenePianoRoll3D();
// >>> modify 20120728 yossiepon begin
	virtual ~MTScenePianoRoll3D();
// <<< modify 20120728 yossiepon end

	//���̎擾
	const TCHAR* GetName();

	//����
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//�ϊ�
// >>> modify 20120728 yossiepon begin
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end

	//�`��
// >>> modify 20120728 yossiepon begin
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
// <<< modify 20120728 yossiepon end

	//�j��
// >>> modify 20120728 yossiepon begin
	virtual void Release();
// <<< modify 20120728 yossiepon end

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
// >>> modify 20120728 yossiepon begin
	virtual int OnRecvSequencerMsg(
// <<< modify 20120728 yossiepon end
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
// >>> modify 20120728 yossiepon begin
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);
// <<< modify 20120728 yossiepon end

	//���t���x�ݒ�
	void SetPlaySpeedRatio(unsigned long ratio);

// >>> modify 20120728 yossiepon begin
protected:

	////��l�̃J����
	MTFirstPersonCam m_FirstPersonCam;

	//�`��I�u�W�F�N�g
	MTNoteBox m_NoteBox;
	MTNoteRipple m_NoteRipple;
	MTGridBox m_GridBox;
	MTPictBoard m_PictBoard;
	MTDashboard m_Dashboard;
	MTStars m_Stars;
	MTTimeIndicator m_TimeIndicator;

	//�s�b�`�x���h���
	MTNotePitchBend m_NotePitchBend;

	//�X�L�b�v���
	bool m_IsSkipping;

	//���C�g�L��
	BOOL m_IsEnableLight;

	virtual void _Reset();

// <<< modify 20120728 yossiepon end

private:

// >>> modify 20120728 yossiepon begin
	//���C�g
	DXDirLight m_DirLight;

// <<< modify 20120728 yossiepon end

	//�}�E�X�����ړ����[�h
	bool m_IsMouseCamMode;

	//������]���[�h
	bool m_IsAutoRollMode;

	//���_���
	MTViewParamMap m_ViewParamMap;

	//�m�[�g�f�U�C���I�u�W�F�N�g
	MTNoteDesign m_NoteDesign;

// >>> modify 20120728 yossiepon begin
	void _SetLightColor(DXDirLight* pLight);
// <<< modify 20120728 yossiepon end

};

