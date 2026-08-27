//******************************************************************************
//
// RTDiagLib / RDDiagManager
//
// Diagnostics manager (metric storage, formatting, component lifecycle).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDDiagManager.h"
#include "RDOsInfo.h"
#include "RDCpuInfo.h"
#include "RDGpuInfo.h"
#include "RDMemoryInfo.h"
#include "RDWmiInfo.h"
#include "RDAppMetrics.h"
#include "RDKdmapiInfo.h"
#include "RDGpuVendorTelemetry.h"
#include "RDFormatProfiles.h"
#include <spdlog/spdlog.h>
#include <cassert>

extern const RDMetricMeta* RDGetMetricMetaTable();
extern size_t RDGetMetricMetaCount();

bool RDDiagManager::s_isInitialized = false;
RDDiagManager::MetricValue RDDiagManager::s_metrics[static_cast<size_t>(RDMetricId::COUNT)];

std::vector<IRDStartupComponent*> RDDiagManager::s_startupComponents;
std::vector<RDDiagManager::IntervalEntry> RDDiagManager::s_intervalComponents;
std::vector<IRDFrameComponent*> RDDiagManager::s_frameComponents;

std::unordered_map<std::string, size_t> RDDiagManager::s_keyToIdMap;

std::vector<std::string> RDDiagManager::s_formatBuffer;
LARGE_INTEGER RDDiagManager::s_perfFrequency = {};

std::unordered_map<const RDFormatTemplateEntry*, std::vector<RDDiagManager::CompiledTemplate>>
	RDDiagManager::s_compiledProfiles;

std::vector<std::function<void()>> RDDiagManager::s_componentDeleters;
ID3D11DeviceContext* RDDiagManager::s_pDeviceContext = nullptr;
RDGpuTimestamp RDDiagManager::s_gpuTimestamp;
LARGE_INTEGER RDDiagManager::s_lastLogTime = {};
DWORD RDDiagManager::s_logIntervalMs = 10000;
std::future<void> RDDiagManager::s_asyncStartup;
std::atomic<bool> RDDiagManager::s_asyncStartupDone{false};

