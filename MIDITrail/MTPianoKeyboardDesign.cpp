//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign
//
// ピアノキーボードデザインクラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardDesign.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//テクスチャ座標算出：ビットマップサイズ = 562 x 562
#define TEXTURE_POINT(x, y)  (D3DXVECTOR2((float)x/561.0f, (float)y/561.0f))

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPianoKeyboardDesign::MTPianoKeyboardDesign(void)
{
	_Initialize();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPianoKeyboardDesign::~MTPianoKeyboardDesign(void)
{
}

//******************************************************************************
// 初期化
//******************************************************************************
int MTPianoKeyboardDesign::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long portIndex = 0;
	unsigned char portNo = 0;

	//ライブモニタ向け設定
	if (pSeqData == NULL) {
		//ポートリスト
		m_PortList.Clear();
		m_PortList.AddPort(0);
	}
	//通常設定
	else {
		//ポートリスト取得
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
	}

	//設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//ポート番号に昇順のインデックスを振る
	//ポート 0番 3番 5番 に出力する場合のインデックスはそれぞれ 0, 1, 2
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_PortIndex[index] = 0;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = (unsigned char)portIndex;
		portIndex++;
	}

	//キー種別初期化
	_InitKeyType();

	//キー座標設定
	_InitKeyPos();

EXIT:;
	return result;
}

//******************************************************************************
// 初期化
//******************************************************************************
void MTPianoKeyboardDesign::_Initialize()
{
	unsigned long i = 0;

	ZeroMemory(&(m_KeyInfo[0]), sizeof(MTKeyInfo) * SM_MAX_NOTE_NUM);

	for (i = 0; i < SM_MAX_PORT_NUM; i++) {
		m_PortIndex[i] = 0;
	}

	//キーのポリゴン座標はベタに作りこんであるため
	//これに関するパラメータは設定ファイルに記載しない

	m_WhiteKeyStep      = 0.236f;
	m_WhiteKeyWidth     = 0.226f;
	m_WhiteKeyHeight    = 0.22f;
	m_WhiteKeyLen       = 1.50f;
	m_BlackKeyWidth     = 0.10f;
	m_BlackKeyHeight    = 0.34f;
	m_BlackKeySlopeLen  = 0.08f;
	m_BlackKeyLen       = 1.00f;
	m_KeySpaceSize      = 0.01f;
	m_KeyRotateAxisXPos = 2.36f;
	m_KeyRotateAngle    = 3.00f;
	m_KeyDownDuration   = 40;         //設定ファイル
	m_KeyUpDuration     = 40;         //設定ファイル
	m_KeyboardStepY     = 0.34f;      //設定ファイル
	m_KeyboardStepZ     = 1.50f;      //設定ファイル
	m_NoteDropPosZ4WhiteKey = 0.25f;
	m_NoteDropPosZ4BlackKey = 0.75f;
	m_BlackKeyShiftCDE  = 0.0216f;    //テクスチャ画像 7ドット相当
	m_BlackKeyShiftFGAB = 0.0340f;    //テクスチャ画像11ドット相当
	m_KeyboardMaxDispNum = 16;        //設定ファイル
	m_WhiteKeyColor =  DXColorUtil::MakeColorFromHexRGBA(_T("FFFFFFFF")); //設定ファイル
	m_BlackKeyColor =  DXColorUtil::MakeColorFromHexRGBA(_T("FFFFFFFF")); //設定ファイル
	m_ActiveKeyColorType = DefaultColor;  //設定ファイル
	m_ActiveKeyColor = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF")); //設定ファイル
	m_ActiveKeyColorDuration = 400;   //設定ファイル
	m_ActiveKeyColorTailRate = 0.5f;  //設定ファイル
	m_KeyDispRangeStart = 0;
	m_KeyDispRangeEnd   = 127;

	return;
}

