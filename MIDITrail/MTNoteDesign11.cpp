//******************************************************************************
//
// MIDITrail / MTNoteDesign11
//
// Note design class for DX11.
// Unified from MTNoteDesign + MTNoteDesignMod (ADR-0054 Mod standardization).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTNoteDesign11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTNoteDesign11::MTNoteDesign11()
{
	_Clear();
}

MTNoteDesign11::~MTNoteDesign11()
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTNoteDesign11::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long portIndex = 0;
	unsigned char portNo = 0;

	if (pSeqData == NULL) {
		m_TimeDivision = 48;
		m_PortList.Clear();
		m_PortList.AddPort(0);
	}
	else {
		m_TimeDivision = pSeqData->GetTimeDivision();
		if (m_TimeDivision == 0) {
			result = YN_SET_ERR("Invalid data found.", 0, 0);
			goto EXIT;
		}
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
	}

	for (index = 0; index < 256; index++) {
		m_PortIndex[index] = 0;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = (unsigned char)portIndex;
		portIndex++;
	}

	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = _LoadUserConf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Playback position
//******************************************************************************
float MTNoteDesign11::GetPlayPosX(unsigned long curTickTime)
{
	return ((float)curTickTime * m_QuarterNoteLength / (float)m_TimeDivision);
}

//******************************************************************************
// Note box center position
//******************************************************************************
Vector3 MTNoteDesign11::GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	float pb = GetPitchBendShift(pitchBendValue, pitchBendSensitivity);

	Vector3 v;
	v.x = GetPlayPosX(curTickTime);
	v.y = GetPortOriginY(portNo) + (m_NoteStep * noteNo + pb);
	v.z = GetPortOriginZ(portNo) + (GetChStep() * chNo);

	return v;
}

//******************************************************************************
// Note box dimensions
//******************************************************************************
float MTNoteDesign11::GetNoteBoxHeight()        { return m_NoteBoxHeight; }
float MTNoteDesign11::GetNoteBoxWidth()         { return m_NoteBoxWidth; }
float MTNoteDesign11::GetNoteStep()             { return m_NoteStep; }
float MTNoteDesign11::GetChStep()               { return m_ChStep; }
float MTNoteDesign11::GetActiveNoteWhiteRate()  { return m_ActiveNoteWhiteRate; }
float MTNoteDesign11::GetActiveNoteBoxSizeRatio() { return m_ActiveNoteBoxSizeRatio; }

//******************************************************************************
// Pitch bend Y shift in note-space coordinates
//******************************************************************************
float MTNoteDesign11::GetPitchBendShift(
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	if (pitchBendValue == 0) return 0.0f;

	if (pitchBendValue < 0) {
		return m_NoteStep * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		return m_NoteStep * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}
}

//******************************************************************************
// Note box vertex positions
//******************************************************************************
void MTNoteDesign11::GetNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	Vector3 center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo,
	                                        pitchBendValue, pitchBendSensitivity);
	float bh = GetNoteBoxHeight();
	float bw = GetNoteBoxWidth();

	*pVector0 = Vector3(center.x, center.y + bh / 2.0f, center.z + bw / 2.0f);
	*pVector1 = Vector3(center.x, center.y + bh / 2.0f, center.z - bw / 2.0f);
	*pVector2 = Vector3(center.x, center.y - bh / 2.0f, center.z + bw / 2.0f);
	*pVector3 = Vector3(center.x, center.y - bh / 2.0f, center.z - bw / 2.0f);
}

//******************************************************************************
// Active note box vertex positions (rate-based decay)
//******************************************************************************
void MTNoteDesign11::GetActiveNoteBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3,
		short pitchBendValue,
		unsigned char pitchBendSensitivity,
		float rate
	)
{
	Vector3 center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo,
	                                        pitchBendValue, pitchBendSensitivity);
	float curSizeRatio = 1.0f;
	if (rate > 0.0f) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f) * GetDecayCoefficient(rate, MTNOTEDESIGN_DECAY_SATURATION_SMOOTH);
	}

	float bh = GetNoteBoxHeight() * curSizeRatio;
	float bw = GetNoteBoxWidth() * curSizeRatio;

	*pVector0 = Vector3(center.x, center.y + bh / 2.0f, center.z + bw / 2.0f);
	*pVector1 = Vector3(center.x, center.y + bh / 2.0f, center.z - bw / 2.0f);
	*pVector2 = Vector3(center.x, center.y - bh / 2.0f, center.z + bw / 2.0f);
	*pVector3 = Vector3(center.x, center.y - bh / 2.0f, center.z - bw / 2.0f);
}

