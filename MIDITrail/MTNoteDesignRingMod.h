//******************************************************************************
//
// MIDITrail / MTNoteDesignRingMod
//
// �m�[�g�f�U�C�������OMod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesignMod.h"
#include "MTNoteDesignRing.h"

using namespace SMIDILib;


//******************************************************************************
// �m�[�g�f�U�C�������OMod�N���X
//******************************************************************************
class MTNoteDesignRingMod : public MTNoteDesignMod, public MTNoteDesignRing
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteDesignRingMod(void);
	virtual ~MTNoteDesignRingMod(void);

	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�m�[�g�{�b�N�X���S���W�擾
	virtual DirectX::SimpleMath::Vector3 GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
	);

	//�m�[�g�{�b�N�X���_���W�擾
	virtual void GetNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		DirectX::SimpleMath::Vector3* pVector0,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector1,	//YZ����+X�����������ĉE��
		DirectX::SimpleMath::Vector3* pVector2,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
	);

	//�������m�[�g�{�b�N�X���_���W�擾
	virtual void GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		DirectX::SimpleMath::Vector3* pVector0,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector1,	//YZ����+X�����������ĉE��
		DirectX::SimpleMath::Vector3* pVector2,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity = 0,	//�ȗ��F�s�b�`�x���h���x
		unsigned long elapsedTime = 0            //�ȗ��F�o�ߎ��ԁi�~���b�j
	);

	//�������m�[�g�{�b�N�X���_���W�擾
	virtual void GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		DirectX::SimpleMath::Vector3* pVector0,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector1,	//YZ����+X�����������ĉE��
		DirectX::SimpleMath::Vector3* pVector2,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity = 0,	//�ȗ��F�s�b�`�x���h���x
		float rate = 0.0f						//�ȗ��F�T�C�Y�䗦
	);

	//���C�u���j�^�p�m�[�g�{�b�N�X���_���W�擾
	virtual void GetNoteBoxVirtexPosLive(
		unsigned long elapsedTime,	//�o�ߎ��ԁi�~���b�j
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		DirectX::SimpleMath::Vector3* pVector0,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector1,	//YZ����+X�����������ĉE��
		DirectX::SimpleMath::Vector3* pVector2,	//YZ����+X�����������č���
		DirectX::SimpleMath::Vector3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
	);

	//�|�[�g���_���W�擾
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//���E���W�z�u�ړ��x�N�g���擾
	virtual DirectX::SimpleMath::Vector3 GetWorldMoveVector();

private:

	virtual int _LoadConfFile(const TCHAR* pSceneName);
};