//******************************************************************************
// キー種別初期化
//******************************************************************************
void MTPianoKeyboardDesign::_InitKeyType()
{
	unsigned long i = 0;
	unsigned char noteNo = 0;
	KeyType type = KeyWhiteC;

	//実際の鍵盤では黒鍵が微妙にずれて配置されているため
	//厳密には(C,F)(D,G,A)(E,B)の形はすべて異なる

	for (i = 0; i < 10; i++) {
		noteNo = (unsigned char)i * 12;				//  ________ 
		m_KeyInfo[noteNo + 0].keyType = KeyWhiteC;	// |        |C
		m_KeyInfo[noteNo + 1].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 2].keyType = KeyWhiteD;	// |        |D
		m_KeyInfo[noteNo + 3].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 4].keyType = KeyWhiteE;	// |________|E
		m_KeyInfo[noteNo + 5].keyType = KeyWhiteF;	// |        |F
		m_KeyInfo[noteNo + 6].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 7].keyType = KeyWhiteG;	// |        |G
		m_KeyInfo[noteNo + 8].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 9].keyType = KeyWhiteA;	// |        |A
		m_KeyInfo[noteNo +10].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo +11].keyType = KeyWhiteB;	// |________|B
	}
	noteNo = 120;									//  ________ 
	m_KeyInfo[noteNo + 0].keyType = KeyWhiteC;		// |        |C
	m_KeyInfo[noteNo + 1].keyType = KeyBlack;		// |----####|
	m_KeyInfo[noteNo + 2].keyType = KeyWhiteD;		// |        |D
	m_KeyInfo[noteNo + 3].keyType = KeyBlack;		// |----####|
	m_KeyInfo[noteNo + 4].keyType = KeyWhiteE;		// |________|E
	m_KeyInfo[noteNo + 5].keyType = KeyWhiteF;		// |        |F
	m_KeyInfo[noteNo + 6].keyType = KeyBlack;		// |----####|
	m_KeyInfo[noteNo + 7].keyType = KeyWhiteB;		// |________|G <= 形状はB

	//キー表示範囲：開始キーの調整
	type = m_KeyInfo[m_KeyDispRangeStart].keyType;
	switch (type) {
		case KeyWhiteC: type = KeyWhiteC; break;
		case KeyWhiteD: type = KeyWhiteC; break;
		case KeyWhiteE: type = KeyWhiteE; break; //変更対象なし
		case KeyWhiteF: type = KeyWhiteF; break;
		case KeyWhiteG: type = KeyWhiteF; break;
		case KeyWhiteA: type = KeyWhiteF; break;
		case KeyWhiteB: type = KeyWhiteB; break; //変更対象なし
		default: break;
	}
	m_KeyInfo[m_KeyDispRangeStart].keyType = type;

	//キー表示範囲：終了キーの調整
	type = m_KeyInfo[m_KeyDispRangeEnd].keyType;
	switch (type) {
		case KeyWhiteC: type = KeyWhiteC; break; //変更対象なし
		case KeyWhiteD: type = KeyWhiteE; break;
		case KeyWhiteE: type = KeyWhiteE; break;
		case KeyWhiteF: type = KeyWhiteF; break; //変更対象なし
		case KeyWhiteG: type = KeyWhiteB; break;
		case KeyWhiteA: type = KeyWhiteB; break;
		case KeyWhiteB: type = KeyWhiteB; break;
		default: break;
	}
	m_KeyInfo[m_KeyDispRangeEnd].keyType = type;

	return;
}