//******************************************************************************
// Grid box vertex positions
//******************************************************************************
void MTNoteDesign11::GetGridBoxVirtexPos(
		unsigned long curTickTime,
		unsigned char portNo,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3
	)
{
	float x = GetPlayPosX(curTickTime);
	float bh = GetNoteBoxHeight();
	float bw = GetNoteBoxWidth();
	float gridHeight = GetNoteStep() * 127;
	float gridWidth  = GetChStep() * 15;
	float oy = GetPortOriginY(portNo);
	float oz = GetPortOriginZ(portNo);

	*pVector0 = Vector3(x, oy + gridHeight + bh / 2.0f, oz + gridWidth + bw / 2.0f);
	*pVector1 = Vector3(x, oy + gridHeight + bh / 2.0f, oz             - bw / 2.0f);
	*pVector2 = Vector3(x, oy              - bh / 2.0f, oz + gridWidth + bw / 2.0f);
	*pVector3 = Vector3(x, oy              - bh / 2.0f, oz             - bw / 2.0f);
}

//******************************************************************************
// Playback section vertex positions
//******************************************************************************
void MTNoteDesign11::GetPlaybackSectionVirtexPos(
		unsigned long curTickTime,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3
	)
{
	Vector3 firstPort[4];
	Vector3 lastPort[4];
	unsigned char lastPortNo = 0;

	m_PortList.GetPort(m_PortList.GetSize() - 1, &lastPortNo);

	GetGridBoxVirtexPos(curTickTime, 0, &firstPort[0], &firstPort[1],
	                    &firstPort[2], &firstPort[3]);
	GetGridBoxVirtexPos(curTickTime, lastPortNo, &lastPort[0], &lastPort[1],
	                    &lastPort[2], &lastPort[3]);

	*pVector0 = lastPort[0];
	*pVector1 = firstPort[1];
	*pVector2 = lastPort[2];
	*pVector3 = firstPort[3];
}

//******************************************************************************
// Ripple size (rate-based decay)
//******************************************************************************
float MTNoteDesign11::GetRippleHeight(float rate)
{
	return m_RippleHeight * GetDecayCoefficient(rate);
}

float MTNoteDesign11::GetRippleWidth(float rate)
{
	return m_RippleWidth * GetDecayCoefficient(rate);
}

float MTNoteDesign11::GetRippleAlpha(float rate)
{
	return GetDecayCoefficient(rate);
}

float MTNoteDesign11::GetDecayCoefficient(float rate, float saturation)
{
	float coeff = 1.0f;

	if (rate < 0.5f) {
		coeff = (powf(2.0f, (0.5f - rate) * 8.0f) + 14.0f) / saturation;
	}
	else {
		coeff = (16.0f - powf(2.0f, (rate - 0.5f) * 8.0f)) / saturation;
	}

	if (coeff > 1.0f) coeff = 1.0f;

	return coeff;
}

//******************************************************************************
// Ripple timing
//******************************************************************************
unsigned long MTNoteDesign11::GetRippleDecayDuration()
{
	return (unsigned long)m_RippleDecayDuration;
}

unsigned long MTNoteDesign11::GetRippleReleaseDuration()
{
	return (unsigned long)m_RippleReleaseDuration;
}

D3D11_BLEND MTNoteDesign11::GetRippleSrcBlend()
{
	return m_RippleSrcBlend;
}

D3D11_BLEND MTNoteDesign11::GetRippleDestBlend()
{
	return m_RippleDestBlend;
}

unsigned long MTNoteDesign11::GetRippleOverwriteTimes()
{
	return (unsigned long)m_RippleOverwriteTimes;
}

float MTNoteDesign11::GetRippleSpacing()
{
	return m_RippleSpacing;
}

//******************************************************************************
// Envelope configuration for GPU shader
//******************************************************************************
MTEnvelopeConfig MTNoteDesign11::GetEnvelopeConfig()
{
	MTEnvelopeConfig config;
	config.decayDurationMs = (float)m_RippleDecayDuration;
	config.releaseDurationMs = (float)m_RippleReleaseDuration;
	config.decayRatio = 0.3f;
	config.sustainRatio = 0.4f;
	return config;
}

