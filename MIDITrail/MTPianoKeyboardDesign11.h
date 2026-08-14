//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign11
//
// Piano keyboard design class.
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <directxtk/SimpleMath.h>
#include "SMIDILib.h"

class MTNotePitchBend;

using namespace SMIDILib;


//******************************************************************************
// Piano keyboard design class for DX11
//******************************************************************************
class MTPianoKeyboardDesign11
{
public:

	enum KeyType {
		KeyWhiteC,
		KeyWhiteD,
		KeyWhiteE,
		KeyWhiteF,
		KeyWhiteG,
		KeyWhiteA,
		KeyWhiteB,
		KeyBlack
	};

	enum ActiveKeyColorType {
		DefaultColor,
		NoteColor
	};

public:

	MTPianoKeyboardDesign11();
	virtual ~MTPianoKeyboardDesign11();

	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	// Port origin (Rain: portNo-based layout)
	float GetPortOriginX(unsigned char portNo);
	float GetPortOriginY(unsigned char portNo);
	float GetPortOriginZ(unsigned char portNo);

	// Port origin (Roll: keyboardIndex/angle-based layout)
	float GetPortOriginX();
	float GetPortOriginY(int keyboardIndex, bool flip);
	float GetPortOriginZ(int keyboardIndex, bool flip);

	// Key type and position
	KeyType GetKeyType(unsigned char noteNo);
	float GetKeyCenterPosX(unsigned char noteNo);

	// White key dimensions
	float GetWhiteKeyStep();
	float GetWhiteKeyWidth();
	float GetWhiteKeyHeight();
	float GetWhiteKeyLen();

	// Black key dimensions
	float GetBlackKeyWidth();
	float GetBlackKeyHeight();
	float GetBlackKeySlopeLen();
	float GetBlackKeyLen();

	// Key spacing and rotation
	float GetKeySpaceSize();
	float GetKeyRotateAxisXPos();
	float GetKeyRotateAngle();

	// Key press timing
	unsigned long GetKeyDownDuration();
	unsigned long GetKeyUpDuration();

	// Pitch bend
	float GetPitchBendShift(short pitchBendValue, unsigned char pitchBendSensitivity);
	float GetMaxPitchBendShift(MTNotePitchBend* pNotePitchBend, unsigned char portNo,
	                          unsigned short activeChMask = 0xFFFF);

	// Note drop position
	float GetNoteDropPosZ(unsigned char noteNo);

	// Key colors
	DirectX::SimpleMath::Color GetWhiteKeyColor();
	DirectX::SimpleMath::Color GetBlackKeyColor();

	// Active key color (Rain: single default color)
	DirectX::SimpleMath::Color GetActiveKeyColor(
			unsigned char noteNo,
			unsigned long elapsedTime,
			DirectX::SimpleMath::Color* pNoteColor = NULL
		);

	// Active key color (Roll: per-channel color)
	DirectX::SimpleMath::Color GetActiveKeyColor(
			unsigned char chNo,
			unsigned char noteNo,
			unsigned long elapsedTime,
			DirectX::SimpleMath::Color* pNoteColor = NULL
		);

	// White key texture coordinates
	void GetWhiteKeyTexturePosTop(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos0,
			DirectX::SimpleMath::Vector2* pTexPos1,
			DirectX::SimpleMath::Vector2* pTexPos2,
			DirectX::SimpleMath::Vector2* pTexPos3,
			DirectX::SimpleMath::Vector2* pTexPos4,
			DirectX::SimpleMath::Vector2* pTexPos5,
			DirectX::SimpleMath::Vector2* pTexPos6,
			DirectX::SimpleMath::Vector2* pTexPos7
		);
	void GetWhiteKeyTexturePosFront(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos0,
			DirectX::SimpleMath::Vector2* pTexPos1,
			DirectX::SimpleMath::Vector2* pTexPos2,
			DirectX::SimpleMath::Vector2* pTexPos3
		);
	void GetWhiteKeyTexturePosSingleColor(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos
		);

