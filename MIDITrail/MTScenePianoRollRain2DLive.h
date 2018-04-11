//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain2DLive
//
// ���C�u���j�^�p�s�A�m���[�����C��2D�V�[���`��N���X
//
// Copyright (C) 2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "MTScenePianoRollRainLive.h"


//******************************************************************************
// �s�A�m���[�����C��2D�V�[���`��N���X
//******************************************************************************
class MTScenePianoRollRain2DLive : public MTScenePianoRollRainLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTScenePianoRollRain2DLive(void);
	virtual ~MTScenePianoRollRain2DLive(void);
	
	//���̎擾
	const TCHAR* GetName();
	
	//����
	virtual int Create(
				HWND hWnd,
				LPDIRECT3DDEVICE9 pD3DDevice,
				SMSeqData* pSeqData
			);
	
	//���_�擾
	virtual void GetDefaultViewParam(MTViewParamMap* pParamMap);

};


