//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign
//
// �s�A�m�L�[�{�[�h�f�U�C���N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
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

#include <directxtk/SimpleMath.h>
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

	//�������L�[�F���
	enum ActiveKeyColorType {
		DefaultColor,	//�f�t�H���g�F
		NoteColor		//�m�[�g�F
	};

public:

	MTPianoKeyboardDesign(void);
	virtual ~MTPianoKeyboardDesign(void);

	//������
// >>> modify 20120728 yossiepon begin
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);
// <<< modify 20120728 yossiepon end

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
	DirectX::SimpleMath::Color GetWhiteKeyColor();

	//�����J���[�擾
	DirectX::SimpleMath::Color GetBlackKeyColor();

	//�������L�[�J���[�擾
	DirectX::SimpleMath::Color GetActiveKeyColor(
			unsigned char noteNo,
			unsigned long elapsedTime,
			DirectX::SimpleMath::Color* pNoteColor = NULL
		);

	//�����e�N�X�`�����W�擾
	void GetWhiteKeyTexturePosTop(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos0,
			DirectX::SimpleMath::Vector2* pTexPos1,
			DirectX::SimpleMath::Vector2* pTexPos2,
			DirectX::SimpleMath::Vector2* pTexPos3,
			DirectX::SimpleMath::Vector2* pTexPos4,
			DirectX::SimpleMath::Vector2* pTexPos5,
			DirectX::SimpleMath::Vector2* pTexPos6,
			DirectX::SimpleMath::Vector2* pTexPos7
		);
	void GetWhiteKeyTexturePosFront(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos0,
			DirectX::SimpleMath::Vector2* pTexPos1,
			DirectX::SimpleMath::Vector2* pTexPos2,
			DirectX::SimpleMath::Vector2* pTexPos3
		);
	void GetWhiteKeyTexturePosSingleColor(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos
		);

	//�����e�N�X�`�����W�擾
	void GetBlackKeyTexturePos(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos0,
			DirectX::SimpleMath::Vector2* pTexPos1,
			DirectX::SimpleMath::Vector2* pTexPos2,
			DirectX::SimpleMath::Vector2* pTexPos3,
			DirectX::SimpleMath::Vector2* pTexPos4,
			DirectX::SimpleMath::Vector2* pTexPos5,
			DirectX::SimpleMath::Vector2* pTexPos6,
			DirectX::SimpleMath::Vector2* pTexPos7,
			DirectX::SimpleMath::Vector2* pTexPos8,
			DirectX::SimpleMath::Vector2* pTexPos9,
			bool isColored = false
		);
	void GetBlackKeyTexturePosSingleColor(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos,
			bool isColored = false
		);

	//�L�[�{�[�h����W�擾
	DirectX::SimpleMath::Vector3 GetKeyboardBasePos(unsigned char portNo, unsigned char chNo);

	//�L�[�{�[�h�ő�\�����擾
	unsigned long GetKeyboardMaxDispNum();

// >>> add 20180404 yossiepon begin
	void SetKeyboardSingle();
// <<< add 20180404 yossiepon end

	//�L�[�\���͈͎擾
	unsigned char GetKeyDispRangeStart();
	unsigned char GetKeyDispRangeEnd();
	bool IsKeyDisp(unsigned char noteNo);

private:

	//�L�[���
	typedef struct {
		KeyType keyType;
		float keyCenterPosX;
	} MTKeyInfo;

private:

	//�L�[���z��
	MTKeyInfo m_KeyInfo[SM_MAX_NOTE_NUM];

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	//�|�[�g���
	SMPortList m_PortList;
	unsigned char m_PortIndex[SM_MAX_PORT_NUM];

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

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

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	float m_KeyboardStepY;

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	float m_KeyboardStepZ;
	int m_KeyboardMaxDispNum;

	//�L�[�F���
	DirectX::SimpleMath::Color m_WhiteKeyColor;
	DirectX::SimpleMath::Color m_BlackKeyColor;

	//�������L�[�F���
	DirectX::SimpleMath::Color m_ActiveKeyColor;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

	int m_ActiveKeyColorDuration;
	float m_ActiveKeyColorTailRate;
	ActiveKeyColorType m_ActiveKeyColorType;

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	//�L�[�\���͈�
	int m_KeyDispRangeStart;
	int m_KeyDispRangeEnd;

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

// >>> modify 20120728 yossiepon begin
	virtual void _Initialize();
// <<< modify 20120728 yossiepon end

// >>> modify access level 20161224 yossiepon begin
private:
// <<< modify 20161224 yossiepon end

	void _InitKeyType();
	void _InitKeyPos();

// >>> modify access level to protected 20161224 yossiepon begin
protected:
// <<< modify 20161224 yossiepon end

// >>> modify 20120728 yossiepon begin
	virtual int _LoadConfFile(const TCHAR* pSceneName);
// <<< modify 20120728 yossiepon end

};