//******************************************************************************
// Note envelope (3-phase: Decay/Sustain/Release)
//******************************************************************************
MTNoteEnvelopeResult MTNoteDesign11::CalcNoteEnvelope(
		unsigned long playTimeMSec,
		unsigned long startTime,
		unsigned long endTime
	)
{
	MTNoteEnvelopeResult result = { 0.0f, BeforeNoteON };

	if (playTimeMSec > endTime) {
		return result;
	}

	unsigned long decayDuration = (unsigned long)m_RippleDecayDuration;
	unsigned long releaseDuration = (unsigned long)m_RippleReleaseDuration;
	unsigned long noteLen = endTime - startTime;

	MTEnvelopeConfig envConfig = GetEnvelopeConfig();
	float decayRatio = envConfig.decayRatio;
	float sustainRatio = envConfig.sustainRatio;
	float releaseRatio = 1.0f - decayRatio - sustainRatio;

	if (noteLen < decayDuration) {
		// Case A: no adjustment (whole note stays in Decay)
	}
	else if (noteLen < (decayDuration + releaseDuration)) {
		// Case B: eliminate Sustain, shrink Release to fit
		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}
	else if (noteLen < (decayDuration + releaseDuration) * 2) {
		// Case C: midpoint split, eliminate Sustain
		unsigned long midTime = (startTime + decayDuration) / 2
		                      + (endTime - releaseDuration) / 2;
		decayDuration = midTime - startTime;
		releaseDuration = endTime - midTime;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}

	// Phase 1: Decay
	if (playTimeMSec < (startTime + decayDuration)) {
		result.keyStatus = BeforeNoteON;
		if (decayDuration == 0) {
			result.keyDownRate = 0.0f;
		}
		else {
			result.keyDownRate = decayRatio * (float)(playTimeMSec - startTime) / (float)decayDuration;
		}
		return result;
	}
	// Phase 2: Sustain
	if (playTimeMSec <= (endTime - releaseDuration)) {
		result.keyStatus = NoteON;
		unsigned long denominator = noteLen - (decayDuration + releaseDuration);
		if (denominator > 0) {
			result.keyDownRate = decayRatio + sustainRatio
				* (float)(playTimeMSec - (startTime + decayDuration)) / (float)denominator;
		}
		else {
			result.keyDownRate = decayRatio + sustainRatio;
		}
		return result;
	}
	// Phase 3: Release
	result.keyStatus = AfterNoteOFF;
	if (releaseDuration == 0) {
		result.keyDownRate = 1.0f;
	}
	else {
		result.keyDownRate = decayRatio + sustainRatio + releaseRatio
			* (float)(playTimeMSec - (endTime - releaseDuration)) / (float)releaseDuration;
	}
	return result;
}

//******************************************************************************
// Picture board relative position
//******************************************************************************
float MTNoteDesign11::GetPictBoardRelativePos()
{
	return m_PictBoardRelativePos;
}

//******************************************************************************
// Port origin coordinates
//******************************************************************************
float MTNoteDesign11::GetPortOriginY(unsigned char portNo)
{
	return (0.0f - (GetNoteStep() * 127.0f / 2.0f));
}

float MTNoteDesign11::GetPortOriginZ(unsigned char portNo)
{
	float pIdx = (float)(m_PortIndex[portNo]);
	float portWidth = GetChStep() * 16.0f;
	return (portWidth * pIdx - portWidth * m_PortList.GetSize() / 2.0f);
}

//******************************************************************************
// World move vector
//******************************************************************************
Vector3 MTNoteDesign11::GetWorldMoveVector()
{
	return Vector3(0.0f, -GetPortOriginY(0), -GetPortOriginZ(0));
}

//******************************************************************************
// Note box color
//******************************************************************************
Color MTNoteDesign11::GetNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	if (m_NoteColorType == Channel) {
		if (chNo >= 16) {
			return Color(1.0f, 1.0f, 1.0f, 1.0f);
		}
		return m_NoteColor[chNo];
	}
	else if (m_NoteColorType == Scale) {
		return m_NoteColorOfScale[(noteNo % 12)];
	}
	return Color(1.0f, 1.0f, 1.0f, 1.0f);
}

//******************************************************************************
// Active note box color (rate-based decay)
//******************************************************************************
Color MTNoteDesign11::GetActiveNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		float rate
	)
{
	float alpha = GetDecayCoefficient(rate, MTNOTEDESIGN_DECAY_SATURATION_SMOOTH);
	Color color = GetNoteBoxColor(portNo, chNo, noteNo);

	float r = color.R() + ((1.0f - color.R()) * alpha * m_ActiveNoteWhiteRate);
	float g = color.G() + ((1.0f - color.G()) * alpha * m_ActiveNoteWhiteRate);
	float b = color.B() + ((1.0f - color.B()) * alpha * m_ActiveNoteWhiteRate);
	float a = color.A();

	return Color(r, g, b, a);
}

Color MTNoteDesign11::GetActiveNoteEmissive()   { return m_ActiveNoteEmissive; }
Color MTNoteDesign11::GetGridLineColor()        { return m_GridLineColor; }
Color MTNoteDesign11::GetPlaybackSectionColor() { return m_PlaybackSectionColor; }