int RDDiagManager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (s_isInitialized) {
		return 0;
	}

	{
		auto logger = spdlog::get("RD");
		if (logger) {
			logger->debug("RDDiagManager::Initialize begin");
		}
	}

	s_pDeviceContext = pContext;

	QueryPerformanceFrequency(&s_perfFrequency);
	for (auto& m : s_metrics) {
		m.intVal = 0;
		m.floatVal = 0.0;
		m.strVal.clear();
	}

	_BuildKeyToIdMap();

	// Phase 1A components
	// Order matters: RDWmiInfo reads GpuName (set by RDGpuInfo) to match driver version
	{
		auto* osInfo = new RDOsInfo();
		RegisterStartupComponent(osInfo);
		s_componentDeleters.push_back([osInfo]() { delete osInfo; });

		auto* cpuInfo = new RDCpuInfo();
		RegisterStartupComponent(cpuInfo);
		RegisterIntervalPollingComponent(cpuInfo);
		s_componentDeleters.push_back([cpuInfo]() { delete cpuInfo; });

		auto* gpuInfo = new RDGpuInfo();
		gpuInfo->SetDevice(pDevice);
		RegisterStartupComponent(gpuInfo);
		RegisterIntervalPollingComponent(gpuInfo);
		s_componentDeleters.push_back([gpuInfo]() { delete gpuInfo; });

		auto* memInfo = new RDMemoryInfo();
		RegisterStartupComponent(memInfo);
		RegisterIntervalPollingComponent(memInfo);
		s_componentDeleters.push_back([memInfo]() { delete memInfo; });

		auto* wmiInfo = new RDWmiInfo();
		RegisterStartupComponent(wmiInfo);
		s_componentDeleters.push_back([wmiInfo]() { delete wmiInfo; });

		auto* appMetrics = new RDAppMetrics();
		RegisterFrameComponent(appMetrics);
		s_componentDeleters.push_back([appMetrics]() { delete appMetrics; });

		auto* synthInfo = new RDKdmapiInfo();
		RegisterStartupComponent(synthInfo);
		RegisterIntervalPollingComponent(synthInfo);
		s_componentDeleters.push_back([synthInfo]() { delete synthInfo; });

		auto* gpuTelemetry = new RDGpuVendorTelemetry();
		gpuTelemetry->SetVendorId(gpuInfo->GetVendorId());
		gpuTelemetry->InitializeProvider();
		RegisterIntervalPollingComponent(gpuTelemetry);
		s_componentDeleters.push_back([gpuTelemetry]() { delete gpuTelemetry; });
	}

	// Startup: fast components run synchronously, slow components run async
	// Registration order: 0=OsInfo, 1=CpuInfo, 2=GpuInfo, 3=MemoryInfo, 4=WmiInfo, 5=SynthInfo
	// Fast (sync, timed): OsInfo(0), CpuInfo(1), MemoryInfo(3)
	// Fast (sync, untimed): SynthInfo(5) — trivial cost, called after timing loop
	// Slow (async): GpuInfo(2), WmiInfo(4)

	static const RDMetricId syncTimingIds[] = {
		RDMetricId::DiagStartupOsInfoUs,
		RDMetricId::DiagStartupCpuInfoUs,
		RDMetricId::DiagStartupMemoryInfoUs,
	};
	static const size_t syncIndices[] = { 0, 1, 3 };

	static const RDMetricId asyncTimingIds[] = {
		RDMetricId::DiagStartupGpuInfoUs,
		RDMetricId::DiagStartupWmiInfoUs,
	};
	static const size_t asyncIndices[] = { 2, 4 };

	{
		LARGE_INTEGER totalStart;
		QueryPerformanceCounter(&totalStart);

		for (size_t j = 0; j < sizeof(syncIndices) / sizeof(syncIndices[0]); j++) {
			size_t i = syncIndices[j];
			if (i >= s_startupComponents.size()) continue;
			LARGE_INTEGER compStart, compEnd;
			QueryPerformanceCounter(&compStart);
			s_startupComponents[i]->CollectStartup();
			QueryPerformanceCounter(&compEnd);
			int64_t us = (compEnd.QuadPart - compStart.QuadPart) * 1000000 / s_perfFrequency.QuadPart;
			SetInt(syncTimingIds[j], us);
		}

		LARGE_INTEGER syncEnd;
		QueryPerformanceCounter(&syncEnd);
		int64_t syncUs = (syncEnd.QuadPart - totalStart.QuadPart) * 1000000 / s_perfFrequency.QuadPart;
		SetInt(RDMetricId::DiagStartupTotalUs, syncUs);
	}

	// SynthInfo startup (trivial cost, untimed)
	if (5 < s_startupComponents.size()) {
		s_startupComponents[5]->CollectStartup();
	}

	if (pDevice != nullptr && pContext != nullptr) {
		s_gpuTimestamp.Initialize(pDevice, pContext);
	}

	s_isInitialized = true;
	s_asyncStartupDone.store(false);
	QueryPerformanceCounter(&s_lastLogTime);

	{
		auto logger = spdlog::get("RD");
		if (logger) {
			logger->debug("RDDiagManager initialized (async startup pending)");
		}
	}

	std::vector<IRDStartupComponent*> asyncComponents;
	for (size_t j = 0; j < sizeof(asyncIndices) / sizeof(asyncIndices[0]); j++) {
		size_t i = asyncIndices[j];
		if (i < s_startupComponents.size()) {
			asyncComponents.push_back(s_startupComponents[i]);
		}
	}

	s_asyncStartup = std::async(std::launch::async, _AsyncStartupProc,
		asyncComponents, asyncTimingIds,
		sizeof(asyncTimingIds) / sizeof(asyncTimingIds[0]));

	return 0;
}

void RDDiagManager::_AsyncStartupProc(
	std::vector<IRDStartupComponent*> components,
	const RDMetricId* timingIds, size_t timingCount)
{
	LARGE_INTEGER freq, totalStart;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&totalStart);

	for (size_t i = 0; i < components.size(); i++) {
		LARGE_INTEGER compStart, compEnd;
		QueryPerformanceCounter(&compStart);
		components[i]->CollectStartup();
		QueryPerformanceCounter(&compEnd);

		if (i < timingCount) {
			int64_t us = (compEnd.QuadPart - compStart.QuadPart) * 1000000 / freq.QuadPart;
			SetInt(timingIds[i], us);
		}
	}

	LARGE_INTEGER totalEnd;
	QueryPerformanceCounter(&totalEnd);
	int64_t prevUs = GetInt(RDMetricId::DiagStartupTotalUs);
	int64_t asyncUs = (totalEnd.QuadPart - totalStart.QuadPart) * 1000000 / freq.QuadPart;
	SetInt(RDMetricId::DiagStartupTotalUs, prevUs + asyncUs);

	s_asyncStartupDone.store(true, std::memory_order_release);
}

