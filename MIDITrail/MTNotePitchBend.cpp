//******************************************************************************
//
// MIDITrail / MTNotePitchBend
//
// Pitch bend data manager.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNotePitchBend.h"

using namespace YNBaseLib;


//******************************************************************************
// Constructor
//******************************************************************************
MTNotePitchBend::MTNotePitchBend(void)
{
	Reset();
	m_isEnable = true;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTNotePitchBend::~MTNotePitchBend(void)
{
}

//******************************************************************************
// Initialize
//******************************************************************************
int MTNotePitchBend::Initialize()
{
	int result = 0;

	Reset();

	return result;
}

//******************************************************************************
// Set pitch bend
//******************************************************************************
int MTNotePitchBend::SetPitchBend(
		unsigned char portNo,
		unsigned char chNo,
		short value,
		unsigned char sensitivity
	)
{
	int result = 0;

	if (chNo >= SM_MAX_CH_NUM) {
		result = YN_SET_ERR("Program error.", value, sensitivity);
		goto EXIT;
	}

	m_PitchBend[portNo][chNo].value = value;
	m_PitchBend[portNo][chNo].sensitivity = sensitivity;

EXIT:;
	return result;
}

//******************************************************************************
// Get pitch bend value
//******************************************************************************
short MTNotePitchBend::GetValue(
		unsigned long portNo,
		unsigned long chNo
	)
{
	short value = 0;

	if ((portNo < SM_MAX_PORT_NUM) && (chNo < SM_MAX_CH_NUM)) {
		value = m_PitchBend[portNo][chNo].value;
	}

	if (!m_isEnable) {
		value = 0;
	}

	return value;
}

//******************************************************************************
// Get pitch bend sensitivity
//******************************************************************************
unsigned char MTNotePitchBend::GetSensitivity(
		unsigned long portNo,
		unsigned long chNo
	)
{
	unsigned char sensitivity = 0;

	if ((portNo < SM_MAX_PORT_NUM) && (chNo < SM_MAX_CH_NUM)) {
		sensitivity = m_PitchBend[portNo][chNo].sensitivity;
	}

	if (!m_isEnable) {
		sensitivity = 0;
	}

	return sensitivity;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTNotePitchBend::Reset()
{
	unsigned long portNo = 0;
	unsigned long chNo = 0;

	for (portNo = 0; portNo < SM_MAX_PORT_NUM; portNo++) {
		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
			m_PitchBend[portNo][chNo].value = 0;
			m_PitchBend[portNo][chNo].sensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
		}
	}

	return;
}

//******************************************************************************
// Set display
//******************************************************************************
void MTNotePitchBend::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


