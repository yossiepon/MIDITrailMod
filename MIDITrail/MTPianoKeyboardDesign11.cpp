//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign11
//
// Piano keyboard design class.
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardDesign11.h"
#include "MTNotePitchBend.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Parameter definitions
//******************************************************************************
//Texture coordinate calculation: bitmap size = 562 x 562
#define TEXTURE_POINT(x, y)  (Vector2((float)x/561.0f, (float)y/561.0f))

//******************************************************************************
// Constructor
//******************************************************************************
MTPianoKeyboardDesign11::MTPianoKeyboardDesign11(void)
{
	_Initialize();
}

//******************************************************************************
// Destructor
//******************************************************************************
MTPianoKeyboardDesign11::~MTPianoKeyboardDesign11(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTPianoKeyboardDesign11::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long portIndex = 0;
	unsigned char portNo = 0;

	//Settings for live monitor
	if (pSeqData == NULL) {
		//Port list
		m_PortList.Clear();
		m_PortList.AddPort(0);
	}
	//Normal settings
	else {
		//Get port list
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
	}

	//Load config file
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//Assign ascending indices to port numbers
	//E.g., when outputting to ports 0, 3, 5, the indices are 0, 1, 2 respectively
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_PortIndex[index] = 0;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = (unsigned char)portIndex;
		portIndex++;
	}

	//Initialize key types
	_InitKeyType();

	//Set key coordinates
	_InitKeyPos();

EXIT:;
	return result;
}

//******************************************************************************
// Initialize
//******************************************************************************
void MTPianoKeyboardDesign11::_Initialize()
{
	unsigned long i = 0;

	ZeroMemory(&(m_KeyInfo[0]), sizeof(MTKeyInfo) * SM_MAX_NOTE_NUM);

	for (i = 0; i < SM_MAX_PORT_NUM; i++) {
		m_PortIndex[i] = 0;
	}

	//Key polygon coordinates are hardcoded directly,
	//so these parameters are not listed in the config file

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
	m_KeyDownDuration   = 40;         //Config file
	m_KeyUpDuration     = 40;         //Config file
	m_KeyboardStepY     = 0.34f;      //Config file
	m_KeyboardStepZ     = 1.50f;      //Config file
	m_NoteDropPosZ4WhiteKey = 0.25f;
	m_NoteDropPosZ4BlackKey = 0.75f;
	m_BlackKeyShiftCDE  = 0.0216f;    //Texture image: 7 dots' worth
	m_BlackKeyShiftFGAB = 0.0340f;    //Texture image: 11 dots' worth
	m_KeyboardMaxDispNum = 16;        //Config file
	m_WhiteKeyColor =  DXColorUtil::MakeColorFromHexRGBA(_T("FFFFFFFF")); //Config file
	m_BlackKeyColor =  DXColorUtil::MakeColorFromHexRGBA(_T("FFFFFFFF")); //Config file
	m_ActiveKeyColorType = DefaultColor;  //Config file
	m_ActiveKeyColor = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF")); //Config file
	m_ActiveKeyColorDuration = 400;   //Config file
	m_ActiveKeyColorTailRate = 0.5f;  //Config file
	m_KeyDispRangeStart = 0;
	m_KeyDispRangeEnd   = 127;

	// Mod members
	m_NoteBoxHeight = 0.0f;
	m_NoteBoxWidth = 0.0f;
	m_NoteStep = 0.0f;
	m_ChStep = 0.0f;
	m_RippleSpacing = 0.002f;
	for (i = 0; i < 16; i++) {
		m_ActiveKeyColorList[i] = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF"));
	}

	return;
}