void RDDiagManager::_OnAsyncStartupComplete()
{
	s_asyncStartup.get();

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	for (auto& entry : s_intervalComponents) {
		entry.pComponent->CollectIntervalPolling();
		entry.lastCollected = now;
	}

	auto logger = spdlog::get("RD");
	if (logger) {
		auto signature = Format(
			RDFormatProfile::MachineSignature,
			RDFormatProfile::MachineSignatureCount);
		for (const auto& entry : signature) {
			logger->info("{}: {}", entry.label, entry.value);
		}
		logger->debug("Async startup complete");
	}
}

void RDDiagManager::Update()
{
	if (!s_isInitialized) {
		return;
	}

	if (!s_asyncStartupDone.load(std::memory_order_acquire)) {
		for (auto* pComp : s_frameComponents) {
			pComp->CollectFrame();
		}
		return;
	}

	if (s_asyncStartup.valid()) {
		_OnAsyncStartupComplete();
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	// Per-component polling timing metric IDs (same order as IntervalPolling registration)
	static const RDMetricId pollingTimingIds[] = {
		RDMetricId::DiagPollingCpuInfoUs,
		RDMetricId::DiagPollingGpuInfoUs,
		RDMetricId::DiagPollingMemoryInfoUs,
	};

	LARGE_INTEGER pollStart;
	QueryPerformanceCounter(&pollStart);
	int polledCount = 0;

	for (size_t i = 0; i < s_intervalComponents.size(); i++) {
		auto& entry = s_intervalComponents[i];
		double elapsedMs = static_cast<double>(now.QuadPart - entry.lastCollected.QuadPart)
			* 1000.0 / static_cast<double>(s_perfFrequency.QuadPart);

		if (elapsedMs >= static_cast<double>(entry.pComponent->GetPollingIntervalMs())) {
			LARGE_INTEGER compStart, compEnd;
			QueryPerformanceCounter(&compStart);
			entry.pComponent->CollectIntervalPolling();
			QueryPerformanceCounter(&compEnd);
			entry.lastCollected = now;
			polledCount++;

			if (i < sizeof(pollingTimingIds) / sizeof(pollingTimingIds[0])) {
				int64_t us = (compEnd.QuadPart - compStart.QuadPart) * 1000000 / s_perfFrequency.QuadPart;
				SetInt(pollingTimingIds[i], us);
			}
		}
	}

	if (polledCount > 0) {
		LARGE_INTEGER pollEnd;
		QueryPerformanceCounter(&pollEnd);
		int64_t pollingUs = (pollEnd.QuadPart - pollStart.QuadPart) * 1000000 / s_perfFrequency.QuadPart;
		SetInt(RDMetricId::DiagPollingTotalUs, pollingUs);
		SetInt(RDMetricId::DiagPollingCount, polledCount);
	}

	for (auto* pComp : s_frameComponents) {
		pComp->CollectFrame();
	}

	if (s_logIntervalMs > 0) {
		double logElapsedMs = static_cast<double>(now.QuadPart - s_lastLogTime.QuadPart)
			* 1000.0 / static_cast<double>(s_perfFrequency.QuadPart);
		if (logElapsedMs >= static_cast<double>(s_logIntervalMs)) {
			auto logger = spdlog::get("RD");
			if (logger) {
				auto rt = Format(RDFormatProfile::RuntimeSystem, RDFormatProfile::RuntimeSystemCount);
				for (const auto& entry : rt) {
					logger->debug("[runtime] {}: {}", entry.label, entry.value);
				}
			}
			s_lastLogTime = now;
		}
	}
}

void RDDiagManager::Terminate()
{
	if (!s_isInitialized) {
		return;
	}

	auto logger = spdlog::get("RD");
	if (logger) {
		logger->debug("RDDiagManager terminated");
	}

	if (s_asyncStartup.valid()) {
		s_asyncStartup.get();
	}
	s_asyncStartupDone.store(false);

	s_gpuTimestamp.Terminate();
	s_pDeviceContext = nullptr;

	s_startupComponents.clear();
	s_intervalComponents.clear();
	s_frameComponents.clear();
	for (auto& deleter : s_componentDeleters) {
		deleter();
	}
	s_componentDeleters.clear();
	s_keyToIdMap.clear();
	s_formatBuffer.clear();
	s_compiledProfiles.clear();

	for (auto& m : s_metrics) {
		m.intVal = 0;
		m.floatVal = 0.0;
		m.strVal.clear();
	}
	s_isInitialized = false;
}

double RDDiagManager::GetFloat(RDMetricId id)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	return s_metrics[static_cast<size_t>(id)].floatVal;
}

