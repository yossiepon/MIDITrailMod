//******************************************************************************
//
// MIDITrail / MTNoteDesignRing
//
// �m�[�g�f�U�C�������O�N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTNoteDesign.h"

using namespace SMIDILib;


//******************************************************************************
// �m�[�g�f�U�C���N���X
//******************************************************************************
class MTNoteDesignRing : public MTNoteDesign
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteDesignRing(void);
	virtual ~MTNoteDesignRing(void);

	//���C�u���j�^���[�h�ݒ�
	void SetLiveMode(void);

	//�m�[�g�{�b�N�X���S���W�擾
	virtual D3DXVECTOR3 GetNoteBoxCenterPosX(
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
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
			);

	//�������m�[�g�{�b�N�X���_���W�擾
	virtual void GetActiveNoteBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0,	//�ȗ��F�s�b�`�x���h���x
				unsigned long elapsedTime = 0            //�ȗ��F�o�ߎ��ԁi�~���b�j
			);

	//���C�u���j�^�p�m�[�g�{�b�N�X���_���W�擾
	virtual void GetNoteBoxVirtexPosLive(
				unsigned long elapsedTime,	//�o�ߎ��ԁi�~���b�j
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
			);

	//�O���b�h�����O����W�擾
	void GetGridRingBasePos(
			unsigned long totalTickTime,
			D3DXVECTOR3* pBasePosStart,
			D3DXVECTOR3* pBasePosEnd
		);

	//���C�u���j�^�p�O���b�h�����O����W�擾
	void GetGridRingBasePosLive(
			D3DXVECTOR3* pBasePosStart,
			D3DXVECTOR3* pBasePosEnd
		);

	//�|�[�g���_���W�擾
	virtual float GetPortOriginY(unsigned char portNo);
	virtual float GetPortOriginZ(unsigned char portNo);

	//���E���W�z�u�ړ��x�N�g���擾
	virtual D3DXVECTOR3 GetWorldMoveVector();

// >>> modify access level to proteced 20191224 yossiepon begin
protected:
// <<< modify access level to proteced 20191224 yossiepon end

	bool m_isLiveMode;
	float m_NoteAngleStep;
	float m_RingRadius;

	// �m�[�g����W�擾
	D3DXVECTOR3 _GetNoteBasePos(
			unsigned long curTickTime,
			unsigned char portNo,
			unsigned char chNo
		);

	// �m�[�g�p�x�擾
	float _GetNoteAngle(
			unsigned char noteNo,
			short pitchBendValue,				//�ȗ��F�s�b�`�x���h
			unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x
		);

	virtual int _LoadConfFile(const TCHAR* pSceneName);

};


