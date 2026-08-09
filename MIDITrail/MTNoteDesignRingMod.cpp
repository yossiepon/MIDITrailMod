//******************************************************************************
//
// MIDITrail / MTNoteDesignRingMod
//
// �m�[�g�f�U�C�������OMod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTNoteDesignRingMod.h"
#include "DXH.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteDesignRingMod::MTNoteDesignRingMod(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteDesignRingMod::~MTNoteDesignRingMod(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTNoteDesignRingMod::Initialize(
	const TCHAR* pSceneName,
	SMSeqData* pSeqData
)
{
	int result = 0;

	result = MTNoteDesignRing::Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = MTNoteDesignMod::Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�{�b�N�X���S���W�擾
//******************************************************************************
Vector3 MTNoteDesignRingMod::GetNoteBoxCenterPosX(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	short pitchBendValue,				//�ȗ��F�s�b�`�x���h
	unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x
)
{
	return MTNoteDesignRing::GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);
}

//******************************************************************************
// �m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesignRingMod::GetNoteBoxVirtexPos(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	Vector3* pVector0,	//YZ����+X�����������č���
	Vector3* pVector1,	//YZ����+X�����������ĉE��
	Vector3* pVector2,	//YZ����+X�����������č���
	Vector3* pVector3,	//YZ����+X�����������ĉE��
	short pitchBendValue,				//�ȗ��F�s�b�`�x���h
	unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x

)
{
	MTNoteDesignRing::GetNoteBoxVirtexPos(
			curTickTime, portNo, chNo, noteNo, pVector0, pVector1, pVector2, pVector3, pitchBendValue, pitchBendSensitivity);
}

//******************************************************************************
// �������m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesignRingMod::GetActiveNoteBoxVirtexPos(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	Vector3* pVector0,	//YZ����+X�����������č���
	Vector3* pVector1,	//YZ����+X�����������ĉE��
	Vector3* pVector2,	//YZ����+X�����������č���
	Vector3* pVector3,	//YZ����+X�����������ĉE��
	short pitchBendValue,				//�ȗ��F�s�b�`�x���h
	unsigned char pitchBendSensitivity,	//�ȗ��F�s�b�`�x���h���x
	unsigned long elapsedTime			//�ȗ��F�o�ߎ��ԁi�~���b�j
)
{
	MTNoteDesignRing::GetActiveNoteBoxVirtexPos(
			curTickTime, portNo, chNo, noteNo, pVector0, pVector1, pVector2, pVector3, pitchBendValue, pitchBendSensitivity, elapsedTime);
}

//******************************************************************************
// �������m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesignRingMod::GetActiveNoteBoxVirtexPos(
	unsigned long curTickTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	Vector3* pVector0,	//YZ����+X�����������č���
	Vector3* pVector1,	//YZ����+X�����������ĉE��
	Vector3* pVector2,	//YZ����+X�����������č���
	Vector3* pVector3,	//YZ����+X�����������ĉE��
	short pitchBendValue,				//�ȗ��F�s�b�`�x���h
	unsigned char pitchBendSensitivity,	//�ȗ��F�s�b�`�x���h���x
	float rate							//�ȗ��F�T�C�Y�䗦
)
{
	Vector3 basePos0;
	Vector3 basePos1;
	Vector3 basePos2;
	float angle0 = 0.0f;
	float angle1 = 0.0f;
	float angle2 = 0.0f;
	float curSizeRatio = 1.0f;

	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (MTNoteDesignMod::m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, MTNOTEDESIGN_DECAY_SATURATION_SMOOTH);
	}

	//�m�[�g����W
	basePos0 = _GetNoteBasePos(curTickTime, portNo, chNo);
	basePos1 = basePos0;
	basePos1.y -= MTNoteDesignMod::GetNoteBoxWidth() * curSizeRatio / 2.0f;
	basePos2 = basePos0;
	basePos2.y += MTNoteDesignMod::GetNoteBoxWidth() * curSizeRatio / 2.0f;

	//�m�[�g�ԍ��Ŋp�x������
	angle0 = _GetNoteAngle(noteNo, pitchBendValue, pitchBendSensitivity);
	angle1 = angle0 - (m_NoteAngleStep * curSizeRatio / 2.0f);
	angle2 = angle0 + (m_NoteAngleStep * curSizeRatio / 2.0f);

	*pVector0 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle1);
	*pVector1 = DXH::RotateYZ(0.0f, 0.0f, basePos2, angle2);
	*pVector2 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle1);
	*pVector3 = DXH::RotateYZ(0.0f, 0.0f, basePos1, angle2);
}

//******************************************************************************
// ���C�u���j�^�p�m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesignRingMod::GetNoteBoxVirtexPosLive(
	unsigned long elapsedTime,
	unsigned char portNo,
	unsigned char chNo,
	unsigned char noteNo,
	Vector3* pVector0,	//YZ����+X�����������č���
	Vector3* pVector1,	//YZ����+X�����������ĉE��
	Vector3* pVector2,	//YZ����+X�����������č���
	Vector3* pVector3,	//YZ����+X�����������ĉE��
	short pitchBendValue,				//�ȗ��F�s�b�`�x���h
	unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x
)
{
	MTNoteDesignRing::GetNoteBoxVirtexPosLive(
		elapsedTime, portNo, chNo, noteNo, pVector0, pVector1, pVector2, pVector3, pitchBendValue, pitchBendSensitivity);
}

//******************************************************************************
// �|�[�g���_Y���W�擾
//******************************************************************************
float MTNoteDesignRingMod::GetPortOriginY(
	unsigned char portNo
)
{
	return MTNoteDesignRing::GetPortOriginY(portNo);
}

//******************************************************************************
// �|�[�g���_Z���W�擾
//******************************************************************************
float MTNoteDesignRingMod::GetPortOriginZ(
	unsigned char portNo
)
{
	return MTNoteDesignRing::GetPortOriginZ(portNo);
}

//******************************************************************************
// ���E���W�z�u�ړ��x�N�g���擾
//******************************************************************************
Vector3 MTNoteDesignRingMod::GetWorldMoveVector()
{
	return MTNoteDesignRing::GetWorldMoveVector();
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTNoteDesignRingMod::_LoadConfFile(
	const TCHAR* pSceneName
)
{
	int result = 0;

	result = MTNoteDesignRing::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = MTNoteDesignMod::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