int64_t RDDiagManager::GetInt(RDMetricId id)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	return s_metrics[static_cast<size_t>(id)].intVal;
}

const char* RDDiagManager::GetString(RDMetricId id)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	return s_metrics[static_cast<size_t>(id)].strVal.c_str();
}

void RDDiagManager::SetFloat(RDMetricId id, double value)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	s_metrics[static_cast<size_t>(id)].floatVal = value;
}

void RDDiagManager::SetInt(RDMetricId id, int64_t value)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	s_metrics[static_cast<size_t>(id)].intVal = value;
}

void RDDiagManager::SetString(RDMetricId id, const char* value)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	s_metrics[static_cast<size_t>(id)].strVal = value ? value : "";
}

void RDDiagManager::RegisterStartupComponent(IRDStartupComponent* pComponent)
{
	s_startupComponents.push_back(pComponent);
}

void RDDiagManager::RegisterIntervalPollingComponent(IRDIntervalPollingComponent* pComponent)
{
	IntervalEntry entry;
	entry.pComponent = pComponent;
	entry.lastCollected.QuadPart = 0;
	s_intervalComponents.push_back(entry);
}

void RDDiagManager::RegisterFrameComponent(IRDFrameComponent* pComponent)
{
	s_frameComponents.push_back(pComponent);
}

void RDDiagManager::GpuTimestampBeginFrame()
{
	s_gpuTimestamp.BeginFrame();
}

void RDDiagManager::GpuTimestampEndFrame()
{
	s_gpuTimestamp.EndFrame();
}

void RDDiagManager::ResetFrameMetrics()
{
	static const RDMetricId appMetrics[] = {
		RDMetricId::AppFrameTimeMs,
		RDMetricId::AppSceneUpdateTimeMs,
		RDMetricId::AppDrawTimeMs,
		RDMetricId::AppPresentTimeMs,
		RDMetricId::AppGpuRenderTimeMs,
		RDMetricId::AppAvgFrameTimeMs,
		RDMetricId::AppFps,
		RDMetricId::AppFps1PercentLow,
		RDMetricId::AppFps01PercentLow,
		RDMetricId::AppFrameTimeStdDev,
		RDMetricId::AppStutterPercent,
		RDMetricId::AppNps,
	};
	for (auto id : appMetrics) {
		s_metrics[static_cast<size_t>(id)].floatVal = 0.0;
	}

	static const RDMetricId appIntMetrics[] = {
		RDMetricId::AppNoteActivationsPerFrame,
		RDMetricId::AppNoteTracking,
		RDMetricId::AppNoteTrackingPeak,
		RDMetricId::AppInstanceCount,
		RDMetricId::AppInstanceBufferSizeKB,
	};
	for (auto id : appIntMetrics) {
		s_metrics[static_cast<size_t>(id)].intVal = 0;
	}

	for (auto* pComp : s_frameComponents) {
		pComp->Reset();
	}
}

void RDDiagManager::SetLogIntervalMs(DWORD intervalMs)
{
	s_logIntervalMs = intervalMs;
}

void RDDiagManager::LogEvent(
	const RDFormatTemplateEntry* pProfile, size_t count,
	const char* eventTag)
{
	auto logger = spdlog::get("RD");
	if (!logger) return;

	auto entries = Format(pProfile, count);
	for (const auto& entry : entries) {
		logger->debug("[{}] {}: {}", eventTag, entry.label, entry.value);
	}
}

