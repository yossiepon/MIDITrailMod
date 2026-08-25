#include "stdafx.h"
#include "RDSynthInfo.h"
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

RDSynthInfo::RDSynthInfo()
	: m_mode(SynthMode::None)
	, m_pfnGetModExtendedDebugInfo(nullptr)
	, m_pfnGetDriverDebugInfo(nullptr)
{
}

RDSynthInfo::~RDSynthInfo()
{
}

void RDSynthInfo::CollectStartup()
{
	RDDiagManager::SetString(RDMetricId::KdmapiStatus, "N/A");
}

bool RDSynthInfo::_TryDetect()
{
	auto logger = spdlog::get("RD");

	HMODULE hMod = GetModuleHandleA("OmniMIDI");
	if (hMod == NULL) {
		return false;
	}

	if (logger) {
		char dllPath[MAX_PATH] = {};
		GetModuleFileNameA(hMod, dllPath, MAX_PATH);
		logger->info("RDSynthInfo: OmniMIDI loaded from: {}", dllPath);
	}

	m_pfnGetModExtendedDebugInfo = reinterpret_cast<GetModExtendedDebugInfoFunc>(
		GetProcAddress(hMod, "GetModExtendedDebugInfo"));

	if (m_pfnGetModExtendedDebugInfo != nullptr) {
		m_mode = SynthMode::Mod;
		RDDiagManager::SetString(RDMetricId::KdmapiStatus, "Mod");
		if (logger) logger->info("RDSynthInfo: OmniMIDI Mod detected (ExtendedDebugInfo available)");
		return true;
	}

	if (logger) logger->info("RDSynthInfo: GetModExtendedDebugInfo not found, trying GetDriverDebugInfo");

	m_pfnGetDriverDebugInfo = reinterpret_cast<GetDriverDebugInfoFunc>(
		GetProcAddress(hMod, "GetDriverDebugInfo"));

	if (m_pfnGetDriverDebugInfo != nullptr) {
		m_mode = SynthMode::Standard;
		RDDiagManager::SetString(RDMetricId::KdmapiStatus, "Std");
		if (logger) logger->info("RDSynthInfo: OmniMIDI standard detected (DebugInfo only)");
		return true;
	}

	if (logger) logger->warn("RDSynthInfo: OmniMIDI loaded but no debug API found");
	return false;
}

void RDSynthInfo::CollectIntervalPolling()
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
		DebugInfo* info = m_pfnGetDriverDebugInfo();
		if (info == nullptr) return;

		RDDiagManager::SetFloat(RDMetricId::KdmapiRenderingTime, info->RenderingTime);
		RDDiagManager::SetFloat(RDMetricId::KdmapiAudioLatency, static_cast<float>(info->AudioLatency));

		DWORD totalVoices = 0;
		for (int i = 0; i < 16; ++i) {
			totalVoices += info->ActiveVoices[i];
		}
		RDDiagManager::SetInt(RDMetricId::KdmapiTotalActiveVoices, totalVoices);
		RDDiagManager::SetInt(RDMetricId::KdmapiMaxVoices, 0);
	}
}