	// Black key texture coordinates
	void GetBlackKeyTexturePos(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos0,
			DirectX::SimpleMath::Vector2* pTexPos1,
			DirectX::SimpleMath::Vector2* pTexPos2,
			DirectX::SimpleMath::Vector2* pTexPos3,
			DirectX::SimpleMath::Vector2* pTexPos4,
			DirectX::SimpleMath::Vector2* pTexPos5,
			DirectX::SimpleMath::Vector2* pTexPos6,
			DirectX::SimpleMath::Vector2* pTexPos7,
			DirectX::SimpleMath::Vector2* pTexPos8,
			DirectX::SimpleMath::Vector2* pTexPos9,
			bool isColored = false
		);
	void GetBlackKeyTexturePosSingleColor(
			unsigned char noteNo,
			DirectX::SimpleMath::Vector2* pTexPos,
			bool isColored = false
		);

	// Keyboard base position (Rain: portNo/chNo-based)
	DirectX::SimpleMath::Vector3 GetKeyboardBasePos(unsigned char portNo, unsigned char chNo);

	// Keyboard base position (Roll: keyboardIndex/angle-based)
	DirectX::SimpleMath::Vector3 GetKeyboardBasePos(int keyboardIndex, float angle);

	// Keyboard display
	unsigned long GetKeyboardMaxDispNum();
	void SetKeyboardSingle();

	// Key display range
	unsigned char GetKeyDispRangeStart();
	unsigned char GetKeyDispRangeEnd();
	bool IsKeyDisp(unsigned char noteNo);

	// Note box dimensions (Roll-specific, from Mod)
	float GetNoteBoxHeight();
	float GetNoteBoxWidth();
	float GetNoteStep();
	float GetChStep();

	// Layout dimensions (Roll-specific, from Mod)
	float GetKeyboardHeight();
	float GetKeyboardWidth();
	float GetGridHeight();
	float GetGridWidth();
	float GetPortHeight();
	float GetPortWidth();
	float GetPlaybackSectionHeight();
	float GetPlaybackSectionWidth();

	// Ripple layout (Roll-specific, from Mod)
	float GetRippleSpacing();
	float GetRippleMargin();
	float GetKeyboardResizeRatio();

private:

	typedef struct {
		KeyType keyType;
		float keyCenterPosX;
	} MTKeyInfo;

	MTKeyInfo m_KeyInfo[SM_MAX_NOTE_NUM];

protected:

	SMPortList m_PortList;
	unsigned char m_PortIndex[SM_MAX_PORT_NUM];

	float m_KeyboardStepY;
	int m_ActiveKeyColorDuration;
	float m_ActiveKeyColorTailRate;
	ActiveKeyColorType m_ActiveKeyColorType;

private:

	// Key dimensions (hardcoded defaults, not from config)
	float m_WhiteKeyStep;
	float m_WhiteKeyWidth;
	float m_WhiteKeyHeight;
	float m_WhiteKeyLen;
	float m_BlackKeyWidth;
	float m_BlackKeyHeight;
	float m_BlackKeySlopeLen;
	float m_BlackKeyLen;
	float m_KeySpaceSize;
	float m_NoteDropPosZ4WhiteKey;
	float m_NoteDropPosZ4BlackKey;
	float m_BlackKeyShiftCDE;
	float m_BlackKeyShiftFGAB;

	// Key rotation
	float m_KeyRotateAxisXPos;
	float m_KeyRotateAngle;
	int m_KeyDownDuration;
	int m_KeyUpDuration;

	// Keyboard layout
	float m_KeyboardStepZ;
	int m_KeyboardMaxDispNum;

	// Key colors
	DirectX::SimpleMath::Color m_WhiteKeyColor;
	DirectX::SimpleMath::Color m_BlackKeyColor;
	DirectX::SimpleMath::Color m_ActiveKeyColor;
	DirectX::SimpleMath::Color m_ActiveKeyColorList[16];

	// Key display range
	int m_KeyDispRangeStart;
	int m_KeyDispRangeEnd;

	// Roll-specific note/grid dimensions (from Mod)
	float m_NoteBoxHeight;
	float m_NoteBoxWidth;
	float m_NoteStep;
	float m_ChStep;
	float m_RippleSpacing;

	void _Initialize();
	void _InitKeyType();
	void _InitKeyPos();

	virtual int _LoadConfFile(const TCHAR* pSceneName);
};