//******************************************************************************
// キー座標設定
//******************************************************************************
void MTPianoKeyboardDesign::_InitKeyPos()
{
	unsigned char noteNo = 0;
	KeyType prevKeyType = KeyWhiteB;
	float posX = 0.0f;
	float shift = 0.0f;

	//先頭ノートの位置
	//posX = GetWhiteKeyStep() / 2.0f;
	m_KeyInfo[noteNo].keyCenterPosX = posX;
	prevKeyType = m_KeyInfo[noteNo].keyType;

	//実際の鍵盤では黒鍵が微妙にずれて配置されている
	//まず白鍵と白鍵の中点に黒鍵を配置して後から補正する

	//2番目以降のノートの位置
	for (noteNo = 1; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		//直前のキーが黒鍵
		if (prevKeyType == KeyBlack) {
			if (m_KeyInfo[noteNo].keyType == KeyBlack) {
				//黒鍵の後に黒鍵はありえない
			}
			else {
				//白鍵と白鍵の中央に黒鍵を配置する
				//実際の鍵盤と異なるが工数削減のため目をつぶる
				posX += (GetWhiteKeyStep() / 2.0f);
			}
		}
		//直前のキーが白鍵
		else {
			if (m_KeyInfo[noteNo].keyType == KeyBlack) {
				posX += (GetWhiteKeyStep() / 2.0f);
			}
			else {
				posX += GetWhiteKeyStep();
			}
		}
		m_KeyInfo[noteNo].keyCenterPosX = posX;
		prevKeyType = m_KeyInfo[noteNo].keyType;
	}

	//黒鍵の配置を補正する
	prevKeyType = KeyWhiteC;
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		if (m_KeyInfo[noteNo].keyType == KeyBlack) {
			//黒鍵の位置補正量を取得
			switch (prevKeyType) {
				case KeyWhiteC: shift = -m_BlackKeyShiftCDE;  break;
				case KeyWhiteD: shift = +m_BlackKeyShiftCDE;  break;
				case KeyWhiteF: shift = -m_BlackKeyShiftFGAB; break;
				case KeyWhiteG: shift =  0.00f;               break;
				case KeyWhiteA: shift = +m_BlackKeyShiftFGAB; break;
				default:        shift =  0.00f;               break;
			}
			//最後の黒鍵は中点に配置
			if (noteNo == 126) {
				shift = 0.00f;
			}
			
			//表示範囲の先頭末尾でひとつだけ取り残される黒鍵は中央に配置する
			if ((noteNo - 1) == m_KeyDispRangeStart) {
				if ((m_KeyInfo[noteNo + 1].keyType == KeyWhiteE) 
				 || (m_KeyInfo[noteNo + 1].keyType == KeyWhiteB)) {
					shift =  0.00f;
				}
			}
			if ((noteNo + 1) == m_KeyDispRangeEnd) {
				if ((m_KeyInfo[noteNo - 1].keyType == KeyWhiteD) 
				 || (m_KeyInfo[noteNo - 1].keyType == KeyWhiteF)) {
					shift =  0.00f;
				}
			}
			
			//位置補正
			m_KeyInfo[noteNo].keyCenterPosX += shift;
		}
		prevKeyType = m_KeyInfo[noteNo].keyType;
	}

	return;
}

//******************************************************************************
// ポート原点X座標取得
//******************************************************************************
float MTPianoKeyboardDesign::GetPortOriginX(
		unsigned char portNo
	)
{
	float keyboardWidth = 0.0f;
	float originX = 0.0f;

	//             +z
	//              |
	//         +----+----+
	//   Ch.15 |    |    |  @:OriginX(for portA,B,C)
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portC
	//         @----+----+
	//   Ch.15 |    |    |
	//         |    |    |
	// -x<-----|----0----|----->+x
	//         |    |    |
	//   Ch. 0 |    |    | portB
	//         @----+----+
	//   Ch.15 |    |    |
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portA
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	keyboardWidth = GetWhiteKeyStep() * (float)(SM_MAX_NOTE_NUM - 53);
	originX = (-keyboardWidth) / 2.0f;

	return originX;
}

//******************************************************************************
// ポート原点Y座標取得
//******************************************************************************
float MTPianoKeyboardDesign::GetPortOriginY(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portHeight = 0.0f;
	float originY = 0.0f;
	float totalHeight = 0.0f;
	unsigned long chNum = 0;

	//     +--+ Ch.15            +y
	//     |   +--+               |
	//     |       +--+           |
	//     |           +--+       |
	//     +--------------@ Ch.0  |
	//     portC           +--+ Ch.15
	//                     |   +--+
	// +z<------------------------0+--+--------------------->-z
	//                     |      |    +--+
	//                     +------|-------@ Ch.0
	//                     portB  |        +--+ Ch.15
	//                            |        |   +--+
	//                            |        |       +--+
	//                            |        |           +--+
	//                            |        +--------------@ Ch.0
	//                           -y        portA

	portIndex = (float)(m_PortIndex[portNo]);
	portHeight =(m_KeyboardStepY * (float)(SM_MAX_CH_NUM -1)) + GetBlackKeyHeight();

	//表示チャンネル数
	chNum = m_PortList.GetSize() * SM_MAX_CH_NUM;
	if ((unsigned long)m_KeyboardMaxDispNum < chNum) {
		chNum = m_KeyboardMaxDispNum;
	}

	totalHeight = portHeight * ((float)chNum / (float)SM_MAX_CH_NUM);
	originY = (portHeight * portIndex) - (totalHeight / 2.0f);

	return originY;
}

