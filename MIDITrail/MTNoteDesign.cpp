//******************************************************************************
//
// MIDITrail / MTNoteDesign
//
// �m�[�g�f�U�C���N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTNoteDesign.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteDesign::MTNoteDesign(void)
{
	_Clear();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteDesign::~MTNoteDesign(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTNoteDesign::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long portIndex = 0;
	unsigned char portNo = 0;

	//���C�u���j�^�����ݒ�
	if (pSeqData == NULL) {
		//����\
		m_TimeDivision = 48;
		//�|�[�g���X�g
		m_PortList.Clear();
		m_PortList.AddPort(0);
	}
	//�ʏ�ݒ�
	else {
		//����\�擾
		m_TimeDivision = pSeqData->GetTimeDivision();
		if (m_TimeDivision == 0) {
			result = YN_SET_ERR("Invalid data found.", 0, 0);
			goto EXIT;
		}
		//�|�[�g���X�g�擾
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
	}

	//�|�[�g�ԍ��ɏ����̃C���f�b�N�X��U��
	//�|�[�g 0�� 3�� 5�� �ɏo�͂���ꍇ�̃C���f�b�N�X�͂��ꂼ�� 0, 1, 2
	for (index = 0; index < 256; index++) {
		m_PortIndex[index] = 0;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = (unsigned char)portIndex;
		portIndex++;
	}

	//�p�����[�^�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;
	
	//���[�U�ݒ�ǂݍ���
	result = _LoadUserConf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���t�ʒu�擾
//******************************************************************************
float MTNoteDesign::GetPlayPosX(
		unsigned long curTickTime
	)
{
	return ((float)curTickTime * m_QuarterNoteLength / (float)m_TimeDivision);
}

//******************************************************************************
// ���C�u���j�^�p�m�[�g�ʒu�擾
//******************************************************************************
float MTNoteDesign::GetLivePosX(
		unsigned long elapsedTime
	)
{
	return (((float)elapsedTime / 1000.0f) * m_LiveNoteLengthPerSecond);
}

//******************************************************************************
// �m�[�g�{�b�N�X���S���W�擾
//******************************************************************************
D3DXVECTOR3 MTNoteDesign::GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x
	)
{
	D3DXVECTOR3 vector;
	float pb = 0.0f;

	//�s�b�`�x���h�ɂ��Y���W�̈ړ���
	if (pitchBendValue < 0) {
		pb = GetNoteStep() * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		pb = GetNoteStep() * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	//���t�ʒu
	vector.x = GetPlayPosX(curTickTime);

	//�m�[�g�ԍ�
	vector.y = GetPortOriginY(portNo) + (m_NoteStep * noteNo + pb);

	//�|�[�g�ԍ��ƃ`�����l���ԍ�
	vector.z = GetPortOriginZ(portNo) + (GetChStep() * chNo);

	return vector;
}

//******************************************************************************
// �m�[�g�{�b�N�X�c�T�C�Y�擾
//******************************************************************************
float MTNoteDesign::GetNoteBoxHeight()
{
	return m_NoteBoxHeight;
}

//******************************************************************************
// �m�[�g�{�b�N�X���T�C�Y�擾
//******************************************************************************
float MTNoteDesign::GetNoteBoxWidht()
{
	return m_NoteBoxWidth;
}

//******************************************************************************
// �m�[�g�Ԋu�擾
//******************************************************************************
float MTNoteDesign::GetNoteStep()
{
	return m_NoteStep;
}

//******************************************************************************
// �`�����l���Ԋu�擾
//******************************************************************************
float MTNoteDesign::GetChStep()
{
	return m_ChStep;
}

//******************************************************************************
//���C�u���j�^�\�����ԁi�~���b�j
//******************************************************************************
unsigned long MTNoteDesign::GetLiveMonitorDisplayDuration()
{
	return (unsigned long)m_LiveMonitorDisplayDuration;
}

//******************************************************************************
// �m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesign::GetNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x

	)
{
	D3DXVECTOR3 center;
	float bh = 0.0f;
	float bw = 0.0f;

	center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);

	bh = GetNoteBoxHeight();
	bw = GetNoteBoxWidht();

	*pVector0 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// �������m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesign::GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity,	//�ȗ��F�s�b�`�x���h���x
		unsigned long elapsedTime			//�ȗ��F�o�ߎ��ԁi�~���b�j
	)
{
	D3DXVECTOR3 center;
	float bh = 0.0f;
	float bw = 0.0f;
	float curSizeRatio = 1.0f;
	
	center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);
	
	if (elapsedTime < (unsigned long)m_ActiveNoteDuration) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * (1.0f - (float)elapsedTime / (float)m_ActiveNoteDuration);
	}
	
	bh = GetNoteBoxHeight() * curSizeRatio;
	bw = GetNoteBoxWidht() * curSizeRatio;
	
	*pVector0 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// ���C�u���j�^�p�m�[�g�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesign::GetNoteBoxVirtexPosLive(
		unsigned long elapsedTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
		short pitchBendValue,				//�ȗ��F�s�b�`�x���h
		unsigned char pitchBendSensitivity	//�ȗ��F�s�b�`�x���h���x
	)
{
	D3DXVECTOR3 center;
	float bh = 0.0f;
	float bw = 0.0f;
	float x = 0.0f;
	unsigned long tickTimeDummy = 0;
	
	center = GetNoteBoxCenterPosX(tickTimeDummy, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);
	
	x = -(GetLivePosX(elapsedTime));
	
	bh = GetNoteBoxHeight();
	bw = GetNoteBoxWidht();
	
	*pVector0 = D3DXVECTOR3(x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// �O���b�h�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesign::GetGridBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3 	//YZ����+X�����������ĉE��
	)
{
	float x = 0.0f;
	float bh = 0.0f;
	float bw = 0.0f;
	float gridHeight = 0.0f;
	float gridWidth = 0.0f;
	float oy = 0.0f;
	float oz = 0.0f;

	x = GetPlayPosX(curTickTime);

	bh = GetNoteBoxHeight();
	bw = GetNoteBoxWidht();

	gridHeight = GetNoteStep() * 127;
	gridWidth  = GetChStep() * 15;

	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);

	*pVector0 = D3DXVECTOR3(x, oy+gridHeight+(bh/2.0f), oz+gridWidth+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(x, oy+gridHeight+(bh/2.0f), oz          -(bw/2.0f));
	*pVector2 = D3DXVECTOR3(x, oy           -(bh/2.0f), oz+gridWidth+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(x, oy           -(bh/2.0f), oz          -(bw/2.0f));
}

//******************************************************************************
// �O���b�h�{�b�N�X���_���W�擾
//******************************************************************************
void MTNoteDesign::GetGridBoxVirtexPosLive(
		unsigned long elapsedTime,
		unsigned char portNo,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3 	//YZ����+X�����������ĉE��
	)
{
	float x = 0.0f;
	float bh = 0.0f;
	float bw = 0.0f;
	float gridHeight = 0.0f;
	float gridWidth = 0.0f;
	float oy = 0.0f;
	float oz = 0.0f;
	
	x = -(GetLivePosX(elapsedTime));
	
	bh = GetNoteBoxHeight();
	bw = GetNoteBoxWidht();
	
	gridHeight = GetNoteStep() * 127;
	gridWidth  = GetChStep() * 15;
	
	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);
	
	*pVector0 = D3DXVECTOR3(x, oy+gridHeight+(bh/2.0f), oz+gridWidth+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(x, oy+gridHeight+(bh/2.0f), oz          -(bw/2.0f));
	*pVector2 = D3DXVECTOR3(x, oy           -(bh/2.0f), oz+gridWidth+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(x, oy           -(bh/2.0f), oz          -(bw/2.0f));
}

//******************************************************************************
// �Đ��ʒ��_���W�擾
//******************************************************************************
void MTNoteDesign::GetPlaybackSectionVirtexPos(
		unsigned long curTickTime,
		D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
		D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
		D3DXVECTOR3* pVector3 	//YZ����+X�����������ĉE��
	)
{
	D3DXVECTOR3 firstPortVecotr[4];
	D3DXVECTOR3 finaltPortVecotr[4];
	unsigned char lastPortNo = 0;

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	GetGridBoxVirtexPos(
			curTickTime,
			0,
			&(firstPortVecotr[0]),
			&(firstPortVecotr[1]),
			&(firstPortVecotr[2]),
			&(firstPortVecotr[3])
		);
	GetGridBoxVirtexPos(
			curTickTime,
			lastPortNo,
			&(finaltPortVecotr[0]),
			&(finaltPortVecotr[1]),
			&(finaltPortVecotr[2]),
			&(finaltPortVecotr[3])
		);

	*pVector0 = finaltPortVecotr[0];
	*pVector1 = firstPortVecotr[1];
	*pVector2 = finaltPortVecotr[2];
	*pVector3 = firstPortVecotr[3];
}

//******************************************************************************
// �g��c�T�C�Y�擾
//******************************************************************************
float MTNoteDesign::GetRippleHeight(
		unsigned long elapsedTime	//�ȗ��F�o�ߎ��ԁi�~���b�j
	)
{
	float height = 0.0f;

	if ((int)elapsedTime <= m_RippleDuration) {
		height = m_RippleHeight * (1.0f - ((float)elapsedTime / m_RippleDuration));
	}

	return height;
}

//******************************************************************************
// �g�䉡�T�C�Y�擾
//******************************************************************************
float MTNoteDesign::GetRippleWidth(
		unsigned long elapsedTime	//�ȗ��F�o�ߎ��ԁi�~���b�j
	)
{
	float width = 0.0f;

	if ((int)elapsedTime <= m_RippleDuration) {
		width = m_RippleWidth * (1.0f - ((float)elapsedTime / m_RippleDuration));
	}

	return width;
}

//******************************************************************************
// �g�䓧���x�擾
//******************************************************************************
float MTNoteDesign::GetRippleAlpha(
		unsigned long elapsedTime	//�o�ߎ��ԁi�~���b�j
	)
{
	float alpha = 1.0f;

	if ((int)elapsedTime <= m_RippleDuration) {
		alpha = 1.0f - ((float)elapsedTime / m_RippleDuration);
	}

	return alpha;
}

//******************************************************************************
// �s�N�`���{�[�h���Έʒu�擾
//******************************************************************************
float MTNoteDesign::GetPictBoardRelativePos()
{
	return m_PictBoardRelativePos;
}

//******************************************************************************
// �|�[�g���_Y���W�擾
//******************************************************************************
float MTNoteDesign::GetPortOriginY(
		unsigned char portNo
	)
{
	//   +y
	//    |
	//    +-- Note#127
	//    |   Note#126
	//    |
	//    |
	// ---0----->+x(time)
	//    |
	//    |
	//    |   Note#1
	//    @-- Note#0  @:OriginY
	//    |
	//   -y

	return (0.0f - (GetNoteStep() * 127.0f / 2.0f));
}

//******************************************************************************
// �|�[�g���_Z���W�擾
//******************************************************************************
float MTNoteDesign::GetPortOriginZ(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portWidth = 0.0f;

	//                  +y
	//                   |
	//         portC   portB   portA
	//       +-------+-------+-------+Note#127
	//       |       |   |   |       |
	//       |       |   |   |       |
	//       |       |   |   |       |
	// +z<---|-------@---0---@-------@--------->-z
	//       |       |   |   |       |
	//       |       |   |   |       |  @:OriginZ(for portA,B,C)
	//       |       |   |   |       |
	//       +-------+-------+-------+Note#0
	//    Ch. 16    0 16 |  0 16    0
	//                   |
	//                  -y

	portIndex = (float)(m_PortIndex[portNo]);
	portWidth = GetChStep() * 16.0f;

	return ((portWidth * portIndex) - (portWidth * m_PortList.GetSize() / 2.0f));
}

//******************************************************************************
// ���E���W�z�u�ړ��x�N�g���擾
//******************************************************************************
D3DXVECTOR3 MTNoteDesign::GetWorldMoveVector()
{
	D3DXVECTOR3 vector;

	vector.x = 0.0f;
	vector.y = - GetPortOriginY(0);
	vector.z = - GetPortOriginZ(0);

	return vector;
}

//******************************************************************************
// �m�[�g�{�b�N�X�J���[�擾
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	D3DXCOLOR color;

	if (m_NoteColorType == Channel) {
		//�`�����l���ԍ��ɂ���ĐF��ς���
		if (chNo >= 16) {
			//�f�[�^�ُ킾����������
			color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
		}
		else {
			color = m_NoteColor[chNo];
		}
	}
	else if (m_NoteColorType == Scale) {
		//���K�ɂ���ĐF��ς���
		color = m_NoteColorOfScale[(noteNo % 12)];
	}

	return color;
}

//******************************************************************************
// �������m�[�g�{�b�N�X�J���[�擾
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetActiveNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned long elapsedTime
	)
{
	D3DXCOLOR color;
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
	float rate = 0.0f;

	color = GetNoteBoxColor(portNo, chNo, noteNo);

	//m_ActiveNoteDuration �����[�X�^�C��
	//  �����J�n���_����m�[�g�F�����ɖ߂��܂ł̎���
	//  ������ m_ActiveNoteEmissive �ɂ���ă����[�X����m�[�gOFF�܂Ŕ�������

	//m_ActiveNoteWhiteRate �ő唒�F��
	//  0.0 �� �m�[�g�F�ω��Ȃ�
	//  0.5 �� �m�[�g�F�Ɣ��̒��ԐF
	//  1.0 �� ��

	rate = 0.0f;
	if ((int)elapsedTime < m_ActiveNoteDuration) {
		rate = 1.0f - ((float)elapsedTime / (float)m_ActiveNoteDuration);
	}
	r = color.r + ((1.0f - color.r) * rate * m_ActiveNoteWhiteRate);
	g = color.g + ((1.0f - color.g) * rate * m_ActiveNoteWhiteRate);
	b = color.b + ((1.0f - color.b) * rate * m_ActiveNoteWhiteRate);
	a = color.a;
	color = D3DXCOLOR(r, g, b, a);

	return color;
}

//******************************************************************************
// �������m�[�g�{�b�N�X�G�~�b�V�u�擾�i�}�e���A���p�j
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetActiveNoteEmissive()
{
	return m_ActiveNoteEmissive;
}

//******************************************************************************
// �O���b�h���C���J���[�擾
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetGridLineColor()
{
	return m_GridLineColor;
}

//******************************************************************************
// �Đ��ʃJ���[�擾
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetPlaybackSectionColor()
{
	return m_PlaybackSectionColor;
}

//******************************************************************************
// �N���A
//******************************************************************************
void MTNoteDesign::_Clear(void)
{
	unsigned long i = 0;
	
	m_TimeDivision = 0;
	m_QuarterNoteLength = 0.0f;
	m_NoteBoxHeight = 0.0f;
	m_NoteBoxWidth = 0.0f;
	m_NoteStep = 0.0f;
	m_ChStep = 0.0f;
	m_RippleHeight = 0.0f;
	m_RippleWidth = 0.0f;
	m_PictBoardRelativePos = 0.0f;
	m_PortList.Clear();

	for (i = 0; i < 256; i++) {
		m_PortIndex[i] = 0;
	}

	m_NoteColorType = Channel;
	for (i = 0; i < 16; i++) {
		m_NoteColor[i] = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
	}
	for (i = 0; i < 12; i++) {
		m_NoteColorOfScale[i] = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
	}
	m_ActiveNoteEmissive   = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
	m_GridLineColor        = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
	m_PlaybackSectionColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA

	m_ActiveNoteDuration = 400;
	m_ActiveNoteWhiteRate = 1.0f;
	m_ActiveNoteBoxSizeRatio = 1.0f;
	m_RippleDuration = 1600;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTNoteDesign::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	TCHAR noteColorType[16] = {_T('\0')};
	unsigned long i = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�X�P�[�����
	//----------------------------------
	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("QuarterNoteLength"), &m_QuarterNoteLength, 1.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteBoxHeight"), &m_NoteBoxHeight, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteBoxWidth"), &m_NoteBoxWidth, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteStep"), &m_NoteStep, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("ChStep"), &m_ChStep, 0.5f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("RippleHeight"), &m_RippleHeight, 1.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("RippleWidth"), &m_RippleWidth, 1.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("PictBoardRelativePos"), &m_PictBoardRelativePos, 1.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("LiveNoteLengthPerSecond"), &m_LiveNoteLengthPerSecond, 2.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("LiveMonitorDisplayDuration"), &m_LiveMonitorDisplayDuration, 30000);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�F���
	//----------------------------------
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;

	//�m�[�g�J���[��ʂ��擾
	result = confFile.GetStr(_T("NoteColorType"), noteColorType, 16, _T("CHANNEL"));
	if (result != 0) goto EXIT;

	//�m�[�g�J���[��ʂ�����
	m_NoteColorType = Channel;
	if (_tcscmp(noteColorType, _T("SCALE")) == 0) {
		m_NoteColorType = Scale;
	}

	//�m�[�g�F�����擾
	for (i = 0; i < 16; i++) {
		_stprintf_s(key, 32, _T("Ch-%02d-NoteRGBA"), i+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;

		m_NoteColor[i] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	}

	//���K�p�m�[�g�F�����擾
	for (i = 0; i < 12; i++) {
		_stprintf_s(key, 32, _T("Scale-%02d-NoteRGBA"), i+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;

		m_NoteColorOfScale[i] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	}

	//�O���b�h���C���F�����擾
	result = confFile.GetStr(_T("GridLineRGBA"), hexColor, 16, _T("444444FF"));
	if (result != 0) goto EXIT;
	m_GridLineColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	//�Đ��ʐF�����擾
	result = confFile.GetStr(_T("PlaybackSectionRGBA"), hexColor, 16, _T("AAAAFFFF"));
	if (result != 0) goto EXIT;
	m_PlaybackSectionColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	//----------------------------------
	//�����m�[�g���
	//----------------------------------
	result = confFile.SetCurSection(_T("ActiveNote"));
	if (result != 0) goto EXIT;

	//�������m�[�g�F���F�p������(msec)
	result = confFile.GetInt(_T("Duration"), &m_ActiveNoteDuration, 400);
	if (result != 0) goto EXIT;

	//�������m�[�g�F���F���F��
	result = confFile.GetFloat(_T("WhiteRate"), &m_ActiveNoteWhiteRate, 0.9f);
	if (result != 0) goto EXIT;

	//�������m�[�g�F���F�}�e���A�������F
	result = confFile.GetStr(_T("EmissiveRGBA"), hexColor, 16, _T("1A1A1A1A"));
	if (result != 0) goto EXIT;
	m_ActiveNoteEmissive = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	//�������m�[�g���F�{�b�N�X�T�C�Y�䗦
	result = confFile.GetFloat(_T("SizeRatio"), &m_ActiveNoteBoxSizeRatio, 1.4f);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�g����
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	//�g��p������(msec)
	result = confFile.GetInt(_T("Duration"), &m_RippleDuration, 1600);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���[�U�ݒ�ǂݍ���
//******************************************************************************
int MTNoteDesign::_LoadUserConf()
{
	int result = 0;
	YNConfFile confFile;
	TCHAR userConfFilePath[_MAX_PATH] = { _T('\0') };
	int lengthMagnification = 0;
	
	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;
	
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);
	
	result = confFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;
	
	//�l���������g�嗦(0-1000)
	result = confFile.SetCurSection(_T("QuarterNote"));
	result = confFile.GetInt(_T("LengthMagnification"), &lengthMagnification, 100);
	
	//�N���b�s���O
	if (lengthMagnification < 0) {
		lengthMagnification = 0;
	}
	if (lengthMagnification > 1000) {
		lengthMagnification = 1000;
	}

	//�l�������̒������X�V
	m_QuarterNoteLength = m_QuarterNoteLength * ((float)lengthMagnification / 100.0f);

EXIT:;
	return result;
}