//******************************************************************************
// Clear
//******************************************************************************
void MTNoteDesign11::_Clear()
{
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

	for (unsigned long i = 0; i < 256; i++) {
		m_PortIndex[i] = 0;
	}

	m_NoteColorType = Channel;
	for (unsigned long i = 0; i < 16; i++) {
		m_NoteColor[i] = Color(1.0f, 1.0f, 1.0f, 1.0f);
	}
	for (unsigned long i = 0; i < 12; i++) {
		m_NoteColorOfScale[i] = Color(1.0f, 1.0f, 1.0f, 1.0f);
	}
	m_ActiveNoteEmissive   = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_GridLineColor        = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_PlaybackSectionColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

	m_ActiveNoteWhiteRate = 1.0f;
	m_ActiveNoteBoxSizeRatio = 1.0f;

	m_RippleDecayDuration = 100;
	m_RippleReleaseDuration = 250;
	m_RippleSrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_RippleDestBlend = D3D11_BLEND_ONE;
	m_RippleOverwriteTimes = 3;
	m_RippleSpacing = 0.002f;
}

//******************************************************************************
// Load configuration
//******************************************************************************
int MTNoteDesign11::_LoadConfFile(const TCHAR* pSceneName)
{
	int result = 0;
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	TCHAR noteColorType[16] = {_T('\0')};
	MTConfFile confFile;
	MTColorConf colorConf;
	MTColorPalette colorPalette;
	Color color;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = colorConf.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	// Scale parameters
	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;
	confFile.GetFloat(_T("QuarterNoteLength"), &m_QuarterNoteLength, 1.0f);
	confFile.GetFloat(_T("NoteBoxHeight"), &m_NoteBoxHeight, 0.1f);
	confFile.GetFloat(_T("NoteBoxWidth"), &m_NoteBoxWidth, 0.1f);
	confFile.GetFloat(_T("NoteStep"), &m_NoteStep, 0.1f);
	confFile.GetFloat(_T("ChStep"), &m_ChStep, 0.5f);
	confFile.GetFloat(_T("RippleHeight"), &m_RippleHeight, 1.0f);
	confFile.GetFloat(_T("RippleWidth"), &m_RippleWidth, 1.0f);
	confFile.GetFloat(_T("PictBoardRelativePos"), &m_PictBoardRelativePos, 1.0f);

	// Color parameters
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;

	confFile.GetStr(_T("NoteColorType"), noteColorType, 16, _T("CHANNEL"));

	m_NoteColorType = Channel;
	if (_tcscmp(noteColorType, _T("SCALE")) == 0) {
		m_NoteColorType = Scale;
	}

	// Note colors from palette
	colorConf.GetSelectedColorPalette(&colorPalette);
	for (unsigned long i = 0; i < 16; i++) {
		result = colorPalette.GetChColor(i, &color);
		if (result != 0) goto EXIT;
		m_NoteColor[i] = color;
	}

	// Scale note colors
	for (unsigned long i = 0; i < 12; i++) {
		_stprintf_s(key, 32, _T("Scale-%02d-NoteRGBA"), i + 1);
		confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		m_NoteColorOfScale[i] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	}

	// Grid line color from palette
	colorPalette.GetGridLineColor(&color);
	m_GridLineColor = color;

	// Playback section color
	confFile.GetStr(_T("PlaybackSectionRGBA"), hexColor, 16, _T("AAAAFFFF"));
	m_PlaybackSectionColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	// Active note parameters
	result = confFile.SetCurSection(_T("ActiveNote"));
	if (result != 0) goto EXIT;

	confFile.GetFloat(_T("WhiteRate"), &m_ActiveNoteWhiteRate, 0.9f);

	confFile.GetStr(_T("EmissiveRGBA"), hexColor, 16, _T("1A1A1A1A"));
	m_ActiveNoteEmissive = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	confFile.GetFloat(_T("SizeRatio"), &m_ActiveNoteBoxSizeRatio, 1.4f);

	// Ripple parameters
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	confFile.GetInt(_T("DecayDuration"), &m_RippleDecayDuration, 100);
	confFile.GetInt(_T("ReleaseDuration"), &m_RippleReleaseDuration, 250);

	confFile.GetInt(_T("SrcBlend"), (int*)&m_RippleSrcBlend, D3D11_BLEND_SRC_ALPHA);
	confFile.GetInt(_T("DestBlend"), (int*)&m_RippleDestBlend, D3D11_BLEND_ONE);

	confFile.GetInt(_T("OverwriteTimes"), &m_RippleOverwriteTimes, 3);
	confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);

EXIT:;
	return result;
}

//******************************************************************************
// Load user config
//******************************************************************************
int MTNoteDesign11::_LoadUserConf()
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

	result = confFile.SetCurSection(_T("QuarterNote"));
	result = confFile.GetInt(_T("LengthMagnification"), &lengthMagnification, 100);

	if (lengthMagnification < 0) lengthMagnification = 0;
	if (lengthMagnification > 1000) lengthMagnification = 1000;

	m_QuarterNoteLength = m_QuarterNoteLength * ((float)lengthMagnification / 100.0f);

EXIT:;
	return result;
}
