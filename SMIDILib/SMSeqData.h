//******************************************************************************
//
// Simple MIDI Library / SMSeqData
//
// �V�[�P���X�f�[�^�N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMCommon.h"
#include "SMTrack.h"
#include "SMBarList.h"
#include "SMPortList.h"
#include <string>
#include <list>

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// �V�[�P���X�f�[�^�N���X
//******************************************************************************
class SMIDILIB_API SMSeqData
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMSeqData();
	virtual ~SMSeqData(void);

	//----------------------------------------------------------------
	//�f�[�^�쐬�n
	//----------------------------------------------------------------
	//SMF�t�H�[�}�b�g�o�^
	void SetSMFFormat(unsigned long smfFormat);

	//���ԉ𑜓x�o�^
	void SetTimeDivision(unsigned long timeDivision);

	//�g���b�N�o�^
	int AddTrack(SMTrack* pTrack);

	//�g���b�N�o�^�I��
	int CloseTrack();

	//�t�@�C�����o�^
	void SetFileName(const char* pFileName);

	//�N���A
	void Clear();

	//----------------------------------------------------------------
	//�f�[�^�擾�n
	//----------------------------------------------------------------
	//SMF�t�H�[�}�b�g�擾
	unsigned long GetSMFFormat();

	//���ԉ𑜓x�擾
	unsigned long GetTimeDivision();

	//�g���b�N���擾
	unsigned long GetTrackNum();

	//�g���b�N�擾
	int GetTrack(unsigned long index, SMTrack* pTrack);

	//�}�[�W�ς݃g���b�N�擾
	int GetMergedTrack(SMTrack* pMergedTrack);

	//�g�[�^���`�b�N�^�C���擾
	unsigned long GetTotalTickTime();

	//�g�[�^�����t���Ԏ擾
	unsigned long GetTotalPlayTime();

	//�e���|�擾
	unsigned long GetTempo();

	//�e���|�擾(BPM)
	unsigned long GetTempoBPM();

	//���q�L���擾�F���q�ƕ���
	unsigned long GetBeatNumerator();
	unsigned long GetBeatDenominator();

	//���ߐ��擾
	unsigned long GetBarNum();

	//�R�s�[���C�g������擾
	const char* GetCopyRight();

	//�^�C�g��������擾
	const char* GetTitle();

	//���߃��X�g�擾
	int GetBarList(SMBarList* pBarList);

	//�|�[�g���X�g�擾
	int GetPortList(SMPortList* pPortList);

	//�t�@�C�����擾
	const char* GetFileName();

private:

	typedef std::list<SMTrack*> SMTrackList;
	typedef std::list<SMTrack*>::iterator SMTrackListItr;

	typedef struct {
		unsigned long index;
		unsigned long deltaTime;
	} SMDeltaTimeBuf;

	typedef std::list<SMDeltaTimeBuf> SMDeltaTimeBufList;
	typedef std::list<SMDeltaTimeBuf>::iterator SMDeltaTimeBufListItr;

private:

	unsigned long m_SMFFormat;
	unsigned long m_TimeDivision;
	unsigned long m_TotalTickTime;
	unsigned long m_TotalPlayTime;
	unsigned long m_Tempo;
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;
	unsigned long m_BarNum;
	std::string m_CopyRight;
	std::string m_Title;
	std::string m_FileName;
	SMTrackList m_TrackList;
	SMTrack* m_pMergedTrack;

	int _MergeTracks();
	double _GetDeltaTimeMsec(unsigned long tempo, unsigned long deltaTime);
	int _GetTempo(unsigned long* pTempo);
	int _GetBeat(unsigned long* pNumerator, unsigned long* pDenominator);
	int _GetBarNum(unsigned long* pBarNum);
	int _CalcTotalTime();
	int _SearchText();

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMSeqData&);
	SMSeqData(const SMSeqData&);

};

} // end of namespace

#pragma warning(default:4251)

