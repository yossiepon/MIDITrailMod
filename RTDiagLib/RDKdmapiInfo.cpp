//******************************************************************************
//
// RTDiagLib / RDKdmapiInfo
//
// OmniMIDI KDMAPI synthesizer info collector.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDKdmapiInfo.h"
#include "RDDiagManager.h"
#include <spdlog/spdlog.h>

// OmniMIDI struct layouts (matching DeveloperContent/OmniMIDI.h)
// Defined locally to avoid build dependency on OmniMIDIMod.
struct DebugInfo
{
	FLOAT  RenderingTime;
	DWORD  ActiveVoices[16];
	DOUBLE ASIOInputLatency;
	DOUBLE ASIOOutputLatency;
	DOUBLE HealthThreadTime;
	DOUBLE ATThreadTime;
	DOUBLE EPThreadTime;
	DOUBLE CookedThreadTime;
	DWORD  CurrentSFList;
	DOUBLE AudioLatency;
	DWORD  AudioBufferSize;
};

struct ExtendedDebugInfo
{
	DWORD  StructSize;
	DWORD  ModVersionMajor;
	DWORD  ModVersionMinor;
	DWORD  ModVersionPatch;
	DWORD  ModVersionDate;
	FLOAT  RenderingTime;
	DOUBLE AudioLatency;
	DWORD  AudioBufferSize;
	DOUBLE ASIOInputLatency;
	DOUBLE ASIOOutputLatency;
	DWORD  CurrentSFList;
	DWORD  ActiveVoicesEx[128];
	DWORD  TotalActiveVoices;
	DWORD  MaxVoices;
	DWORD  ActiveNotesEx[128];
	DWORD  NumChannels;
};

static const wchar_t* OMNIMIDI_PIPE_TEMPLATE = L"\\\\.\\pipe\\OmniMIDIDbg%u";
static const unsigned int PIPE_MAX_ATTEMPTS = 8;

RDKdmapiInfo::RDKdmapiInfo()
	: m_mode(SynthMode::None)
	, m_pfnGetModExtendedDebugInfo(nullptr)
	, m_pfnGetDriverDebugInfo(nullptr)
	, m_hPipe(INVALID_HANDLE_VALUE)
{
}

RDKdmapiInfo::~RDKdmapiInfo()
{
	if (m_hPipe != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}
}

void RDKdmapiInfo::CollectStartup()
{
	HMODULE hMod = LoadLibraryA("OmniMIDI.dll");
	if (hMod != NULL) {
		_TryDetect();
	} else {
		RDDiagManager::SetString(RDMetricId::KdmapiStatus, "N/A");
	}
}

bool RDKdmapiInfo::_TryDetect()
{
	auto logger = spdlog::get("RD");

	HMODULE hMod = GetModuleHandleA("OmniMIDI");
	if (hMod == NULL) {
		return false;
	}

	if (logger) {
		char dllPath[MAX_PATH] = {};
		GetModuleFileNameA(hMod, dllPath, MAX_PATH);
		logger->info("RDKdmapiInfo: OmniMIDI loaded from: {}", dllPath);
	}

	m_pfnGetModExtendedDebugInfo = reinterpret_cast<GetModExtendedDebugInfoFunc>(
		GetProcAddress(hMod, "GetModExtendedDebugInfo"));

	if (m_pfnGetModExtendedDebugInfo != nullptr) {
		m_mode = SynthMode::Mod;
		RDDiagManager::SetString(RDMetricId::KdmapiStatus, "Mod");
		if (logger) logger->info("RDKdmapiInfo: OmniMIDI Mod detected (ExtendedDebugInfo available)");
		return true;
	}

	if (logger) logger->info("RDKdmapiInfo: GetModExtendedDebugInfo not found, trying GetDriverDebugInfo");

	m_pfnGetDriverDebugInfo = reinterpret_cast<GetDriverDebugInfoFunc>(
		GetProcAddress(hMod, "GetDriverDebugInfo"));

	if (m_pfnGetDriverDebugInfo != nullptr) {
		m_mode = SynthMode::Standard;
		RDDiagManager::SetString(RDMetricId::KdmapiStatus, "Std");
		_ConnectDebugPipe();
		if (logger) logger->info("RDKdmapiInfo: OmniMIDI standard detected (DebugInfo only, pipe={})",
			m_hPipe != INVALID_HANDLE_VALUE ? "connected" : "unavailable");
		return true;
	}

	if (logger) logger->warn("RDKdmapiInfo: OmniMIDI loaded but no debug API found");
	return false;
}