//******************************************************************************
// Initialize key types
//******************************************************************************
void MTPianoKeyboardDesign11::_InitKeyType()
{
	unsigned long i = 0;
	unsigned char noteNo = 0;
	KeyType type = KeyWhiteC;

	//On a real keyboard, black keys are slightly offset,
	//so strictly speaking the shapes of (C,F), (D,G,A), (E,B) all differ

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
	m_KeyInfo[noteNo + 7].keyType = KeyWhiteB;		// |________|G <= same shape as B

	//Key display range: adjust start key
	type = m_KeyInfo[m_KeyDispRangeStart].keyType;
	switch (type) {
		case KeyWhiteC: type = KeyWhiteC; break;
		case KeyWhiteD: type = KeyWhiteC; break;
		case KeyWhiteE: type = KeyWhiteE; break; //No change target
		case KeyWhiteF: type = KeyWhiteF; break;
		case KeyWhiteG: type = KeyWhiteF; break;
		case KeyWhiteA: type = KeyWhiteF; break;
		case KeyWhiteB: type = KeyWhiteB; break; //No change target
		default: break;
	}
	m_KeyInfo[m_KeyDispRangeStart].keyType = type;

	//Key display range: adjust end key
	type = m_KeyInfo[m_KeyDispRangeEnd].keyType;
	switch (type) {
		case KeyWhiteC: type = KeyWhiteC; break; //No change target
		case KeyWhiteD: type = KeyWhiteE; break;
		case KeyWhiteE: type = KeyWhiteE; break;
		case KeyWhiteF: type = KeyWhiteF; break; //No change target
		case KeyWhiteG: type = KeyWhiteB; break;
		case KeyWhiteA: type = KeyWhiteB; break;
		case KeyWhiteB: type = KeyWhiteB; break;
		default: break;
	}
	m_KeyInfo[m_KeyDispRangeEnd].keyType = type;

	return;
}

//******************************************************************************
// Set key coordinates
//******************************************************************************
void MTPianoKeyboardDesign11::_InitKeyPos()
{
	unsigned char noteNo = 0;
	KeyType prevKeyType = KeyWhiteB;
	float posX = 0.0f;
	float shift = 0.0f;

	//Position of the first note
	//posX = GetWhiteKeyStep() / 2.0f;
	m_KeyInfo[noteNo].keyCenterPosX = posX;
	prevKeyType = m_KeyInfo[noteNo].keyType;

	//Black keys are slightly offset on a real keyboard,
	//so first place the black key at the midpoint between white keys, then correct it

	//Position of the second and subsequent notes
	for (noteNo = 1; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		//Previous key is black
		if (prevKeyType == KeyBlack) {
			if (m_KeyInfo[noteNo].keyType == KeyBlack) {
				//A black key cannot immediately follow another black key
			}
			else {
				//Place the black key at the center between the white keys
				//Differs from a real keyboard, but overlooked to save effort
				posX += (GetWhiteKeyStep() / 2.0f);
			}
		}
		//Previous key is white
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

	//Correct the black key placement
	prevKeyType = KeyWhiteC;
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		if (m_KeyInfo[noteNo].keyType == KeyBlack) {
			//Get the black key position correction amount
			switch (prevKeyType) {
				case KeyWhiteC: shift = -m_BlackKeyShiftCDE;  break;
				case KeyWhiteD: shift = +m_BlackKeyShiftCDE;  break;
				case KeyWhiteF: shift = -m_BlackKeyShiftFGAB; break;
				case KeyWhiteG: shift =  0.00f;               break;
				case KeyWhiteA: shift = +m_BlackKeyShiftFGAB; break;
				default:        shift =  0.00f;               break;
			}
			//Place the last black key at the midpoint
			if (noteNo == 126) {
				shift = 0.00f;
			}
			
			//A black key left isolated at the start or end of the display range is centered
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
			
			//Position correction
			m_KeyInfo[noteNo].keyCenterPosX += shift;
		}
		prevKeyType = m_KeyInfo[noteNo].keyType;
	}

	return;
}

