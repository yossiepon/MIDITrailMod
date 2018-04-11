//******************************************************************************
//
// Simple MIDI Library / SMFileReader
//
// 標準MIDIファイル読み込みクラス
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
// コンストラクタ
//******************************************************************************
SMFileReader::SMFileReader(void)
{
	m_LogPath[0] = '\0';
	m_pLogFile = NULL;
	m_IsLogOut = false;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMFileReader::~SMFileReader(void)
{
}

//******************************************************************************
// ログ出力パス設定
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
// Standard MIDI File のロード
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

	//ログファイルを開く
	result = _OpenLogFile();
	if (result != 0 ) goto EXIT;

	//ファイルを開く
	hFile = mmioOpen((LPTSTR)pSMFPath, NULL, MMIO_READ);
	if (hFile == NULL) {
		result = YN_SET_ERR("File open error.", GetLastError(), 0);
		goto EXIT;
	}

	//ヘッダ読み込み
	result = _ReadChunkHeader(hFile, &chunkTypeSection, &chunkDataSection);
	if (result != 0 ) goto EXIT;

	if ((chunkDataSection.format != 0) && (chunkDataSection.format != 1)) {
		//フォーマット0,1以外は未対応
		result = YN_SET_ERR("Unsupported SMF format.", chunkDataSection.format, 0);
		goto EXIT;
	}
	if ( chunkDataSection.ntracks == 0) {
		//データ異常
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if ( chunkDataSection.timeDivision == 0) {
		//データ異常
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if ((chunkDataSection.timeDivision & 0x80000000) != 0) {
		//分解能が負の場合はデルタタイムを実時間とみなす仕様がある
		//一般的でないので今のところサポートしない
		result = YN_SET_ERR("Unsupported SMF format.", chunkDataSection.timeDivision, 0);
		goto EXIT;
	}

	pSeqData->SetSMFFormat(chunkDataSection.format);
	pSeqData->SetTimeDivision(chunkDataSection.timeDivision);

	for (i = 0; i < chunkDataSection.ntracks; i++) {
		//トラックヘッダ読み込み
		result = _ReadTrackHeader(hFile, i, &chunkTypeSectionOfTrack);
		if (result != 0 ) goto EXIT;

		//トラックイベント読み込み
		result = _ReadTrackEvents(hFile, chunkTypeSectionOfTrack.chunkSize, &pTrack);
		if (result != 0 ) goto EXIT;

		result = pSeqData->AddTrack(pTrack);
		if (result != 0 ) goto EXIT;
		pTrack = NULL;
	}

	//トラックを閉じる
	result = pSeqData->CloseTrack();
	if (result != 0 ) goto EXIT;

	//ファイル名登録
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
// SMFヘッダ読み込み
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

	//識別子とヘッダデータサイズの読み込み
	apiresult = mmioRead(hFile, (HPSTR)pChunkTypeSection, sizeof(SMFChunkTypeSection));
	if (apiresult != sizeof(SMFChunkTypeSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//エンディアン変換
	_ReverseEndian(&(pChunkTypeSection->chunkSize), sizeof(unsigned long));

	//整合性チェック
	if (memcmp(pChunkTypeSection->chunkType, "MThd", 4) != 0) {
		result = YN_SET_ERR("Invalid data found.", 0, 0);
		goto EXIT;
	}
	if (pChunkTypeSection->chunkSize < sizeof(SMFChunkDataSection)) {
		result = YN_SET_ERR("Invalid data found.", pChunkTypeSection->chunkSize, 0);
		goto EXIT;
	}

	//ヘッダデータの読み込み
	apiresult = mmioRead(hFile, (HPSTR)pChunkDataSection, sizeof(SMFChunkDataSection));
	if (apiresult != sizeof(SMFChunkDataSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//エンディアン変換
	_ReverseEndian(&(pChunkDataSection->format), sizeof(unsigned short));
	_ReverseEndian(&(pChunkDataSection->ntracks), sizeof(unsigned short));
	_ReverseEndian(&(pChunkDataSection->timeDivision), sizeof(unsigned short));

	//指定されたデータサイズまでスキップする（念のため）
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
// SMFトラックヘッダの読み込み
//******************************************************************************
int SMFileReader::_ReadTrackHeader(
		HMMIO hFile,
		unsigned long trackNo,
		SMFChunkTypeSection* pChunkTypeSection
	)
{
	int result = 0;
	long apiresult = 0;

	//識別子とヘッダデータサイズの読み込み
	apiresult = mmioRead(hFile, (HPSTR)pChunkTypeSection, sizeof(SMFChunkTypeSection));
	if (apiresult != sizeof(SMFChunkTypeSection)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}

	//エンディアン変換
	_ReverseEndian(&(pChunkTypeSection->chunkSize), sizeof(unsigned long));

	//整合性チェック
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
// SMFトラックイベントの読み込み
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

	//出力先ポートの初期値はトラック単位で0番とする
	portNo = 0;

	m_PrevStatus = 0;
	while (readSize < chunkSize) {

		//デルタタイム読み込み
		result = _ReadDeltaTime(hFile, &deltaTime, &offset);
		if (result != 0) goto EXIT;
		readSize += offset;

		//イベント読み込み
		result = _ReadEvent(hFile, &event, &isEndOfTrack, &offset);
		if (result != 0) goto EXIT;
		readSize += offset;

		//出力ポートの切り替えを確認
		if (event.GetType() == SMEvent::EventMeta) {
			if (event.GetMetaType() == 0x21) {
				SMEventMeta meta;
				meta.Attach(&event);
				portNo = meta.GetPortNo();
			}
		}

		//イベントリストに追加
		result = pTrack->AddDataSet(deltaTime, &event, portNo);
		if (result != 0) goto EXIT;

		//トラック終端
		if (isEndOfTrack) {
			if (readSize != chunkSize) {
				//データ不正
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
// SMFデルタタイムの読み込み
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
// SMF可変長データサイズの読み込み
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
// イベントの読み込み
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

	//ステータスを読み込む
	apiresult = mmioRead(hFile, (HPSTR)&tmp, sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	//ランニングステータスの省略チェック
	//前回のMIDIイベントが存在してかつ今回の1byte最上位ビットが0なら省略
	if ((m_PrevStatus != 0) && ((tmp & 0x80) == 0)) { 
		//省略されたので前回のMIDIイベントのステータスを引き継ぐ
		status = m_PrevStatus;

		//読み込み位置を戻す
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
		case 0x80:  //ノートオフ
		case 0x90:  //ノートオン
		case 0xA0:  //ポリフォニックキープレッシャー
		case 0xB0:  //コントロールチェンジ
		case 0xC0:  //プログラムチェンジ
		case 0xD0:  //チャンネルプレッシャー
		case 0xE0:  //ピッチベンド
			//MIDIイベント
			result = _ReadEventMIDI(hFile, status, pEvent, &offsetTmp);
			if (result != 0) goto EXIT;
			//ランニングステータス省略判定のため前回ステータスとして記憶する
			m_PrevStatus = status;
			break;
		case 0xF0:
			if ((status == 0xF0) || (status == 0xF7)) {
				//SysExイベント
				result = _ReadEventSysEx(hFile, status, pEvent, &offsetTmp);
				if (result != 0) goto EXIT;
			}
			else if (status == 0xFF) {
				//メタイベント
				result = _ReadEventMeta(hFile, status, pEvent, pIsEndOfTrack, &offsetTmp);
				if (result != 0) goto EXIT;
			}
			else {
				//データ不正
				result = YN_SET_ERR("Invalid data found.", status, 0);
				goto EXIT;
			}
			break;
		default:
			//データ不正
			result = YN_SET_ERR("Invalid data found.", status, 0);
			goto EXIT;
	}
	*pOffset += offsetTmp;

EXIT:;
	return result;
}

//******************************************************************************
// MIDIイベントの読み込み
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

	//DATA1を読み込む
	apiresult = mmioRead(hFile, (HPSTR)&(data[0]), sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	switch (status & 0xF0) {
		case 0x80:  //ノートオフ
		case 0x90:  //ノートオン
		case 0xA0:  //ポリフォニックキープレッシャー
		case 0xB0:  //コントロールチェンジ
		case 0xE0:  //ピッチベンド
			//DATA2を読み込む
			apiresult = mmioRead(hFile, (HPSTR)&(data[1]), sizeof(unsigned char));
			if (apiresult != sizeof(unsigned char)) {
				result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
				goto EXIT;
			}
			*pOffset += sizeof(unsigned char);
			size = 2;
			break;
		case 0xC0:  //プログラムチェンジ
		case 0xD0:  //チャンネルプレッシャー
			//DATA2なし
			size = 1;
			break;
		default:
			//データ不正
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
// SysExイベントの読み込み
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

	//可変長データサイズを読み込む
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

	//可変長データを読み込む
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
// メタイベントの読み込み
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

	//種別を読み込む
	apiresult = mmioRead(hFile, (HPSTR)&type, sizeof(unsigned char));
	if (apiresult != sizeof(unsigned char)) {
		result = YN_SET_ERR("File read error.", GetLastError(), apiresult);
		goto EXIT;
	}
	*pOffset += sizeof(unsigned char);

	//メタイベント種別
	switch (type) {
		            //  size（v:可変長データサイズ）
		case 0x00:  //  2  シーケンス番号
		case 0x01:  //  v  テキスト
		case 0x02:  //  v  著作権表示
		case 0x03:  //  v  シーケンス名／トラック名
		case 0x04:  //  v  楽器名
		case 0x05:  //  v  歌詞
		case 0x06:  //  v  マーカー
		case 0x07:  //  v  キューポイント
		case 0x08:  //  v  プログラム名／音色名
		case 0x09:  //  v  デバイス名 ／音源名
		case 0x20:  //  1  MIDIチャンネルプリフィックス
		case 0x21:  //  1  ポ ート指定
		case 0x2F:  //  0  トラック終端
		case 0x51:  //  3  テンポ設定
		case 0x54:  //  5  SMPTE オフセット
		case 0x58:  //  4  拍子の設定
		case 0x59:  //  2  調の設定
		case 0x7F:  //  v  シーケンサ特定メタイベント
			break;
		default:
			//未知の種別でもエラーにはしない
			// result = YN_SET_ERR("Invalid data found.", type, 0);
			// goto EXIT;
			break;
	}

	if (status == 0x2F) {
		*pIsEndOfTrack = true;
	}

	//可変長データサイズを読み込む
	result = _ReadVariableDataSize(hFile, &size, &offsetTmp);
	if (result != 0) goto EXIT;
	*pOffset += offsetTmp;

	//可変長データを読み込む
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
// エンディアン変換
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
// ログファイルオープン
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
// ログファイルクローズ
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
// ログ出力
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
// ログ出力：ファイルヘッダ
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
// ログ出力：トラックヘッダ
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
// ログ出力：デルタタイム
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
// ログ出力：MIDIイベント
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
// ログ出力：SysExイベント
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
// ログ出力：メタイベント
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

