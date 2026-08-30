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
	DWORD  CurrentEngine;
	DWORD  AudioFrequency;
	DWORD  AudioBitDepth;
	DWORD  AudioSampleFormat;
	BOOL   SincInter;
	DWORD  SincConv;
	DWORD  OutputVolume;
	FLOAT  RenderLoad;
	DOUBLE AudioLatency;
	DWORD  AudioBufferSize;
	char   ASIODeviceName[32];
	DWORD  CurrentSFList;
	DWORD  NumChannels;
	DWORD  TotalActiveVoices;
	DWORD  MaxVoices;
	DWORD  ActiveVoicesEx[128];
	DWORD  ActiveNotesEx[128];
};

static const wchar_t* OMNIMIDI_PIPE_TEMPLATE = L"\\\\.\\pipe\\OmniMIDIDbg%u";
static const unsigned int PIPE_MAX_ATTEMPTS = 8;

RDKdmapiInfo::RDKdmapiInfo()
	: m_mode(SynthMode::None)
	, m_modInfoCollected(false)
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

	// Get KDMAPI version (available on both Mod and Standard)
	auto pfnReturnKDMAPIVer = reinterpret_cast<ReturnKDMAPIVerFunc>(
		GetProcAddress(hMod, "ReturnKDMAPIVer"));
	if (pfnReturnKDMAPIVer != nullptr) {
		DWORD major = 0, minor = 0, build = 0, rev = 0;
		pfnReturnKDMAPIVer(&major, &minor, &build, &rev);
		char verStr[64];
		snprintf(verStr, sizeof(verStr), "%u.%u.%u", major, minor, build);
		RDDiagManager::SetString(RDMetricId::KdmapiVersion, verStr);
	}

	m_pfnGetModExtendedDebugInfo = reinterpret_cast<GetModExtendedDebugInfoFunc>(
		GetProcAddress(hMod, "GetModExtendedDebugInfo"));

	if (m_pfnGetModExtendedDebugInfo != nullptr) {
		m_mode = SynthMode::Mod;
		m_modInfoCollected = false;
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
		RDDiagManager::SetString(RDMetricId::KdmapiModVersion, "Std");
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

		// One-shot: Mod version (available before BASS init via DllMain pre-population)
		if (!m_modInfoCollected && info->ModVersionDate > 0) {
			DWORD date = info->ModVersionDate;
			char modVerStr[64];
			snprintf(modVerStr, sizeof(modVerStr), "Mod %04u-%02u-%02u",
				date / 10000, (date / 100) % 100, date % 100);
			RDDiagManager::SetString(RDMetricId::KdmapiModVersion, modVerStr);

			char omVerStr[32];
			snprintf(omVerStr, sizeof(omVerStr), "%u.%u.%u",
				info->ModVersionMajor, info->ModVersionMinor, info->ModVersionPatch);
			RDDiagManager::SetString(RDMetricId::KdmapiOmniMidiVersion, omVerStr);

			m_modInfoCollected = true;
		}

		// Polling: Synth performance metrics
		RDDiagManager::SetFloat(RDMetricId::SynthRenderLoad, info->RenderLoad);
		RDDiagManager::SetFloat(RDMetricId::SynthRenderHeadroom, 100.0f - info->RenderLoad);
		RDDiagManager::SetFloat(RDMetricId::SynthAudioLatency, static_cast<float>(info->AudioLatency));
		RDDiagManager::SetInt(RDMetricId::SynthActiveVoices, info->TotalActiveVoices);
		RDDiagManager::SetInt(RDMetricId::SynthMaxVoices, info->MaxVoices);

		// Polling: Audio runtime info (BUG-26: all fields now from BASS API)
		if (info->StructSize >= sizeof(ExtendedDebugInfo)) {
			const char* engineNames[] = { "WAV", "BASS", "ASIO", "WASAPI", "XAudio" };
			DWORD engine = info->CurrentEngine;
			bool engineActive = (engine < 5);

			if (engineActive) {
				RDDiagManager::SetString(RDMetricId::KdmapiAudioEngine, engineNames[engine]);
				RDDiagManager::SetInt(RDMetricId::SynthAudioFrequency, info->AudioFrequency);
				RDDiagManager::SetInt(RDMetricId::SynthAudioBufferSize, info->AudioBufferSize);
				RDDiagManager::SetInt(RDMetricId::SynthAudioBitDepth, info->AudioBitDepth);

				BOOL sinc = info->SincInter;
				if (sinc == TRUE) {
					RDDiagManager::SetString(RDMetricId::KdmapiSincInterpolation, "ON");
					const char* sincQualityNames[] = { "Linear", "8pt", "16pt", "32pt", "64pt" };
					DWORD sq = info->SincConv;
					if (sq < 5) {
						RDDiagManager::SetString(RDMetricId::KdmapiSincConvQuality, sincQualityNames[sq]);
					} else {
						RDDiagManager::ClearMetric(RDMetricId::KdmapiSincConvQuality);
					}
				} else if (sinc == FALSE) {
					RDDiagManager::SetString(RDMetricId::KdmapiSincInterpolation, "OFF");
					RDDiagManager::SetString(RDMetricId::KdmapiSincConvQuality, "OFF");
				} else {
					RDDiagManager::ClearMetric(RDMetricId::KdmapiSincInterpolation);
					RDDiagManager::ClearMetric(RDMetricId::KdmapiSincConvQuality);
				}

				if (info->ASIODeviceName[0] != '\0') {
					RDDiagManager::SetString(RDMetricId::KdmapiASIODeviceName, info->ASIODeviceName);
				} else {
					RDDiagManager::ClearMetric(RDMetricId::KdmapiASIODeviceName);
				}

				RDDiagManager::SetInt(RDMetricId::SynthCurrentSFList, info->CurrentSFList);
				RDDiagManager::SetInt(RDMetricId::SynthNumChannels, info->NumChannels);

				const char* sampleTypeNames[] = { "unknown", "int", "float" };
				DWORD fmt = info->AudioSampleFormat;
				RDDiagManager::SetString(RDMetricId::SynthAudioSampleType,
					(fmt < 3) ? sampleTypeNames[fmt] : "unknown");

				DWORD vol = info->OutputVolume;
				if (vol <= 10000) {
					RDDiagManager::SetFloat(RDMetricId::SynthOutputVolume, vol / 100.0);
				} else {
					RDDiagManager::ClearMetric(RDMetricId::SynthOutputVolume);
				}
			} else {
				RDDiagManager::ClearMetric(RDMetricId::KdmapiAudioEngine);
				RDDiagManager::ClearMetric(RDMetricId::SynthAudioFrequency);
				RDDiagManager::ClearMetric(RDMetricId::SynthAudioBufferSize);
				RDDiagManager::ClearMetric(RDMetricId::SynthAudioBitDepth);
				RDDiagManager::ClearMetric(RDMetricId::KdmapiSincInterpolation);
				RDDiagManager::ClearMetric(RDMetricId::KdmapiSincConvQuality);
				RDDiagManager::ClearMetric(RDMetricId::KdmapiASIODeviceName);
				RDDiagManager::ClearMetric(RDMetricId::SynthCurrentSFList);
				RDDiagManager::ClearMetric(RDMetricId::SynthNumChannels);
				RDDiagManager::ClearMetric(RDMetricId::SynthAudioSampleType);
				RDDiagManager::ClearMetric(RDMetricId::SynthOutputVolume);
			}
		}
	}
	else if (m_mode == SynthMode::Standard && m_pfnGetDriverDebugInfo != nullptr) {
		if (m_hPipe == INVALID_HANDLE_VALUE) {
			_ConnectDebugPipe();
		}
		_DrainPipe();

		DebugInfo* info = m_pfnGetDriverDebugInfo();
		if (info == nullptr) return;

		RDDiagManager::SetFloat(RDMetricId::SynthRenderLoad, info->RenderingTime);
		RDDiagManager::SetFloat(RDMetricId::SynthRenderHeadroom, 100.0f - info->RenderingTime);
		RDDiagManager::SetFloat(RDMetricId::SynthAudioLatency, static_cast<float>(info->AudioLatency));

		DWORD totalVoices = 0;
		for (int i = 0; i < 16; ++i) {
			totalVoices += info->ActiveVoices[i];
		}
		RDDiagManager::SetInt(RDMetricId::SynthActiveVoices, totalVoices);

		DWORD maxVoices = 0;
		HKEY hKey = NULL;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\OmniMIDI\\Configuration",
				0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			DWORD dwSize = sizeof(DWORD);
			RegQueryValueExW(hKey, L"MaxVoices", NULL, NULL, (LPBYTE)&maxVoices, &dwSize);
			RegCloseKey(hKey);
		}
		RDDiagManager::SetInt(RDMetricId::SynthMaxVoices, maxVoices);
	}
}
