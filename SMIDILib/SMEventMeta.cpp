//******************************************************************************
//
// Simple MIDI Library / SMEventMeta
//
// Meta event class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2017-2025 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMeta.h"
#include <new>

#include <algorithm>
#include <functional>
#include <cctype>

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// Constructor
//******************************************************************************
SMEventMeta::SMEventMeta()
{
	m_pEvent = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
SMEventMeta::~SMEventMeta(void)
{
}

//******************************************************************************
// Attach event
//******************************************************************************
void SMEventMeta::Attach(
		SMEvent* pEvent
	)
{
	m_pEvent = pEvent;
}

//******************************************************************************
// Get meta event type
//******************************************************************************
unsigned char SMEventMeta::GetType()
{
	unsigned char type = 0;

	if (m_pEvent == NULL) goto EXIT;

	type = m_pEvent->GetMetaType();

EXIT:;
	return type;
}

//******************************************************************************
// Get tempo
//******************************************************************************
unsigned long SMEventMeta::GetTempo()
{
	unsigned long tempo = 0;
	unsigned char* pData = NULL;

	if (m_pEvent == NULL) goto EXIT;

	if (m_pEvent->GetMetaType() != 0x51) {
		goto EXIT;
	}
	if (m_pEvent->GetDataSize() != 3) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	tempo = (pData[0] << 16) | (pData[1] << 8) | (pData[2]);

EXIT:;
	return tempo;
}

//******************************************************************************
// Get tempo(BPM)
//******************************************************************************
unsigned long SMEventMeta::GetTempoBPM()
{
	unsigned long tempo = 0;
	unsigned long tempoBPM = 0;

	tempo = GetTempo();
	if (tempo == 0) goto EXIT;

	tempoBPM = (60 * 1000 * 1000) / tempo;

EXIT:;
	return tempoBPM;
}

//******************************************************************************
// Get text
//******************************************************************************
int SMEventMeta::GetText(
		std::string* pText
	)
{
	int result = 0;
	char* pBuf = NULL;
	unsigned long size = 0;

	if (m_pEvent == NULL) goto EXIT;

	size =  m_pEvent->GetDataSize();

	try {
		pBuf = new char[size + 1];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", size + 1, 0);
		goto EXIT;
	}

	memcpy(pBuf, m_pEvent->GetDataPtr(), size);
	pBuf[size] = '\0';

	*pText = pBuf;


	//// rtrim
	//pText->erase(std::find_if(pText->rbegin(), pText->rend(),
	//	std::not1(std::ptr_fun<int, int>(std::isspace))).base(), pText->end());

	// rtrim
	struct local_func {
		static int isspace(int ch)
		{
			// std::isspace triggers an assertion in the standard library during debug runs,
			// so this is fixed to avoid passing negative values
			return std::isspace(static_cast<unsigned int>(ch) & 0xff);
		}
	};
	pText->erase(std::find_if(pText->rbegin(), pText->rend(),
		std::not1(std::ptr_fun<int, int>(local_func::isspace))).base(), pText->end());



EXIT:;
	delete [] pBuf;
	return result;
}

//******************************************************************************
// Get port number
//******************************************************************************
unsigned char SMEventMeta::GetPortNo()
{
	unsigned char portNo = 0;
	unsigned char* pData = NULL;

	if (m_pEvent == NULL) goto EXIT;

	if (m_pEvent->GetMetaType() != 0x21) {
		goto EXIT;
	}
	if (m_pEvent->GetDataSize() != 1) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();
	portNo = pData[0];

EXIT:;
	return portNo;
}

//******************************************************************************
// Get time signature
//******************************************************************************
void SMEventMeta::GetTimeSignature(
		unsigned long* pNumerator,
		unsigned long* pDenominator
	)
{
	unsigned char* pData = NULL;
	unsigned long i = 0;

	if (m_pEvent == NULL) goto EXIT;

	if (m_pEvent->GetMetaType() != 0x58) {
		goto EXIT;
	}
	if (m_pEvent->GetDataSize() != 4) {
		goto EXIT;
	}

	pData = m_pEvent->GetDataPtr();

	// FF 58 04 nn dd cc bb
	//   nn: numerator
	//   dd: denominator (as a negative power of 2)
	//   cc: number of MIDI clocks per metronome click
	//   bb: number of notated 32nd notes per MIDI quarter note (24 MIDI clocks) (usually 8)
	//   -> cc and bb are ignored

	//numerator
	*pNumerator   = pData[0];

	//denominator
	*pDenominator = 1;
	for (i = 0; i < pData[1]; i++) {
		*pDenominator = *pDenominator * 2;
	}

EXIT:;
	return;
}

} // end of namespace

