//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D
//
// �s�A�m���[��3D�V�[���`��N���X
//
// Copyright (C) 2010-2016 WADA Masashi. All Rights Reserved.
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
#include "MTMeshCtrl.h"
#include "MTBackgroundImage.h"
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
			UINT button,
			WPARAM wParam,
			LPARAM lParam
		);

	//���t�J�n�C�x���g��M
	int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);

	//���t�I���C�x���g��M
	int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);

	//�V�[�P���T���b�Z�[�W��M
// >>> modify 20120728 yossiepon begin
	virtual int OnRecvSequencerMsg(
// <<< modify 20120728 yossiepon end
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
// >>> modify 20120728 yossiepon begin
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);
// <<< modify 20120728 yossiepon end

	//���t���x�ݒ�
	void SetPlaySpeedRatio(unsigned long ratio);

protected:

	//���C�g�L��
	BOOL m_IsEnableLight;

private:

	//���C�g
	DXDirLight m_DirLight;

// >>> modify access level to protected 20161223 yossiepon begin
protected:
// <<< modify 20161223 yossiepon end

	//��l�̃J����
	MTFirstPersonCam m_FirstPersonCam;

	//�`��I�u�W�F�N�g
	MTNoteBox m_NoteBox;
	MTNoteRipple m_NoteRipple;
	MTNotePitchBend m_NotePitchBend;
	MTGridBox m_GridBox;
	MTPictBoard m_PictBoard;
	MTDashboard m_Dashboard;
	MTStars m_Stars;
	MTTimeIndicator m_TimeIndicator;
	MTMeshCtrl m_MeshCtrl;
	MTBackgroundImage m_BackgroundImage;

// >>> modify access level 20161223 yossiepon begin
private:
// <<< modify access level 20161223 yossiepon end

	//�}�E�X�����ړ����[�h
	bool m_IsMouseCamMode;

	//������]���[�h
	bool m_IsAutoRollMode;

	//���_���
	MTViewParamMap m_ViewParamMap;

	//�m�[�g�f�U�C���I�u�W�F�N�g
	MTNoteDesign m_NoteDesign;

// >>> modify access level to protected 20161223 yossiepon begin
protected:
// <<< modify 20161223 yossiepon end

	//�X�L�b�v���
	bool m_IsSkipping;

// >>> modify 20120728 yossiepon begin
	virtual void _Reset();
// <<< modify 20120728 yossiepon end
	void _SetLightColor(DXDirLight* pLight);

// >>> modify access level 20161223 yossiepon begin
private:
// <<< modify access level 20161223 yossiepon end

	int _LoadConf();

};

