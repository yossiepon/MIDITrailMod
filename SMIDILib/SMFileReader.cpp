//******************************************************************************
//
// Simple MIDI Library / SMFileReader
//
// Standard MIDI file reader class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Ced (MIDITrail Mod Mod). All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMFileReader.h"
#include "SMSimpleList.h"
#include "SMCommon.h"
#include "tchar.h"
#include "shlwapi.h"

using namespace YNBaseLib;

namespace SMIDILib {

//******************************************************************************
// Constructor
//******************************************************************************
SMFileReader::SMFileReader(void)
{
	m_LogPath[0] = '\0';
	m_pLogFile = NULL;
	m_IsLogOut = false;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMFileReader::~SMFileReader(void)
{
}

//******************************************************************************
// Log output path setting
//******************************************************************************
int SMFileReader::SetLogPath(
		const WCHAR* pLogPath
	)
{
	int result = 0;
	errno_t eresult = 0;

	m_IsLogOut = false;

	if (pLogPath == NULL) {
		m_LogPath[0] = L'\0';
	}
	else {
		eresult = wcscpy_s(m_LogPath, MAX_PATH, pLogPath);
		if (eresult != 0) {
			result = YN_SET_ERR("Program error.", 0, 0);
			goto EXIT;
		}
	}

	if (wcslen(m_LogPath) > 0) {
		m_IsLogOut = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Load Standard MIDI File
//******************************************************************************
int SMFileReader::Load(
		const WCHAR *pSMFPath,
		SMSeqData* pSeqData,
		SMLoadProgressFunc progressFunc,
		void* progressUserData,
		bool* pWasTruncated
	)
{
	int result = 0;
	unsigned long i = 0;
	HMMIO hFile = NULL;
	HANDLE hOsFile = INVALID_HANDLE_VALUE;
	HANDLE hMapping = NULL;
	LPVOID pMapView = NULL;
	SMFChunkTypeSection chunkTypeSection;
	SMFChunkDataSection chunkDataSection;
	SMFChunkTypeSection chunkTypeSectionOfTrack;
	SMTrack* pTrack = NULL;

	static const unsigned long TOTAL_UNITS = 10000;
	static const unsigned long READ_UNITS  = 6000;
	static const unsigned long MERGE_UNITS = 4000;

	if ((pSMFPath == NULL) || (pSeqData == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pSeqData->Clear();
	SMSimpleList::ResetTruncatedFlag();

	//Open log file
	result = _OpenLogFile();
	if (result != 0 ) goto EXIT;

	//Try memory-mapped I/O first
	hOsFile = CreateFileW(pSMFPath, GENERIC_READ, FILE_SHARE_READ, NULL,
						  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOsFile != INVALID_HANDLE_VALUE) {
		DWORD fileSizeHigh = 0;
		DWORD fileSize = GetFileSize(hOsFile, &fileSizeHigh);
		if (fileSizeHigh == 0 && fileSize > 0) {
			hMapping = CreateFileMapping(hOsFile, NULL, PAGE_READONLY, 0, 0, NULL);
			if (hMapping != NULL) {
				pMapView = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
				if (pMapView != NULL) {
					MMIOINFO mmioInfo;
					ZeroMemory(&mmioInfo, sizeof(MMIOINFO));
					mmioInfo.fccIOProc = FOURCC_MEM;
					mmioInfo.cchBuffer = fileSize;
					mmioInfo.pchBuffer = (HPSTR)pMapView;
					hFile = mmioOpen(NULL, &mmioInfo, MMIO_READ);
				}
			}
		}
	}

	//Fallback to standard mmio if memory mapping failed
	if (hFile == NULL) {
		if (pMapView != NULL) { UnmapViewOfFile(pMapView); pMapView = NULL; }
		if (hMapping != NULL) { CloseHandle(hMapping); hMapping = NULL; }
		if (hOsFile != INVALID_HANDLE_VALUE) { CloseHandle(hOsFile); hOsFile = INVALID_HANDLE_VALUE; }

		hFile = mmioOpenW((LPWSTR)pSMFPath, NULL, MMIO_READ);
		if (hFile == NULL) {
			result = YN_SET_ERR("File open error.", GetLastError(), 0);
			goto EXIT;
		}
	}

	//Skip RIFF header
	result = _SkipRIFFHeader(hFile);
	if (result != 0 ) goto EXIT;

	//Read header
	result = _ReadChunkHeader(hFile, &chunkTypeSection, &chunkDataSection);
	if (result != 0 ) goto EXIT;

	if ((chunkDataSection.format != 0) && (chunkDataSection.format != 1)) {
		result = YN_SET_ERR("Unsupported SMF format.", chunkDataSection.format, 0);
		goto EXIT;
	}
	if ( chunkDataSection.ntracks == 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if ( chunkDataSection.timeDivision == 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if ((chunkDataSection.timeDivision & 0x80000000) != 0) {
		result = YN_SET_ERR("Unsupported SMF format.", chunkDataSection.timeDivision, 0);
		goto EXIT;
	}

	pSeqData->SetSMFFormat(chunkDataSection.format);
	pSeqData->SetTimeDivision(chunkDataSection.timeDivision);

	{
		//Progress context for track reading phase (0 ~ READ_UNITS)
		SMLoadProgressContext readProgress;
		readProgress.func = progressFunc;
		readProgress.userData = progressUserData;
		readProgress.offset = 0;
		readProgress.range = READ_UNITS;
		readProgress.total = TOTAL_UNITS;

		for (i = 0; i < chunkDataSection.ntracks; i++) {
			//Read track header
			result = _ReadTrackHeader(hFile, i, &chunkTypeSectionOfTrack);
			if (result != 0 ) goto EXIT;

			//Read track events
			result = _ReadTrackEvents(hFile, chunkTypeSectionOfTrack.chunkSize, &pTrack,
									 i, chunkDataSection.ntracks,
									 (progressFunc != NULL) ? &readProgress : NULL);
			if (result != 0 ) goto EXIT;

			result = pSeqData->AddTrack(pTrack);
			if (result != 0 ) goto EXIT;
			pTrack = NULL;

			if (SMSimpleList::WasTruncated()) break;
		}
	}

	//Close track (merge phase: READ_UNITS ~ TOTAL_UNITS)
	result = pSeqData->CloseTrack(progressFunc, progressUserData,
								  READ_UNITS, TOTAL_UNITS);
	if (result != 0 ) goto EXIT;

	//Register file name
	pSeqData->SetFileName(PathFindFileNameW(pSMFPath));

EXIT:;
	if (pWasTruncated != NULL) {
		*pWasTruncated = SMSimpleList::WasTruncated();
	}
	if (hFile != NULL) {
		mmioClose(hFile, 0);
		hFile = NULL;
	}
	if (pMapView != NULL) UnmapViewOfFile(pMapView);
	if (hMapping != NULL) CloseHandle(hMapping);
	if (hOsFile != INVALID_HANDLE_VALUE) CloseHandle(hOsFile);
	_CloseLogFile();
	return result;
}

//******************************************************************************
// Skip RIFF header
//******************************************************************************
int SMFileReader::_SkipRIFFHeader(
		HMMIO hFile
	)
{
	int result = 0;
	long apiresult = 0;
	SMFRIFFChunkHeader chunkHeader;
	SMFRIFFSubChunkHeader subChunkHeader;

	//Read RIFF chunk
	apiresult = mmioRead(hFile, (HPSTR)&chunkHeader, sizeof(SMFRIFFChunkHeader));
	if (apiresult != sizeof(SMFRIFFChunkHeader)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//Check identifier
	if (memcmp(chunkHeader.chunkID, "RIFF", 4) != 0) {
		//Not RIFF; rewind read position to start and exit normally
		apiresult = mmioSeek(hFile, 0, SEEK_SET);
		if (apiresult == -1) {
			result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
			goto EXIT;
		}
		goto EXIT;
	}

	//Check format
	if (memcmp(chunkHeader.format, "RMID", 4) != 0) {
		//RIFF but not MIDI data; treat as file error
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}

	//Read RIFF sub-chunk
	apiresult = mmioRead(hFile, (HPSTR)&subChunkHeader, sizeof(SMFRIFFSubChunkHeader));
	if (apiresult != sizeof(SMFRIFFSubChunkHeader)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//Check format
	//  LIST chunks are not supported
	if (memcmp(subChunkHeader.chunkID, "data", 4) != 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Read SMF header
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

	//Read identifier and header data size
	apiresult = mmioRead(hFile, (HPSTR)pChunkTypeSection, sizeof(SMFChunkTypeSection));
	if (apiresult != sizeof(SMFChunkTypeSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//Endian conversion
	_ReverseEndian(&(pChunkTypeSection->chunkSize), sizeof(unsigned long));

	//Consistency check
	if (memcmp(pChunkTypeSection->chunkType, "MThd", 4) != 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if (pChunkTypeSection->chunkSize < sizeof(SMFChunkDataSection)) {
		result = YN_SET_ERR("Invalid data found.", pChunkTypeSection->chunkSize, 0);
		goto EXIT;
	}

	//Read header data
	apiresult = mmioRead(hFile, (HPSTR)pChunkDataSection, sizeof(SMFChunkDataSection));
	if (apiresult != sizeof(SMFChunkDataSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//Endian conversion
	_ReverseEndian(&(pChunkDataSection->format), sizeof(unsigned short));
	_ReverseEndian(&(pChunkDataSection->ntracks), sizeof(unsigned short));
	_ReverseEndian(&(pChunkDataSection->timeDivision), sizeof(unsigned short));

	//Skip to specified data size (just in case)
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
// Read SMF track header
//******************************************************************************
int SMFileReader::_ReadTrackHeader(
		HMMIO hFile,
		unsigned long trackNo,
		SMFChunkTypeSection* pChunkTypeSection
	)
{
	int result = 0;
	long apiresult = 0;

	//Read identifier and header data size
	apiresult = mmioRead(hFile, (HPSTR)pChunkTypeSection, sizeof(SMFChunkTypeSection));
	if (apiresult != sizeof(SMFChunkTypeSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//Endian conversion
	_ReverseEndian(&(pChunkTypeSection->chunkSize), sizeof(unsigned long));

	//Consistency check
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
// Read SMF track events
//******************************************************************************
int SMFileReader::_ReadTrackEvents(
		HMMIO hFile,
		unsigned long chunkSize,
		SMTrack** pPtrTrack,
		unsigned long trackIndex,
		unsigned long trackCount,
		const SMLoadProgressContext* pProgress
	)
{
	int result = 0;
	long apiresult = 0;
	unsigned long readSize = 0;
	unsigned long deltaTime = 0;
	unsigned long offset = 0;
	unsigned char portNo = 0;
	bool isEndOfTrack = false;
	unsigned long lastProgressRead = 0;
	SMEvent event;
	SMTrack* pTrack = NULL;

	static const unsigned long PROGRESS_INTERVAL = 256 * 1024;

	try {
		pTrack = new SMTrack();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//Default output port per track is 0
	portNo = 0;

	m_PrevStatus = 0;
	while (readSize < chunkSize) {

		//Read delta time
		result = _ReadDeltaTime(hFile, &deltaTime, &offset);
		if (result != 0) goto EXIT;
		readSize += offset;

		//Read event
		result = _ReadEvent(hFile, &event, &isEndOfTrack, &offset);
		if (result != 0) goto EXIT;
		readSize += offset;

		//Check for output port change
		if (event.GetType() == SMEvent::EventMeta) {
			if (event.GetMetaType() == 0x21) {
				SMEventMeta meta;
				meta.Attach(&event);
				portNo = meta.GetPortNo();
			}
		}

		//Add to event list
		result = pTrack->AddDataSet(deltaTime, &event, portNo);
		if (result != 0) goto EXIT;

		if (SMSimpleList::WasTruncated()) break;

		//Progress callback
		if (pProgress != NULL && pProgress->func != NULL
			&& (readSize - lastProgressRead) >= PROGRESS_INTERVAL) {
			unsigned long trackFraction = (chunkSize > 0)
				? (unsigned long)((unsigned long long)readSize * 10000 / chunkSize)
				: 10000;
			unsigned long perTrack = pProgress->range / trackCount;
			unsigned long current = pProgress->offset
				+ trackIndex * perTrack
				+ (unsigned long)((unsigned long long)trackFraction * perTrack / 10000);
			pProgress->func(current, pProgress->total, pProgress->userData);
			lastProgressRead = readSize;
		}

		//End of track
		if (isEndOfTrack) {
			//Skip to specified chunk size (just in case)
			if (readSize < chunkSize) {
				offset = chunkSize - readSize;
				apiresult = mmioSeek(hFile, offset, SEEK_CUR);
				if (apiresult == -1) {
					result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
					goto EXIT;
				}
				readSize += offset;
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
// Read SMF delta time
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
// Read SMF variable-length data size
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
// Read event
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

	//Read status
	apiresult = mmioRead(hFile, (HPSTR)&tmp, sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	//Check for running status omission
	//Omitted if a previous MIDI event exists and this byte's high bit is 0
	if ((m_PrevStatus != 0) && ((tmp & 0x80) == 0)) { 
		//Omitted; inherit status from the previous MIDI event
		status = m_PrevStatus;

		//Rewind read position
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
		case 0x80:  //Note Off
		case 0x90:  //Note On
		case 0xA0:  //Polyphonic Key Pressure
		case 0xB0:  //Control Change
		case 0xC0:  //Program Change
		case 0xD0:  //Channel Pressure
		case 0xE0:  //Pitch Bend
			//MIDI event
			result = _ReadEventMIDI(hFile, status, pEvent, &offsetTmp);
			if (result != 0) goto EXIT;
			//Remember as previous status for running status omission check
			m_PrevStatus = status;
			break;
		case 0xF0:
			if ((status == 0xF0) || (status == 0xF7)) {
				//SysEx event
				result = _ReadEventSysEx(hFile, status, pEvent, &offsetTmp);
				if (result != 0) goto EXIT;
			}
			else if (status == 0xFF) {
				//Meta event
				result = _ReadEventMeta(hFile, status, pEvent, pIsEndOfTrack, &offsetTmp);
				if (result != 0) goto EXIT;
			}
			else {
				//Invalid data
				result = YN_SET_ERR("Invalid data found.", status, 0);
				goto EXIT;
			}
			break;
		default:
			//Invalid data
			result = YN_SET_ERR("Invalid data found.", status, 0);
			goto EXIT;
	}
	*pOffset += offsetTmp;

EXIT:;
	return result;
}

//******************************************************************************
// Read MIDI event
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

	//Read DATA1
	apiresult = mmioRead(hFile, (HPSTR)&(data[0]), sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	switch (status & 0xF0) {
		case 0x80:  //Note Off
		case 0x90:  //Note On
		case 0xA0:  //Polyphonic Key Pressure
		case 0xB0:  //Control Change
		case 0xE0:  //Pitch Bend
			//Read DATA2
			apiresult = mmioRead(hFile, (HPSTR)&(data[1]), sizeof(unsigned char));
			if (apiresult != sizeof(unsigned char)) {
				result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
				goto EXIT;
			}
			*pOffset += sizeof(unsigned char);
			size = 2;
			break;
		case 0xC0:  //Program Change
		case 0xD0:  //Channel Pressure
			//No DATA2
			size = 1;
			break;
		default:
			//Invalid data
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
// Read SysEx event
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

	//Read variable-length data size
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

	//Read variable-length data
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
// Read meta event
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

	//Read type
	apiresult = mmioRead(hFile, (HPSTR)&type, sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	//Meta event type
	switch (type) {
		            //  size (v: variable-length data size)
		case 0x00:  //  2  Sequence Number
		case 0x01:  //  v  Text
		case 0x02:  //  v  Copyright Notice
		case 0x03:  //  v  Sequence/Track Name
		case 0x04:  //  v  Instrument Name
		case 0x05:  //  v  Lyric
		case 0x06:  //  v  Marker
		case 0x07:  //  v  Cue Point
		case 0x08:  //  v  Program Name / Patch Name
		case 0x09:  //  v  Device Name / Sound Set Name
		case 0x20:  //  1  MIDI Channel Prefix
		case 0x21:  //  1  Port designation
		case 0x2F:  //  0  End of Track
		case 0x51:  //  3  Set Tempo
		case 0x54:  //  5  SMPTE Offset
		case 0x58:  //  4  Time Signature
		case 0x59:  //  2  Key Signature
		case 0x7F:  //  v  Sequencer-Specific Meta Event
			break;
		default:
			//Do not error on unknown type
			// result = YN_SET_ERR("Invalid data found.", type, 0);
			// goto EXIT;
			break;
	}

	if (type == 0x2F) {
		*pIsEndOfTrack = true;
	}

	//Read variable-length data size
	result = _ReadVariableDataSize(hFile, &size, &offsetTmp);
	if (result != 0) goto EXIT;
	*pOffset += offsetTmp;

	//Read variable-length data
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
// Endian conversion
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
// Open log file
//******************************************************************************
int SMFileReader::_OpenLogFile()
{
	int result = 0;
	errno_t eresult = 0;

	if (wcslen(m_LogPath) == 0) goto EXIT;

	eresult = _wfopen_s(&m_pLogFile, m_LogPath, L"w");
	if (eresult != 0) {
		result = YN_SET_ERR("Log file open error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Close log file
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
// Log output
//******************************************************************************
int SMFileReader::_WriteLog(const char* pText)
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
// Log output: File header
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
// Log output: Track header
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
// Log output: Delta time
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
// Log output: MIDI event
//******************************************************************************
int SMFileReader::_WriteLogEventMIDI(
		unsigned char status,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	const char* cmd = "";
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
// Log output: SysEx event
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
// Log output: Meta event
//******************************************************************************
int SMFileReader::_WriteLogEventMeta(
		unsigned char status,
		unsigned char type,
		unsigned char* pData,
		unsigned long size
	)
{
	int result = 0;
	const char* cmd = "";
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

