//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign
//
// �s�A�m�L�[�{�[�h�f�U�C���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �L�[�{�[�h�̊�{�z�u���W
//
//  +y   +z
//  |    /
//  |   / +-#-#-+-#-#-#-+------
//  |  / / # # / # # # / ...
//  | / / / / / / / / / ...
//  |/ +-+-+-+-+-+-+-+------
// 0+------------------------ +x

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m�L�[�{�[�h�f�U�C���N���X
//******************************************************************************
class MTPianoKeyboardDesign
{
public:

	//�L�[���
	//  �����͔����Ɣ����̒��S��������ɂ���Ĕz�u����Ă���
	//  ���̂��ߔ����̌`��C����B�܂ł��ׂĈقȂ�
	enum KeyType {
		KeyWhiteC,	//����C
		KeyWhiteD,	//����D
		KeyWhiteE,	//����E
		KeyWhiteF,	//����F
		KeyWhiteG,	//����G
		KeyWhiteA,	//����A
		KeyWhiteB,	//����B
		KeyBlack	//����
	};

public:

	MTPianoKeyboardDesign(void);
	virtual ~MTPianoKeyboardDesign(void);

	//������
	int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�|�[�g���_���W�擾
	float GetPortOriginX(unsigned char portNo);
	float GetPortOriginY(unsigned char portNo);
	float GetPortOriginZ(unsigned char portNo);

	//�L�[��ʎ擾
	KeyType GetKeyType(unsigned char noteNo);

	//�L�[���SX���W�擾
	float GetKeyCenterPosX(unsigned char noteNo);

	//�����z�u�Ԋu�擾
	float GetWhiteKeyStep();

	//�������T�C�Y�擾
	float GetWhiteKeyWidth();

	//���������擾
	float GetWhiteKeyHeight();

	//���������擾
	float GetWhiteKeyLen();

	//�������T�C�Y�擾
	float GetBlackKeyWidth();

	//���������擾
	float GetBlackKeyHeight();

	//�����X�Β����擾
	float GetBlackKeySlopeLen();

	//���������擾
	float GetBlackKeyLen();

	//�L�[�Ԋu�T�C�Y�擾
	float GetKeySpaceSize();

	//�L�[������]���SY�����W�擾
	float GetKeyRotateAxisXPos();

	//�L�[������]�p�x
	float GetKeyRotateAngle();

	//�L�[���~���Ԏ擾(msec)
	unsigned long GetKeyDownDuration();

	//�L�[�㏸���Ԏ擾(msec)
	unsigned long GetKeyUpDuration();

	//�s�b�`�x���h�L�[�{�[�h�V�t�g�ʎ擾
	float GetPitchBendShift(short pitchBendValue, unsigned char pitchBendSensitivity);

	//�m�[�g�h���b�v���W�擾
	float GetNoteDropPosZ(unsigned char noteNo);

	//�����J���[�擾
	D3DXCOLOR GetWhiteKeyColor();

	//�����J���[�擾
	D3DXCOLOR GetBlackKeyColor();

	//�������L�[�J���[�擾
	D3DXCOLOR GetActiveKeyColor(unsigned char chNo, unsigned char noteNo, unsigned long elapsedTime);

	//�����e�N�X�`�����W�擾
	void GetWhiteKeyTexturePosTop(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos0,
			D3DXVECTOR2* pTexPos1,
			D3DXVECTOR2* pTexPos2,
			D3DXVECTOR2* pTexPos3,
			D3DXVECTOR2* pTexPos4,
			D3DXVECTOR2* pTexPos5,
			D3DXVECTOR2* pTexPos6,
			D3DXVECTOR2* pTexPos7
		);
	void GetWhiteKeyTexturePosFront(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos0,
			D3DXVECTOR2* pTexPos1,
			D3DXVECTOR2* pTexPos2,
			D3DXVECTOR2* pTexPos3
		);
	void GetWhiteKeyTexturePosSingleColor(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos
		);

	//�����e�N�X�`�����W�擾
	void GetBlackKeyTexturePos(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos0,
			D3DXVECTOR2* pTexPos1,
			D3DXVECTOR2* pTexPos2,
			D3DXVECTOR2* pTexPos3,
			D3DXVECTOR2* pTexPos4,
			D3DXVECTOR2* pTexPos5,
			D3DXVECTOR2* pTexPos6,
			D3DXVECTOR2* pTexPos7,
			D3DXVECTOR2* pTexPos8,
			D3DXVECTOR2* pTexPos9,
			bool isColored = false
		);
	void GetBlackKeyTexturePosSingleColor(
			unsigned char noteNo,
			D3DXVECTOR2* pTexPos,
			bool isColored = false
		);

	//�L�[�{�[�h����W�擾
	D3DXVECTOR3 GetKeyboardBasePos(unsigned char portNo, unsigned char chNo);

	//�L�[�{�[�h�ő�\�����擾
	unsigned long GetKeyboardMaxDispNum();

protected:

	virtual int _LoadConfFile(const TCHAR* pSceneName);


private:

	//�L�[���
	typedef struct {
		KeyType keyType;
		float keyCenterPosX;
	} MTKeyInfo;

private:

	//�L�[���z��
	MTKeyInfo m_KeyInfo[SM_MAX_NOTE_NUM];

	//�|�[�g���
	SMPortList m_PortList;
	unsigned char m_PortIndex[SM_MAX_PORT_NUM];

	//�X�P�[�����
	float m_WhiteKeyStep;
	float m_WhiteKeyWidth;
	float m_WhiteKeyHeight;
	float m_WhiteKeyLen;
	float m_BlackKeyWidth;
	float m_BlackKeyHeight;
	float m_BlackKeySlopeLen;
	float m_BlackKeyLen;
	float m_KeySpaceSize;
	float m_NoteDropPosZ4WhiteKey;
	float m_NoteDropPosZ4BlackKey;
	float m_BlackKeyShiftCDE;
	float m_BlackKeyShiftFGAB;

	//�L�[��]���
	float m_KeyRotateAxisXPos;
	float m_KeyRotateAngle;
	int m_KeyDownDuration;
	int m_KeyUpDuration;

	//�L�[�{�[�h�z�u���
	float m_KeyboardStepY;
	float m_KeyboardStepZ;
	int m_KeyboardMaxDispNum;

	//�L�[�F���
	D3DXCOLOR m_WhiteKeyColor;
	D3DXCOLOR m_BlackKeyColor;

	//�������L�[�F���
	D3DXCOLOR m_ActiveKeyColor[16];
	int m_ActiveKeyColorDuration;
	float m_ActiveKeyColorTailRate;

	void _Initialize();
	void _InitKeyType();
	void _InitKeyPos();

};


