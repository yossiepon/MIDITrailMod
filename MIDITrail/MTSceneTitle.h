//******************************************************************************
//
// MIDITrail / MTSceneTitle
//
// �^�C�g���V�[���`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXCamera.h"
#include "DXDirLight.h"
#include "MTScene.h"
#include "MTLogo.h"
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�J����Z���W
#define MTSCENETITLE_CAMERA_POSZ  (-80.0f)

//�J����Z���W�ω���
#define MTSCENETITLE_CAMERA_POSZ_DELTA  (0.05f)


//******************************************************************************
// �^�C�g���V�[���`��N���X
//******************************************************************************
class MTSceneTitle : public MTScene
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^l
	MTSceneTitle(void);
	virtual ~MTSceneTitle(void);

	//���̎擾
	const TCHAR* GetName();

	//����
	int Create(
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

private:

	//�J�����ʒuZ
	float m_CamPosZ;

	//�J����
	DXCamera m_Camera;

	//���C�g
	DXDirLight m_DirLight;

	//���S�`��I�u�W�F�N�g
	MTLogo m_Logo;

};

