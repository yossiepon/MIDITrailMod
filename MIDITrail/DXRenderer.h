//******************************************************************************
//
// MIDITrail / DXRenderer
//
// �����_���N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXScene.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�f�o�C�X���X�g
#define DXRENDERER_ERR_DEVICE_LOST  (100)

#define DX_MULTI_SAMPLE_TYPE_MIN    (2)
#define DX_MULTI_SAMPLE_TYPE_MAX    (16)


//******************************************************************************
// �����_���N���X
//******************************************************************************
class DXRenderer
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXRenderer();
	virtual ~DXRenderer();

public:

	//������
	int Initialize(HWND hWnd, unsigned long multiSampleType = 0, bool isFullScreen = false);

	//�f�o�C�X�擾
	LPDIRECT3DDEVICE9 GetDevice();

	//�`��
	int RenderScene(DXScene* pScene);

	//�I������
	void Terminate();

	//�A���`�G�C���A�X�T�|�[�g�m�F
	int IsSupportAntialias(unsigned long multiSampleNum, bool* pIsSupport);

	//�C���f�b�N�X�o�b�t�@�T�|�[�g�m�F
	int IsSupportIndexBuffer(bool* pIsSupport, unsigned long* pMaxVertexIndex);

private:

	HWND m_hWnd;
	LPDIRECT3D9 m_pD3D;
	LPDIRECT3DDEVICE9 m_pD3DDevice;
	D3DPRESENT_PARAMETERS m_D3DPP;

	int _RecoverDevice();
	int _CheckAntialiasSupport(
			D3DPRESENT_PARAMETERS d3dpp,
			D3DMULTISAMPLE_TYPE multiSampleType,
			bool* pIsSupport,
			unsigned long* pQualityLevels
		);
	D3DMULTISAMPLE_TYPE _EnumMultiSampleType(unsigned long multiSampleType);

};