//******************************************************************************
// ポート原点Z座標取得
//******************************************************************************
float MTPianoKeyboardDesign::GetPortOriginZ(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portLen = 0.0f;
	float originZ = 0.0f;
	float totalLen = 0.0f;
	unsigned long chNum = 0;

	//             +z
	//              |
	//         +----+----+
	//   Ch.16 |    |    |  @:OriginX(for portA,B,C)
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portC
	//         @----+----+
	//   Ch.16 |    |    |
	//         |    |    |
	// -x<-----|----0----|----->+x
	//         |    |    |
	//   Ch. 0 |    |    | portB
	//         @----+----+
	//   Ch.16 |    |    |
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portA
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	portIndex = (float)(m_PortIndex[portNo]);
	portLen =(m_KeyboardStepZ * (float)(SM_MAX_CH_NUM -1)) + GetWhiteKeyLen();

	//表示チャンネル数
	chNum = m_PortList.GetSize() * SM_MAX_CH_NUM;
	if ((unsigned long)m_KeyboardMaxDispNum < chNum) {
		chNum = m_KeyboardMaxDispNum;
	}

	totalLen = portLen * ((float)chNum / (float)SM_MAX_CH_NUM);
	originZ = (portLen * portIndex) - (totalLen / 2.0f);

	return originZ;
}

//******************************************************************************
// キー種別取得
//******************************************************************************
MTPianoKeyboardDesign::KeyType MTPianoKeyboardDesign::GetKeyType(
		unsigned char noteNo
	)
{
	KeyType keyType = KeyWhiteC;

	if (noteNo < SM_MAX_NOTE_NUM) {
		keyType = m_KeyInfo[noteNo].keyType;
	}

	return keyType;
}

//******************************************************************************
// キー中心X座標取得
//******************************************************************************
float MTPianoKeyboardDesign::GetKeyCenterPosX(
		unsigned char noteNo
	)
{
	float centerPosX = 0.0f;

	if (noteNo < SM_MAX_NOTE_NUM) {
		centerPosX = m_KeyInfo[noteNo].keyCenterPosX;
	}

	return centerPosX;
}

//******************************************************************************
// 白鍵配置間隔取得
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyStep()
{
	return m_WhiteKeyStep;
}

//******************************************************************************
// 白鍵横サイズ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyWidth()
{
	return m_WhiteKeyWidth;
}

//******************************************************************************
// 白鍵高さ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyHeight()
{
	return m_WhiteKeyHeight;
}

//******************************************************************************
// 白鍵長さ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyLen()
{
	return m_WhiteKeyLen;
}

//******************************************************************************
// 黒鍵横サイズ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeyWidth()
{
	return m_BlackKeyWidth;
}

//******************************************************************************
// 黒鍵高さ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeyHeight()
{
	return m_BlackKeyHeight;
}

//******************************************************************************
// 黒鍵傾斜長さ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeySlopeLen()
{
	return m_BlackKeySlopeLen;
}

//******************************************************************************
// 黒鍵長さ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeyLen()
{
	return m_BlackKeyLen;
}

//******************************************************************************
// キー間隔サイズ取得
//******************************************************************************
float MTPianoKeyboardDesign::GetKeySpaceSize()
{
	return m_KeySpaceSize;
}

//******************************************************************************
// キー押下回転中心Y軸座標取得
//******************************************************************************
float MTPianoKeyboardDesign::GetKeyRotateAxisXPos()
{
	return m_KeyRotateAxisXPos;
}

//******************************************************************************
// キー押下回転角度
//******************************************************************************
float MTPianoKeyboardDesign::GetKeyRotateAngle()
{
	return m_KeyRotateAngle;
}

//******************************************************************************
// キー下降時間取得(msec)
//******************************************************************************
unsigned long MTPianoKeyboardDesign::GetKeyDownDuration()
{
	return (unsigned long)m_KeyDownDuration;
}

//******************************************************************************
// キー上昇時間取得(msec)
//******************************************************************************
unsigned long MTPianoKeyboardDesign::GetKeyUpDuration()
{
	return (unsigned long)m_KeyUpDuration;
}

//******************************************************************************
// ノートドロップ座標取得
//******************************************************************************
float MTPianoKeyboardDesign::GetNoteDropPosZ(
		unsigned char noteNo
	)
{
	float dropPosZ = 0.0f;

	if (m_KeyInfo[noteNo].keyType == KeyBlack) {
		dropPosZ = m_NoteDropPosZ4BlackKey;
	}
	else {
		dropPosZ = m_NoteDropPosZ4WhiteKey;
	}

	return dropPosZ;
}