//******************************************************************************
// Get port origin X coordinate
//******************************************************************************
float MTPianoKeyboardDesign11::GetPortOriginX(
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
// Get port origin Y coordinate
//******************************************************************************
float MTPianoKeyboardDesign11::GetPortOriginY(
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

	//Number of displayed channels
	chNum = m_PortList.GetSize() * SM_MAX_CH_NUM;
	if ((unsigned long)m_KeyboardMaxDispNum < chNum) {
		chNum = m_KeyboardMaxDispNum;
	}

	totalHeight = portHeight * ((float)chNum / (float)SM_MAX_CH_NUM);
	originY = (portHeight * portIndex) - (totalHeight / 2.0f);

	return originY;
}

//******************************************************************************
// Get port origin Z coordinate
//******************************************************************************
float MTPianoKeyboardDesign11::GetPortOriginZ(
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

	//Number of displayed channels
	chNum = m_PortList.GetSize() * SM_MAX_CH_NUM;
	if ((unsigned long)m_KeyboardMaxDispNum < chNum) {
		chNum = m_KeyboardMaxDispNum;
	}

	totalLen = portLen * ((float)chNum / (float)SM_MAX_CH_NUM);
	originZ = (portLen * portIndex) - (totalLen / 2.0f);

	return originZ;
}

//******************************************************************************
// Get key type
//******************************************************************************
MTPianoKeyboardDesign11::KeyType MTPianoKeyboardDesign11::GetKeyType(
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
// Get key center X coordinate
//******************************************************************************
float MTPianoKeyboardDesign11::GetKeyCenterPosX(
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
// Get white key placement interval
//******************************************************************************
float MTPianoKeyboardDesign11::GetWhiteKeyStep()
{
	return m_WhiteKeyStep;
}

//******************************************************************************
// Get white key width
//******************************************************************************
float MTPianoKeyboardDesign11::GetWhiteKeyWidth()
{
	return m_WhiteKeyWidth;
}

//******************************************************************************
// Get white key height
//******************************************************************************
float MTPianoKeyboardDesign11::GetWhiteKeyHeight()
{
	return m_WhiteKeyHeight;
}

//******************************************************************************
// Get white key length
//******************************************************************************
float MTPianoKeyboardDesign11::GetWhiteKeyLen()
{
	return m_WhiteKeyLen;
}

//******************************************************************************
// Get black key width
//******************************************************************************
float MTPianoKeyboardDesign11::GetBlackKeyWidth()
{
	return m_BlackKeyWidth;
}

//******************************************************************************
// Get black key height
//******************************************************************************
float MTPianoKeyboardDesign11::GetBlackKeyHeight()
{
	return m_BlackKeyHeight;
}

//******************************************************************************
// Get black key slope length
//******************************************************************************
float MTPianoKeyboardDesign11::GetBlackKeySlopeLen()
{
	return m_BlackKeySlopeLen;
}

//******************************************************************************
// Get black key length
//******************************************************************************
float MTPianoKeyboardDesign11::GetBlackKeyLen()
{
	return m_BlackKeyLen;
}

//******************************************************************************
// Get key spacing size
//******************************************************************************
float MTPianoKeyboardDesign11::GetKeySpaceSize()
{
	return m_KeySpaceSize;
}

//******************************************************************************
// Get key-press rotation center Y coordinate
//******************************************************************************
float MTPianoKeyboardDesign11::GetKeyRotateAxisXPos()
{
	return m_KeyRotateAxisXPos;
}

//******************************************************************************
// Key-press rotation angle
//******************************************************************************
float MTPianoKeyboardDesign11::GetKeyRotateAngle()
{
	return m_KeyRotateAngle;
}

//******************************************************************************
// Get key-down duration (msec)
//******************************************************************************
unsigned long MTPianoKeyboardDesign11::GetKeyDownDuration()
{
	return (unsigned long)m_KeyDownDuration;
}

//******************************************************************************
// Get key-up duration (msec)
//******************************************************************************
unsigned long MTPianoKeyboardDesign11::GetKeyUpDuration()
{
	return (unsigned long)m_KeyUpDuration;
}

//******************************************************************************
// Get note drop coordinate
//******************************************************************************
float MTPianoKeyboardDesign11::GetNoteDropPosZ(
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
// Get pitch bend keyboard shift amount
//******************************************************************************
float MTPianoKeyboardDesign11::GetPitchBendShift(
		short pitchBendValue,				//Pitch bend
		unsigned char pitchBendSensitivity	//Pitch bend sensitivity
	)
{
	float shift = 0.0f;
	float noteStep = 0.0f;

	//Semitone shift amount
	//  Key spacing is not uniform because there is no black key between B->C or E->F
	//  Determine the semitone shift amount so it balances out over one octave
	noteStep = GetWhiteKeyStep() * 7.0f / 12.0f;

	//Keyboard shift amount due to pitch bend
	if (pitchBendValue < 0) {
		shift = noteStep * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		shift = noteStep * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	return shift;
}

float MTPianoKeyboardDesign11::GetMaxPitchBendShift(
		MTNotePitchBend* pNotePitchBend,
		unsigned char portNo,
		unsigned short activeChMask
	)
{
	if (pNotePitchBend == NULL) return 0.0f;

	float maxShift = 0.0f;
	for (unsigned char chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		if (!(activeChMask & (1 << chNo))) continue;
		short v = pNotePitchBend->GetValue(portNo, chNo);
		unsigned char s = pNotePitchBend->GetSensitivity(portNo, chNo);
		float shift = GetPitchBendShift(v, s);
		if (fabsf(shift) > fabsf(maxShift)) maxShift = shift;
	}
	return maxShift;
}

//******************************************************************************
// Get white key color
//******************************************************************************
Color MTPianoKeyboardDesign11::GetWhiteKeyColor()
{
	return m_WhiteKeyColor;
}

//******************************************************************************
// Get black key color
//******************************************************************************
Color MTPianoKeyboardDesign11::GetBlackKeyColor()
{
	return m_BlackKeyColor;
}

//******************************************************************************
// Get active (sounding) key color
//******************************************************************************
Color MTPianoKeyboardDesign11::GetActiveKeyColor(
		unsigned char noteNo,
		unsigned long elapsedTime,
		Color* pNoteColor
	)
{
	Color color;
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 0.0f;
	float rate = 0.0f;
	unsigned long duration = 0;

	//          on     off
	//   White |---+......+---- <- returns to the white key color when off
	//      |   :      :
	//      |   :  +---+     <- stays the intermediate color until off
	//      |   : /:   :
	//      |   :/ :   :
	//   Red  |   +  :   :     <- color immediately after key press (red)
	//      |   :\ :   :
	//      |   : \:   :
	//      |   :  +---+     <- stays the intermediate color until off
	//      |   :  :   :
	//   Black |---+  :   +---- <- returns to the black key color when off
	//   ---+---*------*-------> +t
	//      |   on :   off
	//          <-->duration

	if ((pNoteColor != NULL) && (m_ActiveKeyColorType == NoteColor)) {
		//When a note color is specified
		color = *pNoteColor;
	}
	else {
		//Otherwise use the default color
		color = m_ActiveKeyColor;
	}

	duration = (unsigned long)m_ActiveKeyColorDuration;
	rate     = m_ActiveKeyColorTailRate;

	if (elapsedTime < duration) {
		rate = ((float)elapsedTime / (float)duration) * m_ActiveKeyColorTailRate;
	}

	if (GetKeyType(noteNo) == KeyBlack) {
		r = color.R() - ((color.R()) * rate);
		g = color.G() - ((color.G()) * rate);
		b = color.B() - ((color.B()) * rate);
		a = color.A();
	}
	else {
		r = color.R() + ((1.0f - color.R()) * rate);
		g = color.G() + ((1.0f - color.G()) * rate);
		b = color.B() + ((1.0f - color.B()) * rate);
		a = color.A();
	}
	color = Color(r, g, b, a);

	return color;
}

//******************************************************************************
// Get white key texture coordinates: top face
//******************************************************************************
void MTPianoKeyboardDesign11::GetWhiteKeyTexturePosTop(
		unsigned char noteNo,
		Vector2* pTexPos0,
		Vector2* pTexPos1,
		Vector2* pTexPos2,
		Vector2* pTexPos3,
		Vector2* pTexPos4,
		Vector2* pTexPos5,
		Vector2* pTexPos6,
		Vector2* pTexPos7
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
// Get white key texture coordinates: front face
//******************************************************************************
void MTPianoKeyboardDesign11::GetWhiteKeyTexturePosFront(
		unsigned char noteNo,
		Vector2* pTexPos0,
		Vector2* pTexPos1,
		Vector2* pTexPos2,
		Vector2* pTexPos3
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
// Get white key texture coordinates: solid color
//******************************************************************************
void MTPianoKeyboardDesign11::GetWhiteKeyTexturePosSingleColor(
		unsigned char noteNo,
		Vector2* pTexPos
	)
{
	*pTexPos = TEXTURE_POINT(550, 5);
}

//******************************************************************************
// Get black key texture coordinates: top + side faces
//******************************************************************************
void MTPianoKeyboardDesign11::GetBlackKeyTexturePos(
		unsigned char noteNo,
		Vector2* pTexPos0,
		Vector2* pTexPos1,
		Vector2* pTexPos2,
		Vector2* pTexPos3,
		Vector2* pTexPos4,
		Vector2* pTexPos5,
		Vector2* pTexPos6,
		Vector2* pTexPos7,
		Vector2* pTexPos8,
		Vector2* pTexPos9,
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
		{ { 63- 7, 324}, { 92- 7, 324}, { 92- 7, 305}, { 63- 7, 305}, { 92- 7,   3}, { 63- 7,   3}, { 97- 7, 324}, { 97- 7,   3}, { 58- 7, 324}, { 58- 7,   3} }, // Normal
		{ {447+11, 324}, {476+11, 324}, {476+11, 305}, {447+11, 305}, {476+11,   3}, {447+11,   3}, {481+11, 324}, {481+11,   3}, {442+11, 324}, {442+11,   3} }  // Whitened
	};

	//When coloring a black key polygon,
	//use the whitened texture
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
// Get black key texture coordinates: solid color
//******************************************************************************
void MTPianoKeyboardDesign11::GetBlackKeyTexturePosSingleColor(
		unsigned char noteNo,
		Vector2* pTexPos,
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
// Get keyboard reference coordinate
//******************************************************************************
Vector3 MTPianoKeyboardDesign11::GetKeyboardBasePos(
		unsigned char portNo,
		unsigned char chNo
	)
{
	float ox = 0.0f;
	float oy = 0.0f;
	float oz = 0.0f;
	Vector3 moveVector;

	//Per-port origin coordinate
	ox = GetPortOriginX(portNo);
	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);

	//Placement coordinate accounting for channel
	moveVector.x = ox + 0.0f;
	moveVector.y = oy + ((float)chNo * m_KeyboardStepY);
	moveVector.z = oz + ((float)chNo * m_KeyboardStepZ);

	return moveVector;
}

//******************************************************************************
// Get keyboard display count
//******************************************************************************
unsigned long MTPianoKeyboardDesign11::GetKeyboardMaxDispNum()
{
	return (unsigned long)m_KeyboardMaxDispNum;
}

//******************************************************************************
// Set keyboard display count
//******************************************************************************
void MTPianoKeyboardDesign11::SetKeyboardSingle()
{
	m_KeyboardMaxDispNum = 1;
}

//******************************************************************************
// Key display range: start
//******************************************************************************
unsigned char MTPianoKeyboardDesign11::GetKeyDispRangeStart()
{
	return (unsigned char)m_KeyDispRangeStart;
}

//******************************************************************************
// Key display range: end
//******************************************************************************
unsigned char MTPianoKeyboardDesign11::GetKeyDispRangeEnd()
{
	return (unsigned char)m_KeyDispRangeEnd;
}

//******************************************************************************
// Key display check
//******************************************************************************
bool MTPianoKeyboardDesign11::IsKeyDisp(
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
// Load config file
//******************************************************************************
int MTPianoKeyboardDesign11::_LoadConfFile(
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
	//Piano keyboard information
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

	//Limit the max keyboard display count to one port's worth (16 ch)
	if (m_KeyboardMaxDispNum > SM_MAX_CH_NUM) {
		m_KeyboardMaxDispNum = SM_MAX_CH_NUM;
	}
	if (m_KeyboardMaxDispNum < 0) {
		m_KeyboardMaxDispNum = 0;
	}

	//Clip the key display range
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

	//----------------------------------
	// Mod: Scale parameters (Roll-specific note/grid dimensions)
	//----------------------------------
	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;

	result = confFile.GetFloat(_T("NoteBoxHeight"), &m_NoteBoxHeight, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteBoxWidth"), &m_NoteBoxWidth, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteStep"), &m_NoteStep, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("ChStep"), &m_ChStep, 0.5f);
	if (result != 0) goto EXIT;

	//----------------------------------
	// Mod: Per-channel active key colors
	//----------------------------------
	{
		TCHAR chKey[21] = {_T('\0')};
		result = confFile.SetCurSection(_T("PianoKeyboard"));
		if (result != 0) goto EXIT;

		for (unsigned long ci = 0; ci < 16; ci++) {
			_stprintf_s(chKey, 21, _T("Ch-%02d-ActiveKeyColor"), ci+1);
			result = confFile.GetStr(chKey, hexColor, 16, _T("FF0000FF"));
			if (result != 0) goto EXIT;
			m_ActiveKeyColorList[ci] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
		}
	}

	//----------------------------------
	// Mod: Ripple spacing
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	result = confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Keyboard base position (Roll: keyboardIndex/angle-based)
//******************************************************************************
Vector3 MTPianoKeyboardDesign11::GetKeyboardBasePos(
		int keyboardIndex,
		float angle
	)
{
	angle += angle < 0.0f ? 360.0f : 0.0f;
	bool flip = !((angle > 120.0f) && (angle < 300.0f));

	float ox = GetPortOriginX();
	float oy = GetPortOriginY(keyboardIndex, flip);
	float oz = GetPortOriginZ(keyboardIndex, flip);

	return Vector3(ox, oy, oz);
}

//******************************************************************************
// Port origin X (Roll: playback section center)
//******************************************************************************
float MTPianoKeyboardDesign11::GetPortOriginX()
{
	float originX = -GetPlaybackSectionHeight() / 2.0f;
	originX += GetWhiteKeyStep() * GetKeyboardResizeRatio() / 2.0f;
	return originX;
}

//******************************************************************************
// Port origin Y (Roll: keyboardIndex/flip-based)
//******************************************************************************
float MTPianoKeyboardDesign11::GetPortOriginY(
		int keyboardIndex,
		bool flip
	)
{
	float portWidth = GetPortWidth();

	int keyboardDispNum = GetKeyboardMaxDispNum() > 1 ? m_PortList.GetSize() : 1;

	float originY;

	if (!flip) {
		originY = -portWidth * (float)(keyboardDispNum - keyboardIndex * 2) / 2.0f;
		originY -= GetChStep() * 0.625f;
	}
	else {
		originY = portWidth * (float)(keyboardDispNum - (keyboardIndex + 1) * 2) / 2.0f;
		originY += GetChStep() * 0.375f;
	}

	return originY;
}

//******************************************************************************
// Port origin Z (Roll: keyboardIndex/flip-based)
//******************************************************************************
float MTPianoKeyboardDesign11::GetPortOriginZ(
		int keyboardIndex,
		bool flip
	)
{
	float originZ;

	if (!flip) {
		originZ = -(GetWhiteKeyLen() * GetKeyboardResizeRatio() + GetRippleMargin());
	}
	else {
		originZ = GetRippleMargin();
	}

	return originZ;
}

//******************************************************************************
// Note box dimensions (Roll-specific)
//******************************************************************************
float MTPianoKeyboardDesign11::GetNoteBoxHeight()  { return m_NoteBoxHeight; }
float MTPianoKeyboardDesign11::GetNoteBoxWidth()   { return m_NoteBoxWidth; }
float MTPianoKeyboardDesign11::GetNoteStep()       { return m_NoteStep; }
float MTPianoKeyboardDesign11::GetChStep()         { return m_ChStep; }

//******************************************************************************
// Layout dimensions (Roll-specific)
//******************************************************************************
float MTPianoKeyboardDesign11::GetKeyboardHeight()         { return GetBlackKeyHeight(); }
float MTPianoKeyboardDesign11::GetKeyboardWidth()          { return GetWhiteKeyStep() * (float)(SM_MAX_NOTE_NUM - 53); }
float MTPianoKeyboardDesign11::GetGridHeight()             { return GetNoteStep() * 127.0f; }
float MTPianoKeyboardDesign11::GetGridWidth()              { return GetChStep() * 15.0f; }
float MTPianoKeyboardDesign11::GetPortHeight()             { return GetGridHeight(); }
float MTPianoKeyboardDesign11::GetPortWidth()              { return GetChStep() * 16.0f; }
float MTPianoKeyboardDesign11::GetPlaybackSectionHeight()  { return GetGridHeight() + GetNoteBoxHeight(); }
float MTPianoKeyboardDesign11::GetPlaybackSectionWidth()   { return GetGridWidth() + GetNoteBoxWidth(); }

//******************************************************************************
// Ripple layout (Roll-specific)
//******************************************************************************
float MTPianoKeyboardDesign11::GetRippleSpacing()
{
	return m_RippleSpacing;
}

float MTPianoKeyboardDesign11::GetRippleMargin()
{
	return GetRippleSpacing() * 2;
}

float MTPianoKeyboardDesign11::GetKeyboardResizeRatio()
{
	return GetPlaybackSectionHeight() / GetKeyboardWidth();
}

//******************************************************************************
// Active key color (Roll: per-channel color)
//******************************************************************************
Color MTPianoKeyboardDesign11::GetActiveKeyColor(
		unsigned char chNo,
		unsigned char noteNo,
		unsigned long elapsedTime,
		Color* pNoteColor
	)
{
	Color color;
	float r,g,b,a = 0.0f;
	float rate = 0.0f;
	unsigned long duration = 0;

	if ((pNoteColor != NULL) && (m_ActiveKeyColorType == NoteColor)) {
		color = *pNoteColor;
	}
	else {
		color = m_ActiveKeyColorList[chNo];
	}

	duration = (unsigned long)m_ActiveKeyColorDuration;
	rate     = m_ActiveKeyColorTailRate;

	if (elapsedTime < duration) {
		rate = ((float)elapsedTime / (float)duration) * m_ActiveKeyColorTailRate;
	}

	if (GetKeyType(noteNo) == KeyBlack) {
		r = color.R() - ((color.R()) * rate);
		g = color.G() - ((color.G()) * rate);
		b = color.B() - ((color.B()) * rate);
		a = color.A();
	}
	else {
		r = color.R() + ((1.0f - color.R()) * rate);
		g = color.G() + ((1.0f - color.G()) * rate);
		b = color.B() + ((1.0f - color.B()) * rate);
		a = color.A();
	}
	color = Color(r, g, b, a);

	return color;
}

