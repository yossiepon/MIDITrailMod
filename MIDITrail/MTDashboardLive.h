//******************************************************************************
//
// MIDITrail / MTDashboardLive
//
// ���C�u���j�^�p�_�b�V���{�[�h�`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// MIDI IN �f�C�o�C�X��, �m�[�g�� ��\������B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"
#include "DXColorUtil.h"
#include "MTStaticCaption.h"
#include "MTDynamicCaption.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�t�H���g�ݒ�
//  Windows �F�t�H���g�T�C�Y40 -> �r�b�g�}�b�v�T�C�Y�c40�s�N�Z�� (MS Gothic)
//  Mac OS X�F�t�H���g�T�C�Y40 -> �r�b�g�}�b�v�T�C�Y�c50�s�N�Z�� (Monaco)
#define MTDASHBOARDLIVE_FONTNAME  _T("MS Gothic")
#define MTDASHBOARDLIVE_FONTSIZE  (40)

//�J�E���^�L���v�V����������
#define MTDASHBOARDLIVE_COUNTER_CHARS  "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:/%[] "

//�J�E���^�L���v�V�����T�C�Y
//   1234567890123456789012345678901  (31)
//  "NOTES:00000000 [MONITERING OFF]"
//  �]�T���݂�40�ɂ��Ă���
#define MTDASHBOARDLIVE_COUNTER_SIZE  (40)

//�g�T�C�Y�i�s�N�Z���j
#define MTDASHBOARDLIVE_FRAMESIZE  (5.0f)

//�f�t�H���g�\���g�嗦
#define MTDASHBOARDLIVE_DEFAULT_MAGRATE  (0.45f)  //Windows�łł�0.5

//******************************************************************************
// ���C�u���j�^�p�_�b�V���{�[�h�`��N���X
//******************************************************************************
class MTDashboardLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTDashboardLive(void);
	virtual ~MTDashboardLive(void);
	
	//����
	int Create(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName, HWND hWnd);
	
	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 camVector);
	
	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//���
	void Release();
	
	//���j�^��ԓo�^
	void SetMonitoringStatus(bool isMonitoring);
	
	//�m�[�gON�o�^
	void SetNoteOn();
	
	//���Z�b�g
	void Reset();
	
	//�\���ݒ�
	void SetEnable(bool isEnable);
	
	//MIDI IN �f�o�C�X���o�^
	int SetMIDIINDeviceName(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pName);
	
private:
	
	HWND m_hWnd;

	MTStaticCaption m_Title;
	
	MTDynamicCaption m_Counter;
	float m_PosCounterX;
	float m_PosCounterY;
	float m_CounterMag;
	
	bool m_isMonitoring;
	unsigned long m_NoteCount;
	
	D3DXCOLOR m_CaptionColor;
	
	//�\����
	bool m_isEnable;
	
	int _GetCounterPos(float* pX, float* pY);
	int _GetCounterStr(TCHAR* pStr, unsigned long bufSize);
	int _LoadConfFile(const TCHAR* pSceneName);
	
};