bool RDKdmapiInfo::_ConnectDebugPipe()
{
	wchar_t pipeName[MAX_PATH];
	for (unsigned int i = 1; i <= PIPE_MAX_ATTEMPTS; ++i) {
		swprintf_s(pipeName, MAX_PATH, OMNIMIDI_PIPE_TEMPLATE, i);
		m_hPipe = CreateFileW(
			pipeName,
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (m_hPipe != INVALID_HANDLE_VALUE) {
			DWORD mode = PIPE_READMODE_BYTE;
			SetNamedPipeHandleState(m_hPipe, &mode, NULL, NULL);
			return true;
		}
	}
	return false;
}

void RDKdmapiInfo::_DrainPipe()
{
	if (m_hPipe == INVALID_HANDLE_VALUE) return;

	DWORD bytesAvailable = 0;
	if (!PeekNamedPipe(m_hPipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
		return;
	}

	if (bytesAvailable > 0) {
		char buf[4096];
		while (bytesAvailable > 0) {
			DWORD toRead = (bytesAvailable < sizeof(buf)) ? bytesAvailable : sizeof(buf);
			DWORD bytesRead = 0;
			if (!ReadFile(m_hPipe, buf, toRead, &bytesRead, NULL)) {
				CloseHandle(m_hPipe);
				m_hPipe = INVALID_HANDLE_VALUE;
				return;
			}
			bytesAvailable -= bytesRead;
		}
	}
}

void RDKdmapiInfo::CollectIntervalPolling()
{
	if (m_mode == SynthMode::None) {
		_TryDetect();
		if (m_mode == SynthMode::None) return;
	}

	if (m_mode == SynthMode::Mod && m_pfnGetModExtendedDebugInfo != nullptr) {
		ExtendedDebugInfo* info = m_pfnGetModExtendedDebugInfo();
		if (info == nullptr) return;

		RDDiagManager::SetFloat(RDMetricId::KdmapiRenderingTime, info->RenderingTime);
		RDDiagManager::SetFloat(RDMetricId::KdmapiAudioLatency, static_cast<float>(info->AudioLatency));
		RDDiagManager::SetInt(RDMetricId::KdmapiTotalActiveVoices, info->TotalActiveVoices);
		RDDiagManager::SetInt(RDMetricId::KdmapiMaxVoices, info->MaxVoices);
	}
	else if (m_mode == SynthMode::Standard && m_pfnGetDriverDebugInfo != nullptr) {
		if (m_hPipe == INVALID_HANDLE_VALUE) {
			_ConnectDebugPipe();
		}
		_DrainPipe();

		DebugInfo* info = m_pfnGetDriverDebugInfo();
		if (info == nullptr) return;

		RDDiagManager::SetFloat(RDMetricId::KdmapiRenderingTime, info->RenderingTime);
		RDDiagManager::SetFloat(RDMetricId::KdmapiAudioLatency, static_cast<float>(info->AudioLatency));

		DWORD totalVoices = 0;
		for (int i = 0; i < 16; ++i) {
			totalVoices += info->ActiveVoices[i];
		}
		RDDiagManager::SetInt(RDMetricId::KdmapiTotalActiveVoices, totalVoices);

		DWORD maxVoices = 0;
		HKEY hKey = NULL;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\OmniMIDI\\Configuration",
				0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			DWORD dwSize = sizeof(DWORD);
			RegQueryValueExW(hKey, L"MaxVoices", NULL, NULL, (LPBYTE)&maxVoices, &dwSize);
			RegCloseKey(hKey);
		}
		RDDiagManager::SetInt(RDMetricId::KdmapiMaxVoices, maxVoices);
	}
}