//******************************************************************************
// ピッチベンドキーボードシフト量取得
//******************************************************************************
float MTPianoKeyboardDesign::GetPitchBendShift(
		short pitchBendValue,				//ピッチベンド
		unsigned char pitchBendSensitivity	//ピッチベンド感度
	)
{
	float shift = 0.0f;
	float noteStep = 0.0f;

	//半音の移動量
	//  キーの配置間隔は B->C, E->F の間に黒鍵が存在しないため均一ではない
	//  1オクターブでつじつまが合うように半音のシフト量を決める
	noteStep = GetWhiteKeyStep() * 7.0f / 12.0f;

	//ピッチベンドによるキーボード移動量
	if (pitchBendValue < 0) {
		shift = noteStep * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		shift = noteStep * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	return shift;
}

//******************************************************************************
// 白鍵カラー取得
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesign::GetWhiteKeyColor()
{
	return m_WhiteKeyColor;
}

//******************************************************************************
// 黒鍵カラー取得
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesign::GetBlackKeyColor()
{
	return m_BlackKeyColor;
}

//******************************************************************************
// 発音中キーカラー取得
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesign::GetActiveKeyColor(
		unsigned char noteNo,
		unsigned long elapsedTime,
		D3DXCOLOR* pNoteColor
	)
{
	D3DXCOLOR color;
	float r,g,b,a = 0.0f;
	float rate = 0.0f;
	unsigned long duration = 0;

	//          on     off
	//   白 |---+......+---- ←offになったら白鍵の色に戻す
	//      |   :      :
	//      |   :  +---+     ←offになるまで中間色のまま
	//      |   : /:   :
	//      |   :/ :   :
	//   赤 |   +  :   :     ←キー押下直後の色（赤）
	//      |   :\ :   :
	//      |   : \:   :
	//      |   :  +---+     ←offになるまで中間色のまま
	//      |   :  :   :
	//   黒 |---+  :   +---- ←offになったら黒鍵の色に戻す
	//   ---+---*------*-------> +t
	//      |   on :   off
	//          <-->duration

	if ((pNoteColor != NULL) && (m_ActiveKeyColorType == NoteColor)) {
		//ノート色が指定されている場合
		color = *pNoteColor;
	}
	else {
		//それ以外はデフォルト色とする
		color = m_ActiveKeyColor;
	}

	duration = (unsigned long)m_ActiveKeyColorDuration;
	rate     = m_ActiveKeyColorTailRate;

	if (elapsedTime < duration) {
		rate = ((float)elapsedTime / (float)duration) * m_ActiveKeyColorTailRate;
	}

	if (GetKeyType(noteNo) == KeyBlack) {
		r = color.r - ((color.r) * rate);
		g = color.g - ((color.g) * rate);
		b = color.b - ((color.b) * rate);
		a = color.a;
	}
	else {
		r = color.r + ((1.0f - color.r) * rate);
		g = color.g + ((1.0f - color.g) * rate);
		b = color.b + ((1.0f - color.b) * rate);
		a = color.a;
	}
	color = D3DXCOLOR(r, g, b, a);

	return color;
}

//******************************************************************************
// 白鍵テクスチャ座標取得：上面
//******************************************************************************
void MTPianoKeyboardDesign::GetWhiteKeyTexturePosTop(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos0,
		D3DXVECTOR2* pTexPos1,
		D3DXVECTOR2* pTexPos2,
		D3DXVECTOR2* pTexPos3,
		D3DXVECTOR2* pTexPos4,
		D3DXVECTOR2* pTexPos5,
		D3DXVECTOR2* pTexPos6,
		D3DXVECTOR2* pTexPos7
	)
{
	unsigned long index = 0;
	unsigned long x = 0;
	unsigned long y = 1;

	// 6+-+5       6+-+5       6+-+5  6+-+5       6+-+5     6+-+5       6+-+5
	//  | |         | |         | |    | |         | |       | |         | |
	//  | |         | |         | |    | |         | |       | |         | |
	// 7| |4       7| |4       7| |4  7| |4       7| |4     7| |4       7| |4
	// 3+-+---+2 3+-+-+-+2 3+---+-+2  3+-+---+2 3+-+-+-+2 3+-+-+-+2 3+---+-+2
	//  |     |   |     |   |     |    |     |   |     |   |     |   |     |
	//  |  C  |   |  D  |   |  E  |    |  F  |   |  G  |   |  A  |   |  B  |
	//  |     |   |     |   |     |    |     |   |     |   |     |   |     |
	// 0+-----+1 0+-----+1 0+-----+1  0+-----+1 0+-----+1 0+-----+1 0+-----+1
                                                          
	unsigned long pos[7][8][2] = {
		// 0           1           2           3           4              5              6              7
		{ {  3, 488}, { 77, 488}, { 77, 330}, { 3,  330}, { 56- 7, 330}, { 56- 7,   1}, {  3   ,   1}, {  3   , 330} }, // C
		{ { 79, 488}, {154, 488}, {154, 330}, { 79, 330}, {133+ 7, 330}, {133+ 7,   1}, { 99- 7,   1}, { 99- 7, 330} }, // D
		{ {156, 488}, {230, 488}, {230, 330}, {156, 330}, {230   , 330}, {230   ,   1}, {176+ 7,   1}, {176+ 7, 330} }, // E
		{ {232, 488}, {307, 488}, {307, 330}, {232, 330}, {286-11, 330}, {286-11,   1}, {232   ,   1}, {232   , 330} }, // F
		{ {309, 488}, {384, 488}, {384, 330}, {309, 330}, {363   , 330}, {363   ,   1}, {329-11,   1}, {329-11, 330} }, // G
		{ {386, 488}, {460, 488}, {460, 330}, {386, 330}, {440+11, 330}, {440+11,   1}, {406   ,   1}, {406   , 330} }, // A
		{ {462, 488}, {537, 488}, {537, 330}, {462, 330}, {537   , 330}, {537   ,   1}, {483+11,   1}, {483+11, 330} }  // B
	};

	switch(GetKeyType(noteNo)) {
		case KeyWhiteC: index = 0; break;
		case KeyWhiteD: index = 1; break;
		case KeyWhiteE: index = 2; break;
		case KeyWhiteF: index = 3; break;
		case KeyWhiteG: index = 4; break;
		case KeyWhiteA: index = 5; break;
		case KeyWhiteB: index = 6; break;
	}

	*pTexPos0 = TEXTURE_POINT(pos[index][0][x], pos[index][0][y]);
	*pTexPos1 = TEXTURE_POINT(pos[index][1][x], pos[index][1][y]);
	*pTexPos2 = TEXTURE_POINT(pos[index][2][x], pos[index][2][y]);
	*pTexPos3 = TEXTURE_POINT(pos[index][3][x], pos[index][3][y]);
	*pTexPos4 = TEXTURE_POINT(pos[index][4][x], pos[index][4][y]);
	*pTexPos5 = TEXTURE_POINT(pos[index][5][x], pos[index][5][y]);
	*pTexPos6 = TEXTURE_POINT(pos[index][6][x], pos[index][6][y]);
	*pTexPos7 = TEXTURE_POINT(pos[index][7][x], pos[index][7][y]);

	return;
}

//******************************************************************************
// 白鍵テクスチャ座標取得：前面
//******************************************************************************
void MTPianoKeyboardDesign::GetWhiteKeyTexturePosFront(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos0,
		D3DXVECTOR2* pTexPos1,
		D3DXVECTOR2* pTexPos2,
		D3DXVECTOR2* pTexPos3
	)
{
	unsigned long index = 0;
	unsigned long x = 0;
	unsigned long y = 1;

	//  0+----+1
	//   |    |
	//  2+----+3

	unsigned long pos[7][4][2] = {
		// 0         1         2         3
		{ {  3, 489}, { 77, 489}, {  3, 561}, { 77, 561} }, // C
		{ { 79, 489}, {154, 489}, { 79, 561}, {154, 561} }, // D
		{ {156, 489}, {230, 489}, {156, 561}, {230, 561} }, // E
		{ {232, 489}, {307, 489}, {232, 561}, {307, 561} }, // F
		{ {309, 489}, {384, 489}, {309, 561}, {384, 561} }, // G
		{ {386, 489}, {460, 489}, {386, 561}, {460, 561} }, // A
		{ {462, 489}, {537, 489}, {462, 561}, {537, 561} }  // B
	};

	switch(GetKeyType(noteNo)) {
		case KeyWhiteC: index = 0; break;
		case KeyWhiteD: index = 1; break;
		case KeyWhiteE: index = 2; break;
		case KeyWhiteF: index = 3; break;
		case KeyWhiteG: index = 4; break;
		case KeyWhiteA: index = 5; break;
		case KeyWhiteB: index = 6; break;
	}

	*pTexPos0 = TEXTURE_POINT(pos[index][0][x], pos[index][0][y]);
	*pTexPos1 = TEXTURE_POINT(pos[index][1][x], pos[index][1][y]);
	*pTexPos2 = TEXTURE_POINT(pos[index][2][x], pos[index][2][y]);
	*pTexPos3 = TEXTURE_POINT(pos[index][3][x], pos[index][3][y]);

	return;
}

//******************************************************************************
// 白鍵テクスチャ座標取得：単一色
//******************************************************************************
void MTPianoKeyboardDesign::GetWhiteKeyTexturePosSingleColor(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos
	)
{
	*pTexPos = TEXTURE_POINT(550, 5);
}

//******************************************************************************
// 黒鍵テクスチャ座標取得：上面＋側面
//******************************************************************************
void MTPianoKeyboardDesign::GetBlackKeyTexturePos(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos0,
		D3DXVECTOR2* pTexPos1,
		D3DXVECTOR2* pTexPos2,
		D3DXVECTOR2* pTexPos3,
		D3DXVECTOR2* pTexPos4,
		D3DXVECTOR2* pTexPos5,
		D3DXVECTOR2* pTexPos6,
		D3DXVECTOR2* pTexPos7,
		D3DXVECTOR2* pTexPos8,
		D3DXVECTOR2* pTexPos9,
		bool isColored
	)
{
	unsigned long index = 0;
	unsigned long x = 0;
	unsigned long y = 1;

	// 9+--+ 5+-+4 +--+7
	//  |  |  | |  |  |
	//  |  |  | |  |  |
	//  |  + 3+-+2 +  |
	//  | /   | |  \  |
	// 8+-+  0+-+1  +-+6

	unsigned long pos[2][10][2] = {
		// 0              1              2              3              4              5              6              7              8              9
		{ { 63- 7, 324}, { 92- 7, 324}, { 92- 7, 305}, { 63- 7, 305}, { 92- 7,   3}, { 63- 7,   3}, { 97- 7, 324}, { 97- 7,   3}, { 58- 7, 324}, { 58- 7,   3} }, // 通常
		{ {447+11, 324}, {476+11, 324}, {476+11, 305}, {447+11, 305}, {476+11,   3}, {447+11,   3}, {481+11, 324}, {481+11,   3}, {442+11, 324}, {442+11,   3} }  // 白色化
	};

	//黒鍵ポリゴンに色を付ける場合は
	//白色化したテクスチャを貼り付ける
	if (isColored) {
		index = 1;
	}

	*pTexPos0 = TEXTURE_POINT(pos[index][0][x], pos[index][0][y]);
	*pTexPos1 = TEXTURE_POINT(pos[index][1][x], pos[index][1][y]);
	*pTexPos2 = TEXTURE_POINT(pos[index][2][x], pos[index][2][y]);
	*pTexPos3 = TEXTURE_POINT(pos[index][3][x], pos[index][3][y]);
	*pTexPos4 = TEXTURE_POINT(pos[index][4][x], pos[index][4][y]);
	*pTexPos5 = TEXTURE_POINT(pos[index][5][x], pos[index][5][y]);
	*pTexPos6 = TEXTURE_POINT(pos[index][6][x], pos[index][6][y]);
	*pTexPos7 = TEXTURE_POINT(pos[index][7][x], pos[index][7][y]);
	*pTexPos8 = TEXTURE_POINT(pos[index][8][x], pos[index][8][y]);
	*pTexPos9 = TEXTURE_POINT(pos[index][9][x], pos[index][9][y]);

	return;
}

//******************************************************************************
// 黒鍵テクスチャ座標取得：単一色
//******************************************************************************
void MTPianoKeyboardDesign::GetBlackKeyTexturePosSingleColor(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos,
		bool isColored
	)
{
	if (isColored) {
		*pTexPos = TEXTURE_POINT(550, 5);
	}
	else {
		*pTexPos = TEXTURE_POINT(550, 15);
	}

	return;
}

//******************************************************************************
// キーボード基準座標取得
//******************************************************************************
D3DXVECTOR3 MTPianoKeyboardDesign::GetKeyboardBasePos(
		unsigned char portNo,
		unsigned char chNo
	)
{
	float ox, oy, oz = 0.0f;
	D3DXVECTOR3 moveVector;

	//ポート単位の原点座標
	ox = GetPortOriginX(portNo);
	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);

	//チャンネルを考慮した配置座標
	moveVector.x = ox + 0.0f;
	moveVector.y = oy + ((float)chNo * m_KeyboardStepY);
	moveVector.z = oz + ((float)chNo * m_KeyboardStepZ);

	return moveVector;
}

