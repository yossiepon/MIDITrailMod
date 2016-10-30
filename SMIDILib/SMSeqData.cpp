//******************************************************************************
//
// Simple Base Library / SMSeqData
//
// �V�[�P���X�f�[�^�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMFPUCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMSeqData::SMSeqData()
{
	m_pMergedTrack = NULL;
	Clear();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMSeqData::~SMSeqData(void)
{
	Clear();
}

//******************************************************************************
// SMF�t�H�[�}�b�g�o�^
//******************************************************************************
void SMSeqData::SetSMFFormat(
		unsigned long smfFormat
	)
{
	m_SMFFormat = smfFormat;
}

//******************************************************************************
// ����\�o�^
//******************************************************************************
void SMSeqData::SetTimeDivision(
		unsigned long timeDivision
	)
{
	m_TimeDivision = timeDivision;
}

//******************************************************************************
// �g���b�N�o�^
//******************************************************************************
int SMSeqData::AddTrack(
		SMTrack* pTrack
	)
{
	m_TrackList.push_back(pTrack);
	return 0;
}

//******************************************************************************
// �g���b�N�o�^����
//******************************************************************************
int SMSeqData::CloseTrack()
{
	int result = 0;

	//�g���b�N�}�[�W����
	result = _MergeTracks();
	if (result != 0) goto EXIT;

	//���v���t���ԎZ�o
	result = _CalcTotalTime();
	if (result != 0) goto EXIT;

	//�e���|�擾
	result = _GetTempo(&m_Tempo);
	if (result != 0) goto EXIT;

	//���q�L���擾
	result = _GetBeat(&m_BeatNumerator, &m_BeatDenominator);
	if (result != 0) goto EXIT;

	//���ߐ��擾
	result = _GetBarNum(&m_BarNum);
	if (result != 0) goto EXIT;

	//�e�L�X�g���擾
	result = _SearchText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �g���b�N�}�[�W����
//******************************************************************************
int SMSeqData::_MergeTracks()
{
	int result = 0;
	unsigned long i = 0;
	unsigned char portNo = 0;
	SMTrackListItr trackListItr;
	SMDeltaTimeBuf deltaTimeBuf;
	SMDeltaTimeBufList deltaTimeBufList;
	SMDeltaTimeBufListItr deltaTimeBufListItr;
	SMEvent event;
	SMTrack* pTrack = NULL;
	SMTrack* pMergedTrack = NULL;

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

	try {
		pMergedTrack = new SMTrack();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�f���^�^�C���o�b�t�@���X�g�̍쐬
	for (trackListItr = m_TrackList.begin(); trackListItr != m_TrackList.end(); trackListItr++) {
		pTrack = *trackListItr;
		if (pTrack->GetSize() == 0) continue;

		deltaTimeBuf.index = 0;
		result = pTrack->GetDataSet(0, &deltaTimeBuf.deltaTime, NULL, NULL);
		if (result != 0) goto EXIT;

		deltaTimeBufList.push_back(deltaTimeBuf);
	}

	//�}�[�W����
	while (true) {

		//�e�g���b�N���Q�Ƃ��čł��f���^�^�C�����Z���C�x���g���擾����
		unsigned long deltaTimeMin = 0xFFFFFFFF;
		unsigned long targetTrackIndex = 0;
		bool isDataExist = false;

		trackListItr = m_TrackList.begin();
		deltaTimeBufListItr = deltaTimeBufList.begin();
		for (i = 0; i < m_TrackList.size(); i++) {

			pTrack = *trackListItr;                //�J�����g�g���b�N
			deltaTimeBuf = *deltaTimeBufListItr;   //�J�����g�g���b�N�̃f���^�^�C�����

			//�g���b�N��ǂݏI����Ă��Ȃ���΃f���^�^�C�����Q�Ƃ���
			if (deltaTimeBuf.index < pTrack->GetSize()) {
				//�ŏ��f���^�^�C���̃g���b�N���}�[�N����
				if (deltaTimeBuf.deltaTime < deltaTimeMin) {
					targetTrackIndex = i;
					deltaTimeMin = deltaTimeBuf.deltaTime ;
				}
				isDataExist = true;
			}
			//���̃g���b�N
			trackListItr++;
			deltaTimeBufListItr++;
		}

		//�C�x���g�����݂��Ȃ���΃}�[�W����
		if (!isDataExist) break;

		//�e�g���b�N�̃f���^�^�C�����X�V����
		trackListItr = m_TrackList.begin();
		deltaTimeBufListItr = deltaTimeBufList.begin();
		for (i = 0; i < m_TrackList.size(); i++) {

			pTrack = *trackListItr;               //�J�����g�g���b�N
			deltaTimeBuf = *deltaTimeBufListItr;  //�J�����g�g���b�N�̃f���^�^�C�����

			//�}�[�N�����g���b�N�̓C�x���g���R�s�[���ă}�[�W�g���b�N�ɓo�^
			if (i == targetTrackIndex) {
				result = pTrack->GetDataSet(deltaTimeBuf.index, NULL, &event, &portNo);
				if (result != 0) goto EXIT;

				result = pMergedTrack->AddDataSet(deltaTimeMin, &event, portNo);
				if (result != 0) goto EXIT;

				//�}�[�N�����g���b�N�̎��̃f���^�^�C�����擾����
				deltaTimeBuf.index += 1;
				deltaTimeBuf.deltaTime = 0xFFFFFFFF;
				if (deltaTimeBuf.index < pTrack->GetSize()) {
					result = pTrack->GetDataSet(deltaTimeBuf.index, &deltaTimeBuf.deltaTime, NULL, NULL);
					if (result != 0) goto EXIT;
				}
			}
			//����ȊO�̃g���b�N�̓f���^�^�C�������Z����
			else if (deltaTimeBuf.index < pTrack->GetSize()) {
				deltaTimeBuf.deltaTime -= deltaTimeMin;
			}
			*deltaTimeBufListItr = deltaTimeBuf;

			//���̃g���b�N
			trackListItr++;
			deltaTimeBufListItr++;
		}
	}

	m_pMergedTrack = pMergedTrack;

EXIT:;
	if (result != NULL) {
		delete pMergedTrack;
		pMergedTrack = NULL;
	}
	return result;
}

//******************************************************************************
// �f�[�^�N���A
//******************************************************************************
void SMSeqData::Clear()
{
	SMTrackListItr itr;

	m_SMFFormat = 0;
	m_TimeDivision = 0;
	m_TotalTickTime = 0;
	m_TotalPlayTime = 0;
	m_Tempo = SM_DEFAULT_TEMPO;
	m_BeatNumerator = SM_DEFAULT_TIME_SIGNATURE_NUMERATOR;
	m_BeatDenominator = SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;
	m_BarNum = 0;
	m_CopyRight = "";
	m_Title = "";

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

	for (itr = m_TrackList.begin(); itr != m_TrackList.end(); itr++) {
		delete *itr;
		*itr = NULL;
	}
	m_TrackList.clear();

	return;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// �V�[�P���X�ǉ�
//******************************************************************************
void SMSeqData::AddSequence(SMSeqData &other, short portNo, short chNo)
{
	SMTrackListItr itr = other.m_TrackList.begin();
	std::advance(itr, 1);

	for (; itr != other.m_TrackList.end(); itr++) {

		(*itr)->OverwritePortNo(portNo);
		(*itr)->OverwriteChNo(chNo);

		m_TrackList.push_back(*itr);
	}

	other.m_TrackList.clear();

	CloseTrack();

	return;
}

// <<< add 20120728 yossiepon end

//******************************************************************************
// SMF�t�H�[�}�b�g�擾
//******************************************************************************
unsigned long SMSeqData::GetSMFFormat()
{
	return m_SMFFormat;
}

//******************************************************************************
// ����\�擾
//******************************************************************************
unsigned long SMSeqData::GetTimeDivision()
{
	return m_TimeDivision;
}

//******************************************************************************
// �g���b�N���擾
//******************************************************************************
unsigned long SMSeqData::GetTrackNum()
{
	return m_TrackList.size();
}

//******************************************************************************
// �g���b�N�擾
//******************************************************************************
int SMSeqData::GetTrack(
		unsigned long index,
		SMTrack* pTrack
	)
{
	int result = 0;
	SMTrackListItr itr;
	SMTrack *pSrcTrack;

	if (pTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (index >= GetTrackNum()) {
		result = YN_SET_ERR("Program error.", index, GetTrackNum());
		goto EXIT;
	}

	itr = m_TrackList.begin();
	advance(itr, index);
	pSrcTrack = *itr;

	result = pTrack->CopyFrom(pSrcTrack);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �}�[�W�g���b�N�擾
//******************************************************************************
int SMSeqData::GetMergedTrack(
		SMTrack* pMergedTrack
	)
{
	int result = 0;

	if (pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = pMergedTrack->CopyFrom(m_pMergedTrack);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���v�`�b�N�^�C���擾
//******************************************************************************
unsigned long SMSeqData::GetTotalTickTime()
{
	return m_TotalTickTime;
}

//******************************************************************************
// ���v���t���Ԏ擾�imsec.�j
//******************************************************************************
unsigned long SMSeqData::GetTotalPlayTime()
{
	return m_TotalPlayTime;
}

//******************************************************************************
// �e���|�擾(��sec.)
//******************************************************************************
unsigned long SMSeqData::GetTempo()
{
	return m_Tempo;
}

//******************************************************************************
// �e���|�擾(BPM)
//******************************************************************************
unsigned long SMSeqData::GetTempoBPM()
{
	return ((60 * 1000 * 1000) / m_Tempo);
}

//******************************************************************************
// ���q�L���擾�F���q
//******************************************************************************
unsigned long SMSeqData::GetBeatNumerator()
{
	return m_BeatNumerator;
}

//******************************************************************************
// ���q�L���擾�F����
//******************************************************************************
unsigned long SMSeqData::GetBeatDenominator()
{
	return m_BeatDenominator;
}

//******************************************************************************
// ���ߐ��擾
//******************************************************************************
unsigned long SMSeqData::GetBarNum()
{
	return m_BarNum;
}

//******************************************************************************
// ���쌠�e�L�X�g�擾
//******************************************************************************
const char* SMSeqData::GetCopyRight()
{
	return m_CopyRight.c_str();
}

//******************************************************************************
// �^�C�g���e�L�X�g�擾
//******************************************************************************
const char* SMSeqData::GetTitle()
{
	return m_Title.c_str();
}

//******************************************************************************
// ���v���t���ԎZ�o
//******************************************************************************
int SMSeqData::_CalcTotalTime()
{
	int result = 0;	
	unsigned long tempo = 0;
	unsigned long deltaTime = 0;
	unsigned long index = 0;
	double totalPlayTime = 0.0f;
	SMEvent event;
	SMEventMeta metaEvent;
	SMFPUCtrl fpuCtrl;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���������_���Z���x��{���x�ɐݒ�
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	tempo = SM_DEFAULT_TEMPO;
	m_TotalTickTime = 0;
	m_TotalPlayTime = 0;

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		//�g���b�N����f�[�^�Z�b�g�擾
		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		//�f���^�^�C���������Ԃɕϊ����ĉ��t���Ԃɉ��Z
		//  1msec������؂�̂Ă�ƌ덷���~�ς��邽��double�ŐώZ����
		m_TotalTickTime += deltaTime;
		totalPlayTime += _GetDeltaTimeMsec(tempo, deltaTime);

		//���^�C�x���g�����ꂽ��e���|�̍X�V���m�F����
		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			if (metaEvent.GetType() == 0x51) {
				tempo = metaEvent.GetTempo();
			}
		}
	}

	m_TotalPlayTime = (unsigned long)totalPlayTime;

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �f���^�^�C���擾�i�~���b�j
//******************************************************************************
double SMSeqData::_GetDeltaTimeMsec(
		unsigned long tempo,
		unsigned long deltaTime
	)
{
	double deltaTimeMsec = 0;

	//(1) �l������������̕���\ division
	//    ��F48
	//(2) �g���b�N�f�[�^�̃f���^�^�C�� delta
	//    ����\�̒l��p���ĕ\�����鎞�ԍ�
	//    ����\��48�Ńf���^�^�C����24�Ȃ甪���������̎��ԍ�
	//(3) �e���|�ݒ�i�}�C�N���b�j tempo
	//    �l�������̎����ԊԊu
	//
	// �f���^�^�C���ɑΉ���������ԊԊu�i�~���b�j
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)

	deltaTimeMsec = ((double)deltaTime * (double)tempo) / (1000.0 * (double)m_TimeDivision);

	return deltaTimeMsec;
}

//******************************************************************************
// �e���|�擾
//******************************************************************************
int SMSeqData::_GetTempo(
		unsigned long* pTempo
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	SMEventMeta metaEvent;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//MIDI�d�l�ɂ����ăe���|�̃f�t�H���g��BPM120 = 500msec = 500,000��sec
	*pTempo = SM_DEFAULT_TEMPO;

	//�V�[�P���X�̐擪�i�f���^�^�C���[���j����e���|������
	//������Ȃ���΃f�t�H���g�l���̗p�����
	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (deltaTime != 0) break;

		//���^�C�x���g�ȊO�͖���
		if (event.GetType() != SMEvent::EventMeta) continue;

		//���q�L�����擾
		metaEvent.Attach(&event);
		if (metaEvent.GetType() == 0x51) {
			*pTempo = metaEvent.GetTempo();
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���q�L���擾
//******************************************************************************
int SMSeqData::_GetBeat(
		unsigned long* pNumerator,
		unsigned long* pDenominator
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	SMEventMeta metaEvent;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//MIDI�d�l�ɂ����Ĕ��q�L���̃f�t�H���g��4/4
	*pNumerator   = SM_DEFAULT_TIME_SIGNATURE_NUMERATOR;
	*pDenominator = SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;

	//�V�[�P���X�̐擪�i�f���^�^�C���[���j���甏�q�L��������
	//������Ȃ���΃f�t�H���g�l���̗p�����
	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (deltaTime != 0) break;

		//���^�C�x���g�ȊO�͖���
		if (event.GetType() != SMEvent::EventMeta) continue;

		//���q�L�����擾
		metaEvent.Attach(&event);
		if (metaEvent.GetType() == 0x58) {
			metaEvent.GetTimeSignature(pNumerator, pDenominator);
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���ߐ��擾
//******************************************************************************
int SMSeqData::_GetBarNum(
		unsigned long* pBarNum
	)
{
	int result = 0;
	SMBarList barList;

	result = GetBarList(&barList);
	if (result != 0) goto EXIT;

	*pBarNum = barList.GetSize();

EXIT:;
	return result;
}

//******************************************************************************
// �e�L�X�g��񌟍�
//******************************************************************************
int SMSeqData::_SearchText()
{
	int result = 0;	
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	bool isFoundText = false;
	SMTrackListItr itr;
	SMTrack* pTrack = NULL;
	SMEvent event;
	SMEventMeta metaEvent;

	//�g���b�N�����݂��Ȃ���Ή������Ȃ�
	if (m_TrackList.size() == 0) goto EXIT;

	//��1�g���b�N(Conductor Track)���Q�Ƃ���
	itr = m_TrackList.begin();
	pTrack = *itr;

	//���쌠�\��������
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		//���쌠�\���̓f���^�^�C���[���ɋL�^�����
		if (deltaTime != 0) break;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			if (metaEvent.GetType() == 0x02) {
				result = metaEvent.GetText(&m_CopyRight);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}

	//�V�[�P���X��������
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			//�C�Ӄe�L�X�g
			if ((metaEvent.GetType() == 0x01) && (!isFoundText)) {
				result = metaEvent.GetText(&m_Title);
				if (result != 0) goto EXIT;

				//�V�[�P���X����D�悷��̂Ō����͌p������
				isFoundText = true;
			}
			//�V�[�P���X��
			if (metaEvent.GetType() == 0x03) {
				result = metaEvent.GetText(&m_Title);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���߃��X�g�擾
//******************************************************************************
int SMSeqData::GetBarList(
		SMBarList* pBarList
	)
{
	int result = 0;	
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	unsigned long prevBarTime = 0;
	unsigned long nextBarTime = 0;
	unsigned long totalTickTime = 0;
	unsigned long numerator = 0;
	unsigned long denominator = 0;
	unsigned long tickTimeOfBar = 0;
	SMEvent event;

	if (pBarList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pBarList->Clear();

	//1���߂�����̃`�b�N�^�C��
	tickTimeOfBar = (SM_DEFAULT_TIME_SIGNATURE_NUMERATOR * m_TimeDivision * 4) / SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;

	//1���ߖڊJ�n�n�_�Ƃ��ēo�^
	totalTickTime = 0;
	prevBarTime = totalTickTime;
	result = pBarList->AddBar(totalTickTime);
	if (result != 0) goto EXIT;

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {
		SMEventMeta metaEvent;

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		totalTickTime += deltaTime;

		//�o�ߎ��ԓ��ŏ��߂̋�؂�������ēo�^����
		while(true) {
			nextBarTime = prevBarTime + tickTimeOfBar;
			if (nextBarTime <= totalTickTime) {
				pBarList->AddBar(nextBarTime);
				prevBarTime = nextBarTime;
			}
			else {
				break;
			}
		}

		//�ȍ~�͔��q�L�������ꂽ�ꍇ�̑Ή�

		//���^�C�x���g�ȊO�͖���
		if (event.GetType() != SMEvent::EventMeta) continue;

		//���q�L���ȊO�͖���
		metaEvent.Attach(&event);
		if (metaEvent.GetType() != 0x58) continue;

		//���q�L�����擾
		metaEvent.GetTimeSignature(&numerator, &denominator);
		if (denominator == 0) {
			//�f�[�^�ُ�
			result = YN_SET_ERR("Invalid data found.", index, numerator);
			goto EXIT;
		}

		//1���߂�����̃`�b�N�^�C�����X�V
		tickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

		//���q�L���X�V�̂���1���ߖڊJ�n�n�_�Ƃ��ēo�^
		if (prevBarTime != totalTickTime) {
			prevBarTime = totalTickTime;
			result = pBarList->AddBar(totalTickTime);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g���X�g�擾
//******************************************************************************
int SMSeqData::GetPortList(
		SMPortList* pPortList
	)
{
	int result = 0;	
	unsigned long index = 0;
	unsigned char portNo = 0;
	unsigned char port[256];
	SMEvent event;

	if (pPortList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pPortList->Clear();

	for (index = 0; index < 256; index++) {
		port[index] = 0;
	}

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {
		result = m_pMergedTrack->GetDataSet(index, NULL, &event, &portNo);
		if (result != 0) goto EXIT;

		port[portNo] = 1;
	}

	for (index = 0; index < 256; index++) {
		if (port[index] != 0) {
			pPortList->AddPort((unsigned char)index);
		}
	}

EXIT:;
	return result;
}

} // end of namespace

