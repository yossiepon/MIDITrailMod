//******************************************************************************
//
// MIDITrail / MTNoteBoxLive
//
// ���C�u���j�^�p�m�[�g�{�b�N�X�`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// ���C�u���j�^�p�m�[�g�{�b�N�X��`�悷��B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "MTNoteDesign.h"
#include "MTNotePitchBend.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő唭���m�[�g�`�搔
#define MTNOTEBOX_MAX_LIVENOTE_NUM  (2048)

// TODO: �ő�m�[�g�`�搔���ςɂ���

//******************************************************************************
// ���C�u���j�^�p�m�[�g�{�b�N�X�`��N���X
//******************************************************************************
class MTNoteBoxLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteBoxLive(void);
	virtual ~MTNoteBoxLive(void);
	
	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			MTNotePitchBend* pNotePitchBend
		);
	
	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, float rollAngle);
	
	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);
	
	//���
	void Release();
	
	//���Z�b�g
	void Reset();
	
	//�m�[�gON�o�^
	void SetNoteOn(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo,
			unsigned char velocity
		);
	
	//�m�[�gOFF�o�^
	void SetNoteOff(
			unsigned char portNo,
			unsigned char chNo,
			unsigned char noteNo
		);
	
	//�S�m�[�gOFF
	void AllNoteOff();
	void AllNoteOffOnCh(unsigned char portNo, unsigned char chNo);
	
private:
	
	//�����m�[�g���\����
	struct NoteStatus {
		bool isActive;
		unsigned char portNo;
		unsigned char chNo;
		unsigned char noteNo;
		unsigned long startTime;
		unsigned long endTime;
	};
	
	//���_�o�b�t�@�\����
	struct MTNOTEBOX_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
	};
	
private:
	
	//�m�[�g�f�U�C��
	MTNoteDesign m_NoteDesign;
	
	//�m�[�g�{�b�N�X
	DXPrimitive m_PrimitiveNotes;
	unsigned long m_NoteNum;
	NoteStatus* m_pNoteStatus;
	
	//�s�b�`�x���h���
	MTNotePitchBend* m_pNotePitchBend;
	
	//���C�u���j�^�\�����ԁi�~���b�j
	unsigned long m_LiveMonitorDisplayDuration;
	
	//���_�o�b�t�@FVF�t�H�[�}�b�g
	unsigned long _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE); }
	
	int _CreateNoteBox(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateNoteStatus();
	
	int _CreateVertexOfNote(
			NoteStatus noteStatus,
			MTNOTEBOX_VERTEX* pVertex,
			unsigned long vertexOffset,
			unsigned long* pIbIndex,
			unsigned long curTime,
			bool isEnablePitchBend = false
		);
	unsigned long _GetVertexIndexOfNote(unsigned long index);
	
	void _MakeMaterial(D3DMATERIAL9* pMaterial);
	
	int _TransformNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateStatusOfNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	int _UpdateVertexOfNotes(LPDIRECT3DDEVICE9 pD3DDevice);
	
	void _ClearOldestNoteStatus(unsigned long* pCleardIndex);
	
};