std::vector<RDFormattedEntry> RDDiagManager::Format(
	const RDFormatTemplateEntry* pProfile, size_t count)
{
	auto it = s_compiledProfiles.find(pProfile);
	if (it == s_compiledProfiles.end()) {
		std::vector<CompiledTemplate> compiled;
		compiled.resize(count);
		for (size_t i = 0; i < count; ++i) {
			compiled[i].label = pProfile[i].label;
			_CompileTemplate(pProfile[i].label, compiled[i].labelSegments);
			_CompileTemplate(pProfile[i].templateStr, compiled[i].segments);
		}
		it = s_compiledProfiles.emplace(pProfile, std::move(compiled)).first;
	}

	const auto& compiled = it->second;
	s_formatBuffer.clear();
	s_formatBuffer.reserve(compiled.size() * 2);

	std::vector<RDFormattedEntry> result;
	result.reserve(compiled.size());

	for (const auto& tmpl : compiled) {
		if (!tmpl.labelSegments.empty()) {
			s_formatBuffer.push_back(_FormatCompiledTemplate(tmpl.labelSegments));
		} else {
			s_formatBuffer.push_back(tmpl.label);
		}
		const char* fmtLabel = s_formatBuffer.back().c_str();

		s_formatBuffer.push_back(_FormatCompiledTemplate(tmpl.segments));
		RDFormattedEntry entry;
		entry.label = fmtLabel;
		entry.value = s_formatBuffer.back().c_str();
		result.push_back(entry);
	}

	return result;
}

void RDDiagManager::_BuildKeyToIdMap()
{
	const RDMetricMeta* table = RDGetMetricMetaTable();
	size_t count = RDGetMetricMetaCount();
	for (size_t i = 0; i < count; ++i) {
		s_keyToIdMap[table[i].key] = static_cast<size_t>(table[i].id);
	}
}

void RDDiagManager::_CompileTemplate(
	const char* templateStr,
	std::vector<CompiledSegment>& outSegments)
{
	outSegments.clear();
	const char* p = templateStr;
	std::string literal;

	const RDMetricMeta* metaTable = RDGetMetricMetaTable();

	while (*p) {
		if (p[0] == '$' && p[1] == '{') {
			if (!literal.empty()) {
				CompiledSegment seg;
				seg.kind = CompiledSegment::Literal;
				seg.literal = std::move(literal);
				seg.metricId = RDMetricId::COUNT;
				seg.metricType = RDMetricType::Int;
				outSegments.push_back(std::move(seg));
				literal.clear();
			}

			p += 2;
			const char* keyStart = p;
			while (*p && *p != '}') ++p;

			std::string key(keyStart, p);
			if (*p == '}') ++p;

			auto it = s_keyToIdMap.find(key);
			if (it != s_keyToIdMap.end()) {
				CompiledSegment seg;
				seg.kind = CompiledSegment::Metric;
				seg.metricId = static_cast<RDMetricId>(it->second);
				seg.metricType = metaTable[it->second].type;
				outSegments.push_back(std::move(seg));
			} else {
				CompiledSegment seg;
				seg.kind = CompiledSegment::Literal;
				seg.literal = "${" + key + "}";
				seg.metricId = RDMetricId::COUNT;
				seg.metricType = RDMetricType::Int;
				outSegments.push_back(std::move(seg));

				auto logger = spdlog::get("RD");
				if (logger) {
					logger->warn("Unknown metric key in template: {}", key);
				}
			}
		} else {
			literal += *p;
			++p;
		}
	}

	if (!literal.empty()) {
		CompiledSegment seg;
		seg.kind = CompiledSegment::Literal;
		seg.literal = std::move(literal);
		seg.metricId = RDMetricId::COUNT;
		seg.metricType = RDMetricType::Int;
		outSegments.push_back(std::move(seg));
	}
}

static std::string _FormatIntWithCommas(int64_t value)
{
	if (value < 0) return "-" + _FormatIntWithCommas(-value);
	std::string s = std::to_string(value);
	int pos = static_cast<int>(s.length()) - 3;
	while (pos > 0) {
		s.insert(pos, ",");
		pos -= 3;
	}
	return s;
}

std::string RDDiagManager::_FormatCompiledTemplate(
	const std::vector<CompiledSegment>& segments)
{
	std::string result;
	for (const auto& seg : segments) {
		if (seg.kind == CompiledSegment::Literal) {
			result += seg.literal;
		} else {
			size_t idx = static_cast<size_t>(seg.metricId);
			switch (seg.metricType) {
			case RDMetricType::Int:
				result += _FormatIntWithCommas(s_metrics[idx].intVal);
				break;
			case RDMetricType::Float:
				{
					char buf[32];
					snprintf(buf, sizeof(buf), "%.2f", s_metrics[idx].floatVal);
					result += buf;
				}
				break;
			case RDMetricType::String:
				result += s_metrics[idx].strVal;
				break;
			}
		}
	}
	return result;
}