//******************************************************************************
// キーボード表示数取得
//******************************************************************************
unsigned long MTPianoKeyboardDesign::GetKeyboardMaxDispNum()
{
	return (unsigned long)m_KeyboardMaxDispNum;
}

// >>> add 20180404 yossiepon begin
//******************************************************************************
// キーボード表示数設定
//******************************************************************************
void MTPianoKeyboardDesign::SetKeyboardSingle()
{
	m_KeyboardMaxDispNum = 1;
}
// <<< add 20180404 yossiepon end

//******************************************************************************
// キー表示範囲：開始
//******************************************************************************
unsigned char MTPianoKeyboardDesign::GetKeyDispRangeStart()
{
	return (unsigned char)m_KeyDispRangeStart;
}

//******************************************************************************
// キー表示範囲：終了
//******************************************************************************
unsigned char MTPianoKeyboardDesign::GetKeyDispRangeEnd()
{
	return (unsigned char)m_KeyDispRangeEnd;
}

//******************************************************************************
// キー表示判定
//******************************************************************************
bool MTPianoKeyboardDesign::IsKeyDisp(
		unsigned char noteNo
	)
{
	bool isDisp = false;

	if ((m_KeyDispRangeStart <= noteNo) && (noteNo <= m_KeyDispRangeEnd)) {
		isDisp = true;
	}

	return isDisp;
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTPianoKeyboardDesign::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR hexColor[16] = {_T('\0')};
	TCHAR activeKeyColorType[32] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//ピアノキーボード情報
	//----------------------------------
	result = confFile.SetCurSection(_T("PianoKeyboard"));
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyDownDuration"), &m_KeyDownDuration, 40);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyUpDuration"), &m_KeyUpDuration, 40);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("KeyboardStepY"), &m_KeyboardStepY, 0.34f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("KeyboardStepZ"), &m_KeyboardStepZ, 1.50f);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyboardMaxDispNum"), &m_KeyboardMaxDispNum, 16);
	if (result != 0) goto EXIT;

	result = confFile.GetStr(_T("WhiteKeyColor"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	m_WhiteKeyColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	result = confFile.GetStr(_T("BlackKeyColor"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	m_BlackKeyColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	result = confFile.GetStr(_T("ActiveKeyColorType"), activeKeyColorType, 32, _T("STANDARD"));
	if (result != 0) goto EXIT;
	if (_tcscmp(activeKeyColorType, _T("NOTE")) == 0) {
		m_ActiveKeyColorType = NoteColor;
	}
	else {
		m_ActiveKeyColorType = DefaultColor;
	}
	result = confFile.GetStr(_T("ActiveKeyColor"), hexColor, 16, _T("FF0000FF"));
	if (result != 0) goto EXIT;
	m_ActiveKeyColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	result = confFile.GetInt(_T("ActiveKeyColorDuration"), &m_ActiveKeyColorDuration, 400);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("ActiveKeyColorTailRate"), &m_ActiveKeyColorTailRate, 0.5f);
	if (result != 0) goto EXIT;

	result = confFile.GetInt(_T("KeyDispRangeStart"), &m_KeyDispRangeStart, 0);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyDispRangeEnd"), &m_KeyDispRangeEnd, 127);
	if (result != 0) goto EXIT;

	//キーボード最大表示数は1ポート分（16ch）に制限する
	if (m_KeyboardMaxDispNum > SM_MAX_CH_NUM) {
		m_KeyboardMaxDispNum = SM_MAX_CH_NUM;
	}
	if (m_KeyboardMaxDispNum < 0) {
		m_KeyboardMaxDispNum = 0;
	}

	//キー表示範囲のクリッピング
	if (m_KeyDispRangeStart < 0) {
		m_KeyDispRangeStart = 0;
	}
	if (m_KeyDispRangeStart > 127) {
		m_KeyDispRangeStart = 127;
	}
	if (m_KeyDispRangeEnd < 0) {
		m_KeyDispRangeEnd = 0;
	}
	if (m_KeyDispRangeEnd > 127) {
		m_KeyDispRangeEnd = 127;
	}
	if (m_KeyDispRangeStart > m_KeyDispRangeEnd) {
		m_KeyDispRangeEnd = m_KeyDispRangeStart;
	}

EXIT:;
	return result;
}


