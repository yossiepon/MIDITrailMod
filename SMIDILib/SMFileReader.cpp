//******************************************************************************
//
// Simple MIDI Library / SMFileReader
//
// �W��MIDI�t�@�C���ǂݍ��݃N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMFileReader.h"
#include "SMCommon.h"
#include "tchar.h"
#include "shlwapi.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMFileReader::SMFileReader(void)
{
	m_LogPath[0] = '\0';
	m_pLogFile = NULL;
	m_IsLogOut = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMFileReader::~SMFileReader(void)
{
}

//******************************************************************************
// ���O�o�̓p�X�ݒ�
//******************************************************************************
int SMFileReader::SetLogPath(
		const TCHAR* pLogPath
	)
{
	int result = 0;
	errno_t eresult = 0;

	m_IsLogOut = false;

	if (pLogPath == NULL) {
		m_LogPath[0] = '\0';
	}
	else {
		eresult = _tcscpy_s(m_LogPath, MAX_PATH, pLogPath);
		if (eresult != 0) {
			result = YN_SET_ERR("Program error.", 0, 0);
			goto EXIT;
		}
	}

	if (_tcslen(m_LogPath) > 0) {
		m_IsLogOut = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Standard MIDI File �̃��[�h
//******************************************************************************
int SMFileReader::Load(
		const TCHAR *pSMFPath,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long i = 0;
	HMMIO hFile = NULL;
	SMFChunkTypeSection chunkTypeSection;
	SMFChunkDataSection chunkDataSection;
	SMFChunkTypeSection chunkTypeSectionOfTrack;
	SMTrack* pTrack = NULL;

	if ((pSMFPath == NULL) || (pSeqData == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pSeqData->Clear();

	//���O�t�@�C�����J��
	result = _OpenLogFile();
	if (result != 0 ) goto EXIT;

	//�t�@�C�����J��
	hFile = mmioOpen((LPTSTR)pSMFPath, NULL, MMIO_READ);
	if (hFile == NULL) {
		result = YN_SET_ERR("File open error.", GetLastError(), 0);
		goto EXIT;
	}

	//�w�b�_�ǂݍ���
	result = _ReadChunkHeader(hFile, &chunkTypeSection, &chunkDataSection);
	if (result != 0 ) goto EXIT;

	if ((chunkDataSection.format != 0) && (chunkDataSection.format != 1)) {
		//�t�H�[�}�b�g0,1�ȊO�͖��Ή�
		result = YN_SET_ERR("Unsupported SMF format.", chunkDataSection.format, 0);
		goto EXIT;
	}
	if ( chunkDataSection.ntracks == 0) {
		//�f�[�^�ُ�
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if ( chunkDataSection.timeDivision == 0) {
		//�f�[�^�ُ�
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if ((chunkDataSection.timeDivision & 0x80000000) != 0) {
		//����\�����̏ꍇ�̓f���^�^�C���������ԂƂ݂Ȃ��d�l������
		//��ʓI�łȂ��̂ō��̂Ƃ���T�|�[�g���Ȃ�
		result = YN_SET_ERR("Unsupported SMF format.", chunkDataSection.timeDivision, 0);
		goto EXIT;
	}

	pSeqData->SetSMFFormat(chunkDataSection.format);
	pSeqData->SetTimeDivision(chunkDataSection.timeDivision);

	for (i = 0; i < chunkDataSection.ntracks; i++) {
		//�g���b�N�w�b�_�ǂݍ���
		result = _ReadTrackHeader(hFile, i, &chunkTypeSectionOfTrack);
		if (result != 0 ) goto EXIT;

		//�g���b�N�C�x���g�ǂݍ���
		result = _ReadTrackEvents(hFile, chunkTypeSectionOfTrack.chunkSize, &pTrack);
		if (result != 0 ) goto EXIT;

		result = pSeqData->AddTrack(pTrack);
		if (result != 0 ) goto EXIT;
		pTrack = NULL;
	}

	//�g���b�N�����
	result = pSeqData->CloseTrack();
	if (result != 0 ) goto EXIT;

	//�t�@�C�����o�^
	pSeqData->SetFileName(PathFindFileName(pSMFPath));

EXIT:;
	if (hFile != NULL) {
		mmioClose(hFile, 0);
		hFile = NULL;
	}
	_CloseLogFile();
	return result;
}

//******************************************************************************
// SMF�w�b�_�ǂݍ���
//******************************************************************************
int SMFileReader::_ReadChunkHeader(
		HMMIO hFile,
		SMFChunkTypeSection* pChunkTypeSection,
		SMFChunkDataSection* pChunkDataSection
	)
{
	int result = 0;
	long apiresult = 0;
	long offset = 0;

	//���ʎq�ƃw�b�_�f�[�^�T�C�Y�̓ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)pChunkTypeSection, sizeof(SMFChunkTypeSection));
	if (apiresult != sizeof(SMFChunkTypeSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//�G���f�B�A���ϊ�
	_ReverseEndian(&(pChunkTypeSection->chunkSize), sizeof(unsigned long));

	//�������`�F�b�N
	if (memcmp(pChunkTypeSection->chunkType, "MThd", 4) != 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if (pChunkTypeSection->chunkSize < sizeof(SMFChunkDataSection)) {
		result = YN_SET_ERR("Invalid data found.", pChunkTypeSection->chunkSize, 0);
		goto EXIT;
	}

	//�w�b�_�f�[�^�̓ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)pChunkDataSection, sizeof(SMFChunkDataSection));
	if (apiresult != sizeof(SMFChunkDataSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//�G���f�B�A���ϊ�
	_ReverseEndian(&(pChunkDataSection->format), sizeof(unsigned short));
	_ReverseEndian(&(pChunkDataSection->ntracks), sizeof(unsigned short));
	_ReverseEndian(&(pChunkDataSection->timeDivision), sizeof(unsigned short));

	//�w�肳�ꂽ�f�[�^�T�C�Y�܂ŃX�L�b�v����i�O�̂��߁j
	if (pChunkTypeSection->chunkSize > sizeof(SMFChunkDataSection)) {
		offset = pChunkTypeSection->chunkSize - sizeof(SMFChunkDataSection);
		apiresult = mmioSeek(hFile, offset, SEEK_CUR);
		if (apiresult == -1) {
			result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
			goto EXIT;
		}
	}

	result = _WriteLogChunkHeader(pChunkTypeSection, pChunkDataSection);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// SMF�g���b�N�w�b�_�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadTrackHeader(
		HMMIO hFile,
		unsigned long trackNo,
		SMFChunkTypeSection* pChunkTypeSection
	)
{
	int result = 0;
	long apiresult = 0;

	//���ʎq�ƃw�b�_�f�[�^�T�C�Y�̓ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)pChunkTypeSection, sizeof(SMFChunkTypeSection));
	if (apiresult != sizeof(SMFChunkTypeSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//�G���f�B�A���ϊ�
	_ReverseEndian(&(pChunkTypeSection->chunkSize), sizeof(unsigned long));

	//�������`�F�b�N
	if (memcmp(pChunkTypeSection->chunkType, "MTrk", 4) != 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}

	result = _WriteLogTrackHeader(trackNo, pChunkTypeSection);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// SMF�g���b�N�C�x���g�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadTrackEvents(
		HMMIO hFile,
		unsigned long chunkSize,
		SMTrack** pPtrTrack
	)
{
	int result = 0;
	unsigned long readSize = 0;
	unsigned long deltaTime = 0;
	unsigned long offset = 0;
	unsigned char portNo = 0;
	bool isEndOfTrack = false;
	SMEvent event;
	SMTrack* pTrack = NULL;

	try {
		pTrack = new SMTrack();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�o�͐�|�[�g�̏����l�̓g���b�N�P�ʂ�0�ԂƂ���
	portNo = 0;

	m_PrevStatus = 0;
	while (readSize < chunkSize) {

		//�f���^�^�C���ǂݍ���
		result = _ReadDeltaTime(hFile, &deltaTime, &offset);
		if (result != 0) goto EXIT;
		readSize += offset;

		//�C�x���g�ǂݍ���
		result = _ReadEvent(hFile, &event, &isEndOfTrack, &offset);
		if (result != 0) goto EXIT;
		readSize += offset;

		//�o�̓|�[�g�̐؂�ւ����m�F
		if (event.GetType() == SMEvent::EventMeta) {
			if (event.GetMetaType() == 0x21) {
				SMEventMeta meta;
				meta.Attach(&event);
				portNo = meta.GetPortNo();
			}
		}

		//�C�x���g���X�g�ɒǉ�
		result = pTrack->AddDataSet(deltaTime, &event, portNo);
		if (result != 0) goto EXIT;

		//�g���b�N�I�[
		if (isEndOfTrack) {
			if (readSize != chunkSize) {
				//�f�[�^�s��
				result = YN_SET_ERR("Invalid data found.", readSize, chunkSize);
				goto EXIT;
			}
			break;
		}
	}

	*pPtrTrack = pTrack;
	pTrack = NULL;

EXIT:;
	delete pTrack;
	return result;
}

//******************************************************************************
// SMF�f���^�^�C���̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadDeltaTime(
		HMMIO hFile,
		unsigned long* pDeltaTime,
		unsigned long* pOffset
	)
{
	int result = 0;

	result = _ReadVariableDataSize(hFile, pDeltaTime, pOffset);
	if (result != 0) goto EXIT;

	result = _WriteLogDeltaTime(*pDeltaTime);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// SMF�ϒ��f�[�^�T�C�Y�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadVariableDataSize(
		HMMIO hFile,
		unsigned long* pVariableDataSize,
		unsigned long* pOffset
	)
{
	int result = 0;
	int i = 0;
	long apiresult = 0;
	unsigned char tmp = 0;

	*pVariableDataSize = 0;
	*pOffset = 0;

	for (i = 0; i < 4; i++){
		apiresult = mmioRead(hFile, (HPSTR)&tmp, sizeof(unsigned char));
		if (apiresult != sizeof(unsigned char)) {
			result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
			goto EXIT;
		}

		*pOffset += sizeof(unsigned char);
		*pVariableDataSize = (*pVariableDataSize << 7) | (tmp & 0x7F);

		if ((tmp & 0x80) == 0) break;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �C�x���g�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadEvent(
		HMMIO hFile,
		SMEvent* pEvent,
		bool* pIsEndOfTrack,
		unsigned long* pOffset
	)
{
	int result = 0;
	long apiresult = 0;
	unsigned char tmp = 0;
	unsigned char status = 0;
	unsigned long offsetTmp = 0;
	*pIsEndOfTrack = false;
	*pOffset = 0;

	//�X�e�[�^�X��ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)&tmp, sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	//�����j���O�X�e�[�^�X�̏ȗ��`�F�b�N
	//�O���MIDI�C�x���g�����݂��Ă������1byte�ŏ�ʃr�b�g��0�Ȃ�ȗ�
	if ((m_PrevStatus != 0) && ((tmp & 0x80) == 0)) { 
		//�ȗ����ꂽ�̂őO���MIDI�C�x���g�̃X�e�[�^�X�������p��
		status = m_PrevStatus;

		//�ǂݍ��݈ʒu��߂�
		apiresult = mmioSeek(hFile, -1, SEEK_CUR);
		if (apiresult == -1) {
			result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
			goto EXIT;
		}
		*pOffset -= 1;
	}
	else {
		status = tmp;
	}

	switch (status & 0xF0) {
		case 0x80:  //�m�[�g�I�t
		case 0x90:  //�m�[�g�I��
		case 0xA0:  //�|���t�H�j�b�N�L�[�v���b�V���[
		case 0xB0:  //�R���g���[���`�F���W
		case 0xC0:  //�v���O�����`�F���W
		case 0xD0:  //�`�����l���v���b�V���[
		case 0xE0:  //�s�b�`�x���h
			//MIDI�C�x���g
			result = _ReadEventMIDI(hFile, status, pEvent, &offsetTmp);
			if (result != 0) goto EXIT;
			//�����j���O�X�e�[�^�X�ȗ�����̂��ߑO��X�e�[�^�X�Ƃ��ċL������
			m_PrevStatus = status;
			break;
		case 0xF0:
			if ((status == 0xF0) || (status == 0xF7)) {
				//SysEx�C�x���g
				result = _ReadEventSysEx(hFile, status, pEvent, &offsetTmp);
				if (result != 0) goto EXIT;
			}
			else if (status == 0xFF) {
				//���^�C�x���g
				result = _ReadEventMeta(hFile, status, pEvent, pIsEndOfTrack, &offsetTmp);
				if (result != 0) goto EXIT;
			}
			else {
				//�f�[�^�s��
				result = YN_SET_ERR("Invalid data found.", status, 0);
				goto EXIT;
			}
			break;
		default:
			//�f�[�^�s��
			result = YN_SET_ERR("Invalid data found.", status, 0);
			goto EXIT;
	}
	*pOffset += offsetTmp;

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�C�x���g�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadEventMIDI(
		HMMIO hFile,
		unsigned char status,
		SMEvent* pEvent,
		unsigned long* pOffset
	)
{
	int result = 0;
	int apiresult = 0;
	unsigned char data[2];
	unsigned long size = 0;

	*pOffset = 0;

	//DATA1��ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)&(data[0]), sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	switch (status & 0xF0) {
		case 0x80:  //�m�[�g�I�t
		case 0x90:  //�m�[�g�I��
		case 0xA0:  //�|���t�H�j�b�N�L�[�v���b�V���[
		case 0xB0:  //�R���g���[���`�F���W
		case 0xE0:  //�s�b�`�x���h
			//DATA2��ǂݍ���
			apiresult = mmioRead(hFile, (HPSTR)&(data[1]), sizeof(unsigned char));
			if (apiresult != sizeof(unsigned char)) {
				result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
				goto EXIT;
			}
			*pOffset += sizeof(unsigned char);
			size = 2;
			break;
		case 0xC0:  //�v���O�����`�F���W
		case 0xD0:  //�`�����l���v���b�V���[
			//DATA2�Ȃ�
			size = 1;
			break;
		default:
			//�f�[�^�s��
			result = YN_SET_ERR("Invalid data found.", status, 0);
			goto EXIT;
	}

	result = pEvent->SetMIDIData(status, data, size);
	if (result != 0) goto EXIT;

	result = _WriteLogEventMIDI(status, data, size);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// SysEx�C�x���g�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadEventSysEx(
		HMMIO hFile,
		unsigned char status,
		SMEvent* pEvent,
		unsigned long* pOffset
	)
{
	int result = 0;
	int apiresult = 0;
	unsigned long size = 0;
	unsigned char* pData = NULL;
	unsigned long offsetTmp = 0;
	*pOffset = 0;

	//�ϒ��f�[�^�T�C�Y��ǂݍ���
	result = _ReadVariableDataSize(hFile, &size, &offsetTmp);
	if (result != 0) goto EXIT;
	*pOffset += offsetTmp;

	try {
		pData = new unsigned char[size];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�ϒ��f�[�^��ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)(pData), size);
	if (apiresult != size) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += size;

	result = pEvent->SetSysExData(status, pData, size);
	if (result != 0) goto EXIT;

	result = _WriteLogEventSysEx(status, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	delete [] pData;
	return result;
}

//******************************************************************************
// ���^�C�x���g�̓ǂݍ���
//******************************************************************************
int SMFileReader::_ReadEventMeta(
		HMMIO hFile,
		unsigned char status,
		SMEvent* pEvent,
		bool* pIsEndOfTrack,
		unsigned long* pOffset
	)
{
	int result = 0;
	int apiresult = 0;
	unsigned long size = 0;
	unsigned char type = 0;
	unsigned char* pData = NULL;
	unsigned long offsetTmp = 0;
	*pIsEndOfTrack = false;
	*pOffset = 0;

	//��ʂ�ǂݍ���
	apiresult = mmioRead(hFile, (HPSTR)&type, sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	//���^�C�x���g���
	switch (type) {
		            //  size�iv:�ϒ��f�[�^�T�C�Y�j
		case 0x00:  //  2  �V�[�P���X�ԍ�
		case 0x01:  //  v  �e�L�X�g
		case 0x02:  //  v  ���쌠�\��
		case 0x03:  //  v  �V�[�P���X���^�g���b�N��
		case 0x04:  //  v  �y�햼
		case 0x05:  //  v  �̎�
		case 0x06:  //  v  �}�[�J�[
		case 0x07:  //  v  �L���[�|�C���g
		case 0x08:  //  v  �v���O�������^���F��
		case 0x09:  //  v  �f�o�C�X�� �^������
		case 0x20:  //  1  MIDI�`�����l���v���t�B�b�N�X
		case 0x21:  //  1  �| �[�g�w��
		case 0x2F:  //  0  �g���b�N�I�[
		case 0x51:  //  3  �e���|�ݒ�
		case 0x54:  //  5  SMPTE �I�t�Z�b�g
		case 0x58:  //  4  ���q�̐ݒ�
		case 0x59:  //  2  ���̐ݒ�
		case 0x7F:  //  v  �V�[�P���T���胁�^�C�x���g
			break;
		default:
			//���m�̎�ʂł��G���[�ɂ͂��Ȃ�
			// result = YN_SET_ERR("Invalid data found.", type, 0);
			// goto EXIT;
			break;
	}

	if (status == 0x2F) {
		*pIsEndOfTrack = true;
	}

	//�ϒ��f�[�^�T�C�Y��ǂݍ���
	result = _ReadVariableDataSize(hFile, &size, &offsetTmp);
	if (result != 0) goto EXIT;
	*pOffset += offsetTmp;

	//�ϒ��f�[�^��ǂݍ���
	if (size > 0) {
		try {
			pData = new unsigned char[size];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}
		apiresult = mmioRead(hFile, (HPSTR)pData, size);
		if (apiresult != size) {
			result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
			goto EXIT;
		}
		*pOffset += size;
	}

	result = pEvent->SetMetaData(status, type, pData, size);
	if (result != 0) goto EXIT;

	result = _WriteLogEventMeta(status, type, pData, size);
	if (result != 0) goto EXIT;

EXIT:;
	delete [] pData;
	return result;
}

//******************************************************************************
// �G���f�B�A���ϊ�
//******************************************************************************
void SMFileReader::_ReverseEndian(
		void* pData,
		unsigned long size
	)
{
	unsigned char tmp;
	unsigned char* pHead = (unsigned char*)pData;
	unsigned char* pTail = pHead + size - 1;

	while (pHead < pTail) {
		tmp = *pHead;
		*pHead = *pTail;
		*pTail = tmp;
		pHead += 1;
		pTail -= 1;
	}

	return;
}

//******************************************************************************
// ���O�t�@�C���I�[�v��
//******************************************************************************
int SMFileReader::_OpenLogFile()
{
	int result = 0;
	errno_t eresult = 0;

	if (_tcslen(m_LogPath) == 0) goto EXIT;

	eresult = _tfopen_s(&m_pLogFile, m_LogPath, _T("w"));
	if (eresult != 0) {
		result = YN_SET_ERR("Log file open error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���O�t�@�C���N���[�Y
//******************************************************************************
int SMFileReader::_CloseLogFile()
{
	int result = 0;
	int eresult = 0;

	if (!m_IsLogOut) goto EXIT;

	eresult = fclose(m_pLogFile);
	if (eresult != 0) {
		result = YN_SET_ERR("Log file close error.", 0, 0);
		goto EXIT;
	}

	m_pLogFile = NULL;

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o��
//******************************************************************************
int SMFileReader::_WriteLog(char* pText)
{
	int result = 0;
	size_t size = 0;
	size_t eresult = 0;

	if (!m_IsLogOut) goto EXIT;

	size = strlen(pText);

	eresult = fwrite(pText, size, 1, m_pLogFile);
	if (eresult != size) {
		result = YN_SET_ERR("Log file write error.", size, eresult);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o�́F�t�@�C���w�b�_
//******************************************************************************
int SMFileReader::_WriteLogChunkHeader(
		SMFChunkTypeSection* pChunkTypeSection,
		SMFChunkDataSection* pChunkDataSection
	)
{
	int result = 0;
	char msg[256];

	if (!m_IsLogOut) goto EXIT;

	_WriteLog("--------------------\n");
	_WriteLog("File Header\n");
	_WriteLog("--------------------\n");
	_WriteLog("Chunk Type : MThd\n");
	sprintf_s(msg, 256, "Length     : %d\n", pChunkTypeSection->chunkSize);
	_WriteLog(msg);
	sprintf_s(msg, 256, "Format     : %d\n", pChunkDataSection->format);
	_WriteLog(msg);
	sprintf_s(msg, 256, "nTracks    : %d\n", pChunkDataSection->ntracks);
	_WriteLog(msg);
	sprintf_s(msg, 256, "Devision   : %d\n", pChunkDataSection->timeDivision);
	_WriteLog(msg);

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o�́F�g���b�N�w�b�_
//******************************************************************************
int SMFileReader::_WriteLogTrackHeader(
		unsigned long trackNo,
		SMFChunkTypeSection* pChunkTypeSection
	)
{
	int result = 0;
	char msg[256];

	if (!m_IsLogOut) goto EXIT;

	_WriteLog("--------------------\n");
	sprintf_s(msg, 256, "Track No.%d\n", trackNo);
	_WriteLog(msg);
	_WriteLog("--------------------\n");
	_WriteLog("Chunk Type : MTrk\n");
	sprintf_s(msg, 256, "Length     : %d\n", pChunkTypeSection->chunkSize);
	_WriteLog(msg);
	_WriteLog("Delta Time | Event\n");

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o�́F�f���^�^�C��
//******************************************************************************
int SMFileReader::_WriteLogDeltaTime(
		unsigned long deltaTime
	)
{
	int result = 0;
	char msg[256];

	if (!m_IsLogOut) goto EXIT;

	sprintf_s(msg, 256, "% 10d | ", deltaTime);
	_WriteLog(msg);

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o�́FMIDI�C�x���g
//******************************************************************************
int SMFileReader::_WriteLogEventMIDI(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	char* cmd = "";
	char msg[256];

	if (!m_IsLogOut) goto EXIT;

	switch (status & 0xF0) {
		case 0x80: cmd = "Note Off";				break;
		case 0x90: cmd = "Note On";					break;
		case 0xA0: cmd = "Polyphonic Key Pressure";	break;
		case 0xB0: cmd = "Control Change";			break;
		case 0xC0: cmd = "Program Change";			break;
		case 0xD0: cmd = "Channel Pressure";		break;
		case 0xE0: cmd = "PitchBend";				break;
		default:   cmd = "unknown";					break;
	}

	sprintf_s(msg, 256, "MIDI: ch.%d cmd=<%s>", (status & 0x0F), cmd);
	_WriteLog(msg);

	if (size == 2) {
		sprintf_s(msg, 256, " data=[ %02X %02X %02X ]\n", status, pData[0], pData[1]);
	}
	else {
		sprintf_s(msg, 256, " data=[ %02X %02X ]\n", status, pData[0]);
	}
	_WriteLog(msg);

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o�́FSysEx�C�x���g
//******************************************************************************
int SMFileReader::_WriteLogEventSysEx(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	char msg[256];
	unsigned long i = 0;

	if (!m_IsLogOut) goto EXIT;

	sprintf_s(msg, 256, "SysEx: status=%02X size=%d data=[", status, size);
	_WriteLog(msg);

	for (i = 0; i < size; i++) {
		sprintf_s(msg, 256, " %02X", pData[i]);
		_WriteLog(msg);
	}
	_WriteLog(" ]\n");

EXIT:;
	return result;
}

//******************************************************************************
// ���O�o�́F���^�C�x���g
//******************************************************************************
int SMFileReader::_WriteLogEventMeta(
		unsigned char status,
		unsigned char type,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	char* cmd = "";
	char msg[256];
	unsigned long i = 0;

	if (!m_IsLogOut) goto EXIT;

	switch (type) {
		case 0x00: cmd = "Sequence Number";					break;
		case 0x01: cmd = "Text Event";						break;
		case 0x02: cmd = "Copyright Notice";				break;
		case 0x03: cmd = "Sequence/Track Name";				break;
		case 0x04: cmd = "Instrument Name";					break;
		case 0x05: cmd = "Lyric";							break;
		case 0x06: cmd = "Marker";							break;
		case 0x07: cmd = "Cue Point";						break;
		case 0x08: cmd = "Program Name";					break;
		case 0x09: cmd = "Device Name";						break;
		case 0x21: cmd = "Port Number (Undocumented)";		break;
		case 0x2F: cmd = "End of Track";					break;
		case 0x51: cmd = "Set Tempo";						break;
		case 0x54: cmd = "SMPTE Offset";					break;
		case 0x58: cmd = "Time Signature";					break;
		case 0x59: cmd = "Key Signature";					break;
		case 0x7F: cmd = "Sequencer-Specific Meta-Event";	break;
		default:   cmd = "<unknown>";						break;
	}

	sprintf_s(msg, 256, "Meta: status=%02X type=%02X<%s> size=%d data=[", status, type, cmd, size);
	_WriteLog(msg);

	for (i = 0; i < size; i++) {
		sprintf_s(msg, 256, " %02X", pData[i]);
		_WriteLog(msg);
	}
	_WriteLog(" ]\n");

EXIT:;
	return result;
}

} // end of namespace

