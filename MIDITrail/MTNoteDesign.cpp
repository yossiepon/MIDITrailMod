//******************************************************************************
//
// MIDITrail / MTNoteDesign
//
// Note design class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "MTNoteDesign.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTNoteDesign::MTNoteDesign()
{
	_Clear();
}

//******************************************************************************
// Destructor
//******************************************************************************
MTNoteDesign::~MTNoteDesign()
{
}

//******************************************************************************
// Initialize
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
float MTNoteDesign::GetPlayPosX(unsigned long curTickTime)
{
	return ((float)curTickTime * m_QuarterNoteLength / (float)m_TimeDivision);
}

//******************************************************************************
// Live monitor note position
//******************************************************************************
float MTNoteDesign::GetLivePosX(unsigned long elapsedTime)
{
	return (((float)elapsedTime / 1000.0f) * m_LiveNoteLengthPerSecond);
}

//******************************************************************************
// Note box center position
//******************************************************************************
Vector3 MTNoteDesign::GetNoteBoxCenterPosX(
		unsigned long curTickTime,
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		short pitchBendValue,
		unsigned char pitchBendSensitivity
	)
{
	float pb = 0.0f;

	if (pitchBendValue < 0) {
		pb = GetNoteStep() * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		pb = GetNoteStep() * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	Vector3 v;
	v.x = GetPlayPosX(curTickTime);
	v.y = GetPortOriginY(portNo) + (m_NoteStep * noteNo + pb);
	v.z = GetPortOriginZ(portNo) + (GetChStep() * chNo);

	return v;
}

//******************************************************************************
// Note box dimensions
//******************************************************************************
float MTNoteDesign::GetNoteBoxHeight() { return m_NoteBoxHeight; }
float MTNoteDesign::GetNoteBoxWidth()  { return m_NoteBoxWidth; }
float MTNoteDesign::GetNoteStep()      { return m_NoteStep; }
float MTNoteDesign::GetChStep()        { return m_ChStep; }

unsigned long MTNoteDesign::GetLiveMonitorDisplayDuration()
{
	return (unsigned long)m_LiveMonitorDisplayDuration;
}

//******************************************************************************
// Note box vertex positions
//******************************************************************************
void MTNoteDesign::GetNoteBoxVirtexPos(
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
// Active note box vertex positions
//******************************************************************************
void MTNoteDesign::GetActiveNoteBoxVirtexPos(
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
		unsigned long elapsedTime
	)
{
	Vector3 center = GetNoteBoxCenterPosX(curTickTime, portNo, chNo, noteNo,
	                                        pitchBendValue, pitchBendSensitivity);
	float curSizeRatio = 1.0f;
	if (elapsedTime < (unsigned long)m_ActiveNoteDuration) {
		curSizeRatio = 1.0f + (m_ActiveNoteBoxSizeRatio - 1.0f)
		             * (1.0f - (float)elapsedTime / (float)m_ActiveNoteDuration);
	}

	float bh = GetNoteBoxHeight() * curSizeRatio;
	float bw = GetNoteBoxWidth() * curSizeRatio;

	*pVector0 = Vector3(center.x, center.y + bh / 2.0f, center.z + bw / 2.0f);
	*pVector1 = Vector3(center.x, center.y + bh / 2.0f, center.z - bw / 2.0f);
	*pVector2 = Vector3(center.x, center.y - bh / 2.0f, center.z + bw / 2.0f);
	*pVector3 = Vector3(center.x, center.y - bh / 2.0f, center.z - bw / 2.0f);
}

//******************************************************************************
// Live monitor note box vertex positions
//******************************************************************************
void MTNoteDesign::GetNoteBoxVirtexPosLive(
		unsigned long elapsedTime,
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
	unsigned long tickTimeDummy = 0;
	Vector3 center = GetNoteBoxCenterPosX(tickTimeDummy, portNo, chNo, noteNo,
	                                        pitchBendValue, pitchBendSensitivity);
	float x = -(GetLivePosX(elapsedTime));
	float bh = GetNoteBoxHeight();
	float bw = GetNoteBoxWidth();

	*pVector0 = Vector3(x, center.y + bh / 2.0f, center.z + bw / 2.0f);
	*pVector1 = Vector3(x, center.y + bh / 2.0f, center.z - bw / 2.0f);
	*pVector2 = Vector3(x, center.y - bh / 2.0f, center.z + bw / 2.0f);
	*pVector3 = Vector3(x, center.y - bh / 2.0f, center.z - bw / 2.0f);
}

//******************************************************************************
// Grid box vertex positions
//******************************************************************************
void MTNoteDesign::GetGridBoxVirtexPos(
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
// Live grid box vertex positions
//******************************************************************************
void MTNoteDesign::GetGridBoxVirtexPosLive(
		unsigned long elapsedTime,
		unsigned char portNo,
		Vector3* pVector0,
		Vector3* pVector1,
		Vector3* pVector2,
		Vector3* pVector3
	)
{
	float x = -(GetLivePosX(elapsedTime));
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
void MTNoteDesign::GetPlaybackSectionVirtexPos(
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
// Ripple parameters
//******************************************************************************
float MTNoteDesign::GetRippleHeight(unsigned long elapsedTime)
{
	if ((int)elapsedTime <= m_RippleDuration) {
		return m_RippleHeight * (1.0f - ((float)elapsedTime / m_RippleDuration));
	}
	return 0.0f;
}

float MTNoteDesign::GetRippleWidth(unsigned long elapsedTime)
{
	if ((int)elapsedTime <= m_RippleDuration) {
		return m_RippleWidth * (1.0f - ((float)elapsedTime / m_RippleDuration));
	}
	return 0.0f;
}

float MTNoteDesign::GetRippleAlpha(unsigned long elapsedTime)
{
	if ((int)elapsedTime <= m_RippleDuration) {
		return 1.0f - ((float)elapsedTime / m_RippleDuration);
	}
	return 1.0f;
}

//******************************************************************************
// Picture board relative position
//******************************************************************************
float MTNoteDesign::GetPictBoardRelativePos()
{
	return m_PictBoardRelativePos;
}

//******************************************************************************
// Port origin coordinates
//******************************************************************************
float MTNoteDesign::GetPortOriginY(unsigned char portNo)
{
	return (0.0f - (GetNoteStep() * 127.0f / 2.0f));
}

float MTNoteDesign::GetPortOriginZ(unsigned char portNo)
{
	float pIdx = (float)(m_PortIndex[portNo]);
	float portWidth = GetChStep() * 16.0f;
	return (portWidth * pIdx - portWidth * m_PortList.GetSize() / 2.0f);
}

//******************************************************************************
// World move vector
//******************************************************************************
Vector3 MTNoteDesign::GetWorldMoveVector()
{
	return Vector3(0.0f, -GetPortOriginY(0), -GetPortOriginZ(0));
}

//******************************************************************************
// Note box color
//******************************************************************************
Color MTNoteDesign::GetNoteBoxColor(
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
// Active note box color
//******************************************************************************
Color MTNoteDesign::GetActiveNoteBoxColor(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned long elapsedTime
	)
{
	Color color = GetNoteBoxColor(portNo, chNo, noteNo);

	float rate = 0.0f;
	if ((int)elapsedTime < m_ActiveNoteDuration) {
		rate = 1.0f - ((float)elapsedTime / (float)m_ActiveNoteDuration);
	}

	float r = color.R() + ((1.0f - color.R()) * rate * m_ActiveNoteWhiteRate);
	float g = color.G() + ((1.0f - color.G()) * rate * m_ActiveNoteWhiteRate);
	float b = color.B() + ((1.0f - color.B()) * rate * m_ActiveNoteWhiteRate);
	float a = color.A();

	return Color(r, g, b, a);
}

Color MTNoteDesign::GetActiveNoteEmissive()   { return m_ActiveNoteEmissive; }
Color MTNoteDesign::GetGridLineColor()        { return m_GridLineColor; }
Color MTNoteDesign::GetPlaybackSectionColor() { return m_PlaybackSectionColor; }

//******************************************************************************
// Clear
//******************************************************************************
void MTNoteDesign::_Clear()
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

	m_ActiveNoteDuration = 400;
	m_ActiveNoteWhiteRate = 1.0f;
	m_ActiveNoteBoxSizeRatio = 1.0f;
	m_RippleDuration = 1600;

	m_LiveMonitorDisplayDuration = 30000;
	m_LiveNoteLengthPerSecond = 2.0f;
}

//******************************************************************************
// Load configuration
//******************************************************************************
int MTNoteDesign::_LoadConfFile(const TCHAR* pSceneName)
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
	confFile.GetFloat(_T("LiveNoteLengthPerSecond"), &m_LiveNoteLengthPerSecond, 2.0f);
	confFile.GetInt(_T("LiveMonitorDisplayDuration"), &m_LiveMonitorDisplayDuration, 30000);

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

	confFile.GetInt(_T("Duration"), &m_ActiveNoteDuration, 400);
	confFile.GetFloat(_T("WhiteRate"), &m_ActiveNoteWhiteRate, 0.9f);

	confFile.GetStr(_T("EmissiveRGBA"), hexColor, 16, _T("1A1A1A1A"));
	m_ActiveNoteEmissive = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	confFile.GetFloat(_T("SizeRatio"), &m_ActiveNoteBoxSizeRatio, 1.4f);

	// Ripple parameters
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	confFile.GetInt(_T("Duration"), &m_RippleDuration, 1600);

EXIT:;
	return result;
}

//******************************************************************************
// Load user config
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

	result = confFile.SetCurSection(_T("QuarterNote"));
	result = confFile.GetInt(_T("LengthMagnification"), &lengthMagnification, 100);

	if (lengthMagnification < 0) lengthMagnification = 0;
	if (lengthMagnification > 1000) lengthMagnification = 1000;

	m_QuarterNoteLength = m_QuarterNoteLength * ((float)lengthMagnification / 100.0f);

EXIT:;
	return result;
}
