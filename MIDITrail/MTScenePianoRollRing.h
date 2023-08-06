//******************************************************************************
//
// MIDITrail / MTScenePianoRollRing
//
// �s�A�m���[�������O�V�[���`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTFirstPersonCam.h"
#include "MTNoteBoxRing.h"
#include "MTNoteRippleRing.h"
#include "MTNoteDesignRing.h"
#include "MTNotePitchBend.h"
#include "MTGridRing.h"
#include "MTPictBoardRing.h"
#include "MTDashboard.h"
#include "MTStars.h"
#include "MTTimeIndicatorRing.h"
#include "MTMeshCtrl.h"
#include "MTBackgroundImage.h"
#include "MTConfFile.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m���[�������O�V�[���`��N���X
//******************************************************************************
class MTScenePianoRollRing : public MTScene
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTScenePianoRollRing();
	// >>> modify 20191222 yossiepon begin
	virtual ~MTScenePianoRollRing();
	// <<< modify 20191222 yossiepon end

	//���̎擾
	const TCHAR* GetName();

	//����
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

	//�ϊ�
	// >>> modify 20191222 yossiepon begin
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);
	// <<< modify 20191222 yossiepon end

	//�`��
	// >>> modify 20191222 yossiepon begin
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	// <<< modify 20191222 yossiepon end

	//�j��
	// >>> modify 20191222 yossiepon begin
	virtual void Release();
	// <<< modify 20191222 yossiepon end

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
	// >>> modify 20191222 yossiepon begin
	virtual int OnRecvSequencerMsg(
	// <<< modify 20191222 yossiepon end
			unsigned long param1,
			unsigned long param2
		);

	//�����߂�
	int Rewind();

	//���_�擾�^�o�^
	void GetDefaultViewParam(MTViewParamMap* pParamMap);
	void GetViewParam(MTViewParamMap* pParamMap);
	void SetViewParam(MTViewParamMap* pParamMap);
	void MoveToStaticViewpoint(unsigned long viewpointNo);

	//���_���Z�b�g
	void ResetViewpoint();

	//�G�t�F�N�g�ݒ�
	// >>> modify 20191222 yossiepon begin
	virtual void SetEffect(MTScene::EffectType type, bool isEnable);
	// <<< modify 20191222 yossiepon end

	//���t���x�ݒ�
	void SetPlaySpeedRatio(unsigned long ratio);

protected:

	//���C�g�L��
	BOOL m_IsEnableLight;

// >>> modify access level to protected 20191222 yossiepon begin
//private:
// <<< modify 20191222 yossiepon end

	//���C�g
	DXDirLight m_DirLight;

	//��l�̃J����
	MTFirstPersonCam m_FirstPersonCam;

	//�`��I�u�W�F�N�g
	MTNoteBoxRing m_NoteBox;
	MTNoteRippleRing m_NoteRipple;
	MTNotePitchBend m_NotePitchBend;
	MTGridRing m_GridRing;
	MTPictBoardRing m_PictBoard;
	MTDashboard m_Dashboard;
	MTStars m_Stars;
	MTTimeIndicatorRing m_TimeIndicator;
	MTMeshCtrl m_MeshCtrl;
	MTBackgroundImage m_BackgroundImage;

	//�}�E�X�����ړ����[�h
	bool m_IsMouseCamMode;

	//������]���[�h
	bool m_IsAutoRollMode;

	//���_���
	MTViewParamMap m_ViewParamMap;
	MTViewParamMap m_Viewpoint2;
	MTViewParamMap m_Viewpoint3;

	//�m�[�g�f�U�C���I�u�W�F�N�g
	MTNoteDesignRing m_NoteDesign;

	//�X�L�b�v���
	bool m_IsSkipping;

	//�V�[�����Z�b�g
	// >>> modify 20191222 yossiepon begin
	virtual void _Reset();
	// <<< modify 20191222 yossiepon end

	void _SetLightColor(DXDirLight* pLight);
	int _LoadConf();
	int _LoadConfViewpoint(MTConfFile* pConfFile, unsigned long viewpointNo, MTScene::MTViewParamMap* pParamMap);

};

