//******************************************************************************
//
// MIDITrail / DXDirLight
//
// �f�B���N�V���i�����C�g�N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// �f�B���N�V���i�����C�g�N���X
//******************************************************************************
class DXDirLight
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXDirLight(void);
	virtual ~DXDirLight(void);

	//������
	int Initialize();

	//���C�g�F�ݒ�
	void SetColor(D3DXCOLOR diffuse, D3DXCOLOR specular, D3DXCOLOR ambient);

	//���C�g�����o�^
	void SetDirection(D3DXVECTOR3 dirVector);

	//���C�g�����擾
	D3DXVECTOR3 GetDirection();

	//�f�o�C�X�ւ̃��C�g�o�^
	int SetDevice(
			LPDIRECT3DDEVICE9 pD3DDevice,
			BOOL isLightON
		);

// >>> add 20121229 yossiepon begin
	//�f�o�C�X�ւ̃��C�g�o�^
	int SetDevice(
			LPDIRECT3DDEVICE9 pD3DDevice,
			DWORD index,
			BOOL isLightON
		);
// <<< add 20121229 yossiepon end

private:

	D3DLIGHT9 m_Light;

};


