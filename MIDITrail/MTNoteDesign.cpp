//******************************************************************************
//
// MIDITrail / MTNoteDesign
//
// ノートデザインクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTNoteDesign.h"
#include <math.h>
#include <algorithm>

using namespace YNBaseLib;

//------------------------------------------------------------------------------
// merged note + source track number, sorted by note start tick
//------------------------------------------------------------------------------
namespace {
	struct MTNoteTrackItem {
		SMNote note;
		unsigned char trackNo;
	};
	bool MTCompareNoteTrackItem(const MTNoteTrackItem& a, const MTNoteTrackItem& b) {
		return a.note.startTime < b.note.startTime;
	}
}


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteDesign::MTNoteDesign(void)
{
	_Clear();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteDesign::~MTNoteDesign(void)
{
}

//******************************************************************************
// 初期化
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

	//ced 20260628: CHANNELTRACK の「各chの最初のトラック」マップを曲ごとにリセット
	for (index = 0; index < 16; index++) {
		m_FirstTrackForChannel[index] = -1;
	}

	//ライブモニタ向け設定
	if (pSeqData == NULL) {
		//分解能
		m_TimeDivision = 48;
		//ポートリスト
		m_PortList.Clear();
		m_PortList.AddPort(0);
	}
	//通常設定
	else {
		//分解能取得
		m_TimeDivision = pSeqData->GetTimeDivision();
		if (m_TimeDivision == 0) {
			result = YN_SET_ERR("Invalid data found.", 0, 0);
			goto EXIT;
		}
		//ポートリスト取得
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
	}

	//ポート番号に昇順のインデックスを振る
	//ポート 0番 3番 5番 に出力する場合のインデックスはそれぞれ 0, 1, 2
	for (index = 0; index < 256; index++) {
		m_PortIndex[index] = 0;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = (unsigned char)portIndex;
		portIndex++;
	}

	//パラメータ設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//ユーザ設定読み込み
	result = _LoadUserConf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 演奏位置取得
//******************************************************************************
float MTNoteDesign::GetPlayPosX(
		unsigned long curTickTime
	)
{
	// no song loaded (live monitor): m_TimeDivision is 0 -> the tick->X mapping is
	// undefined. Return 0 so the now-line sits at the origin and nothing divides by
	// zero (which would yield NaN and corrupt the camera / note placement).
	if (m_TimeDivision == 0) return 0.0f;
	return ((float)curTickTime * m_QuarterNoteLength / (float)m_TimeDivision);
}

//******************************************************************************
// ライブモニタ用ノート位置取得
//******************************************************************************
float MTNoteDesign::GetLivePosX(
		unsigned long elapsedTime
	)
{
	return (((float)elapsedTime / 1000.0f) * m_LiveNoteLengthPerSecond);
}

//******************************************************************************
// ノートボックス中心座標取得
//******************************************************************************
D3DXVECTOR3 MTNoteDesign::GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
	)
{
	D3DXVECTOR3 vector;
	float pb = 0.0f;

	//ピッチベンドによるY座標の移動量
	if (pitchBendValue < 0) {
		pb = GetNoteStep() * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		pb = GetNoteStep() * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	//演奏位置
	vector.x = GetPlayPosX(curTickTime);

	//ノート番号
	vector.y = GetPortOriginY(portNo) + (m_NoteStep * noteNo + pb);

	//ポート番号とチャンネル番号
	vector.z = GetPortOriginZ(portNo) + (GetChStep() * chNo);

	return vector;
}

//******************************************************************************
// ノートボックス縦サイズ取得
//******************************************************************************
float MTNoteDesign::GetNoteBoxHeight()
{
	return m_NoteBoxHeight;
}

//******************************************************************************
// ノートボックス横サイズ取得
//******************************************************************************
float MTNoteDesign::GetNoteBoxWidth()
{
	return m_NoteBoxWidth;
}

//******************************************************************************
// ノート間隔取得
//******************************************************************************
float MTNoteDesign::GetNoteStep()
{
	return m_NoteStep;
}

//******************************************************************************
// チャンネル間隔取得
//******************************************************************************
float MTNoteDesign::GetChStep()
{
	return m_ChStep;
}

//******************************************************************************
//ライブモニタ表示期間（ミリ秒）
//******************************************************************************
unsigned long MTNoteDesign::GetLiveMonitorDisplayDuration()
{
	return (unsigned long)m_LiveMonitorDisplayDuration;
}

//******************************************************************************
// ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesign::GetNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度

	)
{
	D3DXVECTOR3 center;
	float bh = 0.0f;
	float bw = 0.0f;

	center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo, pitchBendValue, pitchBendSensitivity);

	bh = GetNoteBoxHeight();
	bw = GetNoteBoxWidth();

	*pVector0 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// 発音中ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesign::GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity,	//省略可：ピッチベンド感度
		unsigned long elapsedTime			//省略可：経過時間（ミリ秒）
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
	bw = GetNoteBoxWidth() * curSizeRatio;
	
	*pVector0 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(center.x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(center.x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// ライブモニタ用ノートボックス頂点座標取得
//******************************************************************************
void MTNoteDesign::GetNoteBoxVirtexPosLive(
		unsigned long elapsedTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3,	//YZ平面+X軸方向を見て右下
		short pitchBendValue,				//省略可：ピッチベンド
		unsigned char pitchBendSensitivity	//省略可：ピッチベンド感度
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
	bw = GetNoteBoxWidth();
	
	*pVector0 = D3DXVECTOR3(x, center.y+(bh/2.0f), center.z+(bw/2.0f));
	*pVector1 = D3DXVECTOR3(x, center.y+(bh/2.0f), center.z-(bw/2.0f));
	*pVector2 = D3DXVECTOR3(x, center.y-(bh/2.0f), center.z+(bw/2.0f));
	*pVector3 = D3DXVECTOR3(x, center.y-(bh/2.0f), center.z-(bw/2.0f));
}

//******************************************************************************
// グリッドボックス頂点座標取得
//******************************************************************************
void MTNoteDesign::GetGridBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3 	//YZ平面+X軸方向を見て右下
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
	bw = GetNoteBoxWidth();

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
// グリッドボックス頂点座標取得
//******************************************************************************
void MTNoteDesign::GetGridBoxVirtexPosLive(
		unsigned long elapsedTime,
		unsigned char portNo,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3 	//YZ平面+X軸方向を見て右下
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
	bw = GetNoteBoxWidth();
	
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
// 再生面頂点座標取得
//******************************************************************************
void MTNoteDesign::GetPlaybackSectionVirtexPos(
		unsigned long curTickTime,
		D3DXVECTOR3* pVector0,	//YZ平面+X軸方向を見て左上
		D3DXVECTOR3* pVector1,	//YZ平面+X軸方向を見て右上
		D3DXVECTOR3* pVector2,	//YZ平面+X軸方向を見て左下
		D3DXVECTOR3* pVector3 	//YZ平面+X軸方向を見て右下
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
// 波紋縦サイズ取得
//******************************************************************************
float MTNoteDesign::GetRippleHeight(
		unsigned long elapsedTime	//省略可：経過時間（ミリ秒）
	)
{
	float height = 0.0f;

	if ((int)elapsedTime <= m_RippleDuration) {
		height = m_RippleHeight * (1.0f - ((float)elapsedTime / m_RippleDuration));
	}

	return height;
}

//******************************************************************************
// 波紋横サイズ取得
//******************************************************************************
float MTNoteDesign::GetRippleWidth(
		unsigned long elapsedTime	//省略可：経過時間（ミリ秒）
	)
{
	float width = 0.0f;

	if ((int)elapsedTime <= m_RippleDuration) {
		width = m_RippleWidth * (1.0f - ((float)elapsedTime / m_RippleDuration));
	}

	return width;
}

//******************************************************************************
// 波紋透明度取得
//******************************************************************************
float MTNoteDesign::GetRippleAlpha(
		unsigned long elapsedTime	//経過時間（ミリ秒）
	)
{
	float alpha = 1.0f;

	if ((int)elapsedTime <= m_RippleDuration) {
		alpha = 1.0f - ((float)elapsedTime / m_RippleDuration);
	}

	return alpha;
}

//******************************************************************************
// ピクチャボード相対位置取得
//******************************************************************************
float MTNoteDesign::GetPictBoardRelativePos()
{
	return m_PictBoardRelativePos;
}

//******************************************************************************
// ポート原点Y座標取得
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
// ポート原点Z座標取得
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
// 世界座標配置移動ベクトル取得
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
// ノートボックスカラー取得
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	D3DXCOLOR color;

	if (m_NoteColorType == Channel) {
		//チャンネル番号によって色を変える
		if (chNo >= 16) {
			//データ異常だが無視する
			color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); //RGBA
		}
		else {
			color = m_NoteColor[chNo];
		}
	}
	else if (m_NoteColorType == Scale) {
		//音階によって色を変える
		color = m_NoteColorOfScale[(noteNo % 12)];
	}
	else if (m_NoteColorType == ChannelTrack) {
		//�g���b�N�~�`�����l���̃J���[���[�v�i�g���b�N�����͂����ł͕s���̂�
		//�`�����l���F�Ƀt�H�[���o�b�N�B�����̃g���b�N�F�̓����_���ŏĂ��j
		color = m_NoteColor[(chNo < 16) ? chNo : 0];
	}

	return color;
}

//******************************************************************************
// track x channel color-loop mode?
//******************************************************************************
bool MTNoteDesign::IsTrackColorMode()
{
	return (m_NoteColorType == ChannelTrack);
}

//******************************************************************************
// track x channel color (golden-angle hue cycle, HSV -> RGB)
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetTrackChannelColor(
		unsigned char trackNo,
		unsigned char chNo
	)
{
	// ced 20260628: CHANNELTRACK の配色方式を変更。
	//   - 各チャンネルの「最初に現れたトラック」はそのチャンネル色（パレット[ch]）。
	//   - 同じチャンネルを共有する2本目以降のトラックは (track,channel) の決定論ハッシュで
	//     パレット16色のいずれかへ分散して色を分ける。
	//   ノート順に呼ばれる前提で「最初のトラック」を on-the-fly に記録する
	//   （m_FirstTrackForChannel は Initialize でリセット）。
	unsigned char ch = (chNo < 16) ? chNo : 0;

	if (m_FirstTrackForChannel[ch] < 0) {
		m_FirstTrackForChannel[ch] = (int)trackNo;   //このchで最初に現れたトラック
	}

	if (m_FirstTrackForChannel[ch] == (int)trackNo) {
		//主トラック：チャンネル色
		return m_NoteColor[ch];
	}

	//副トラック：決定論ハッシュ（Knuth 乗算）でパレット index 0-15 に分散
	unsigned int h = (unsigned int)trackNo * 2654435761u;
	h ^= (unsigned int)chNo * 40503u;
	unsigned int idx = (h >> 16) % 16u;
	return m_NoteColor[idx];
}

//******************************************************************************
// build merged note list keeping each note's source track number
//******************************************************************************
int MTNoteDesign::BuildMergedNoteListWithTrack(
		SMSeqData* pSeqData,
		SMNoteList* pNoteList,
		std::vector<unsigned char>* pTrackNoList
	)
{
	int result = 0;
	unsigned long trackNum = 0;
	unsigned long t = 0;
	unsigned long n = 0;
	size_t k = 0;
	SMTrack track;
	SMNoteList tmpList;
	SMNote note;
	std::vector<MTNoteTrackItem> items;

	if ((pSeqData == NULL) || (pNoteList == NULL) || (pTrackNoList == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pNoteList->Clear();
	pTrackNoList->clear();

	// gather (note, trackNo) across all tracks (per-track lists carry absolute
	// tick times, so all tracks share one timeline)
	trackNum = pSeqData->GetTrackNum();
	for (t = 0; t < trackNum; t++) {
		result = pSeqData->GetTrack(t, &track);
		if (result != 0) goto EXIT;

		result = track.GetNoteList(&tmpList);
		if (result != 0) goto EXIT;

		for (n = 0; n < tmpList.GetSize(); n++) {
			result = tmpList.GetNote(n, &note);
			if (result != 0) goto EXIT;

			MTNoteTrackItem item;
			item.note = note;
			item.trackNo = (unsigned char)(t & 0xFF);
			items.push_back(item);
		}
	}

	// sort by start tick. Sort small (startTime,index) keys instead of the
	// 44-byte items - the old stable_sort moved gigabytes for a Black MIDI.
	// std::pair's default order = (startTime, originalIndex) = stable by start.
	{
		std::vector<std::pair<unsigned long, unsigned long> > keys(items.size());
		for (k = 0; k < items.size(); k++) {
			keys[k].first = items[k].note.startTime;
			keys[k].second = (unsigned long)k;
		}
		std::sort(keys.begin(), keys.end());

		for (k = 0; k < keys.size(); k++) {
			unsigned long src = keys[k].second;
			result = pNoteList->AddNote(items[src].note);
			if (result != 0) goto EXIT;
			pTrackNoList->push_back(items[src].trackNo);
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートボックスカラー取得
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

	//m_ActiveNoteDuration リリースタイム
	//  発音開始時点からノート色を元に戻すまでの時間
	//  ただし m_ActiveNoteEmissive によってリリース後もノートOFFまで発光する

	//m_ActiveNoteWhiteRate 最大白色率
	//  0.0 → ノート色変化なし
	//  0.5 → ノート色と白の中間色
	//  1.0 → 白

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
// 発音中ノートボックスエミッシブ取得（マテリアル用）
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetActiveNoteEmissive()
{
	return m_ActiveNoteEmissive;
}

//******************************************************************************
// active-note swell ratio / white-flash rate ([ActiveNote] conf)
//******************************************************************************
float MTNoteDesign::GetActiveNoteBoxSizeRatio()
{
	return m_ActiveNoteBoxSizeRatio;
}

float MTNoteDesign::GetActiveNoteWhiteRate()
{
	return m_ActiveNoteWhiteRate;
}

//******************************************************************************
// グリッドラインカラー取得
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetGridLineColor()
{
	return m_GridLineColor;
}

//******************************************************************************
// 再生面カラー取得
//******************************************************************************
D3DXCOLOR MTNoteDesign::GetPlaybackSectionColor()
{
	return m_PlaybackSectionColor;
}

//******************************************************************************
// クリア
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
		m_FirstTrackForChannel[i] = -1;   //ced 20260628: CHANNELTRACK 用
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
// 設定ファイル読み込み
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
	//スケール情報
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
	//色情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;

	//ノートカラー種別を取得
	result = confFile.GetStr(_T("NoteColorType"), noteColorType, 16, _T("CHANNEL"));
	if (result != 0) goto EXIT;

	//ノートカラー種別を決定
	m_NoteColorType = Channel;
	if (_tcscmp(noteColorType, _T("SCALE")) == 0) {
		m_NoteColorType = Scale;
	}
	else if (_tcscmp(noteColorType, _T("CHANNELTRACK")) == 0) {
		m_NoteColorType = ChannelTrack;
	}

	//ノート色／グリッドライン色を取得（1.4.1 カラーパレット対応）
	//  選択中のカラーパレットから取得する。パレット0＝デフォルトはシーンiniの
	//  Ch-NN-NoteRGBA / GridLineRGBA を読むため、従来と同一色になる（退行なし）。
	//  ※ Mod 機能（ActiveKeyColor / EmissiveRGBA / CaptionRGBA）は別系統で ini のまま。
	{
		MTColorConf colorConf;
		MTColorPalette colorPalette;
		D3DXCOLOR palColor;
		result = colorConf.Initialize(pSceneName);
		if (result != 0) goto EXIT;
		colorConf.GetSelectedColorPalette(&colorPalette);

		//ノート色情報（パレットから）
		for (i = 0; i < 16; i++) {
			result = colorPalette.GetChColor(i, &palColor);
			if (result != 0) goto EXIT;
			m_NoteColor[i] = palColor;
		}

		//グリッドライン色情報（パレットから）
		colorPalette.GetGridLineColor(&m_GridLineColor);
	}

	//音階用ノート色情報を取得（パレット非対応：iniから取得）
	for (i = 0; i < 12; i++) {
		_stprintf_s(key, 32, _T("Scale-%02d-NoteRGBA"), i+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;

		m_NoteColorOfScale[i] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	}

	//再生面色情報を取得
	result = confFile.GetStr(_T("PlaybackSectionRGBA"), hexColor, 16, _T("AAAAFFFF"));
	if (result != 0) goto EXIT;
	m_PlaybackSectionColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	//----------------------------------
	//発音ノート情報
	//----------------------------------
	result = confFile.SetCurSection(_T("ActiveNote"));
	if (result != 0) goto EXIT;

	//発音中ノート色情報：継続時間(msec)
	result = confFile.GetInt(_T("Duration"), &m_ActiveNoteDuration, 400);
	if (result != 0) goto EXIT;

	//発音中ノート色情報：白色率
	result = confFile.GetFloat(_T("WhiteRate"), &m_ActiveNoteWhiteRate, 0.9f);
	if (result != 0) goto EXIT;

	//発音中ノート色情報：マテリアル発光色
	result = confFile.GetStr(_T("EmissiveRGBA"), hexColor, 16, _T("1A1A1A1A"));
	if (result != 0) goto EXIT;
	m_ActiveNoteEmissive = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	//発音中ノート情報：ボックスサイズ比率
	result = confFile.GetFloat(_T("SizeRatio"), &m_ActiveNoteBoxSizeRatio, 1.4f);
	if (result != 0) goto EXIT;

	//----------------------------------
	//波紋情報
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	//波紋継続時間(msec)
	result = confFile.GetInt(_T("Duration"), &m_RippleDuration, 1600);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ユーザ設定読み込み
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
	
	//四分音符長拡大率(0-1000)
	result = confFile.SetCurSection(_T("QuarterNote"));
	result = confFile.GetInt(_T("LengthMagnification"), &lengthMagnification, 100);
	
	//クリッピング
	if (lengthMagnification < 0) {
		lengthMagnification = 0;
	}
	if (lengthMagnification > 1000) {
		lengthMagnification = 1000;
	}

	//四分音符の長さを更新
	m_QuarterNoteLength = m_QuarterNoteLength * ((float)lengthMagnification / 100.0f);

EXIT:;
	return result;
}

