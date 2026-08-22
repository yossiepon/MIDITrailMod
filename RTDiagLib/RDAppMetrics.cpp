#include "stdafx.h"
#include "RDAppMetrics.h"
#include "RDDiagManager.h"
#include <algorithm>
#include <cmath>
#include <numeric>

static const float GAP_THRESHOLD_MS = 1000.0f;

RDAppMetrics::RDAppMetrics()
	: m_frameTimeRing(DEFAULT_RING_BUFFER_SIZE, 0.0f)
	, m_ringHead(0)
	, m_ringCount(0)
{
	m_sortBuffer.reserve(DEFAULT_RING_BUFFER_SIZE);
}

void RDAppMetrics::Reset()
{
	std::fill(m_frameTimeRing.begin(), m_frameTimeRing.end(), 0.0f);
	m_ringHead = 0;
	m_ringCount = 0;
	m_sortBuffer.clear();
}

void RDAppMetrics::CollectFrame()
{
	float frameTimeMs = static_cast<float>(RDDiagManager::GetFloat(RDMetricId::AppFrameTimeMs));

	if (frameTimeMs <= 0.0f || frameTimeMs > GAP_THRESHOLD_MS) {
		return;
	}

	m_frameTimeRing[m_ringHead] = frameTimeMs;
	m_ringHead = (m_ringHead + 1) % m_frameTimeRing.size();
	if (m_ringCount < m_frameTimeRing.size()) {
		m_ringCount++;
	}

	if (m_ringCount >= 30) {
		_ComputeStatistics();
	}
}

void RDAppMetrics::_ComputeStatistics()
{
	m_sortBuffer.clear();

	size_t start = (m_ringCount < m_frameTimeRing.size())
		? 0
		: m_ringHead;

	for (size_t i = 0; i < m_ringCount; i++) {
		size_t idx = (start + i) % m_frameTimeRing.size();
		m_sortBuffer.push_back(m_frameTimeRing[idx]);
	}

	size_t n = m_sortBuffer.size();

	// FPS: read directly from ring buffer (newest entries), independent of sort order
	{
		size_t fpsCount = (std::min)(m_ringCount, FPS_WINDOW_FRAMES);
		double fpsSum = 0.0;
		for (size_t i = 0; i < fpsCount; i++) {
			size_t idx = (m_ringHead + m_frameTimeRing.size() - 1 - i) % m_frameTimeRing.size();
			fpsSum += m_frameTimeRing[idx];
		}
		double fpsMean = fpsSum / fpsCount;
		RDDiagManager::SetFloat(RDMetricId::AppAvgFrameTimeMs, fpsMean);
		double avgFps = (fpsMean > 0.0) ? 1000.0 / fpsMean : 0.0;
		RDDiagManager::SetFloat(RDMetricId::AppFps, avgFps);
	}

	// Mean over full buffer (for StdDev and Stutter)
	double sum = 0.0;
	for (float ft : m_sortBuffer) {
		sum += ft;
	}
	double mean = sum / n;

	// Standard deviation (jitter)
	double sqSum = 0.0;
	for (float ft : m_sortBuffer) {
		double diff = ft - mean;
		sqSum += diff * diff;
	}
	double stdDev = std::sqrt(sqSum / n);
	RDDiagManager::SetFloat(RDMetricId::AppFrameTimeStdDev, stdDev);

	// Sort for percentile calculation (ascending = fastest frames first)
	std::sort(m_sortBuffer.begin(), m_sortBuffer.end());

	// 1% Low: average of the slowest 1% of frame times → convert to FPS
	{
		size_t count1pct = (std::max)(static_cast<size_t>(1), n / 100);
		double slowSum = 0.0;
		for (size_t i = n - count1pct; i < n; i++) {
			slowSum += m_sortBuffer[i];
		}
		double avgSlowest = slowSum / count1pct;
		double fps1pctLow = (avgSlowest > 0.0) ? 1000.0 / avgSlowest : 0.0;
		RDDiagManager::SetFloat(RDMetricId::AppFps1PercentLow, fps1pctLow);
	}

	// 0.1% Low: average of the slowest 0.1% of frame times → convert to FPS
	{
		size_t count01pct = (std::max)(static_cast<size_t>(1), n / 1000);
		double slowSum = 0.0;
		for (size_t i = n - count01pct; i < n; i++) {
			slowSum += m_sortBuffer[i];
		}
		double avgSlowest = slowSum / count01pct;
		double fps01pctLow = (avgSlowest > 0.0) ? 1000.0 / avgSlowest : 0.0;
		RDDiagManager::SetFloat(RDMetricId::AppFps01PercentLow, fps01pctLow);
	}

	// Stutter detection (CapFrameX method): frames exceeding 2.5x average
	{
		double threshold = mean * 2.5;
		size_t stutterCount = 0;
		for (float ft : m_sortBuffer) {
			if (ft > threshold) {
				stutterCount++;
			}
		}
		double stutterPct = static_cast<double>(stutterCount) / n * 100.0;
		RDDiagManager::SetFloat(RDMetricId::AppStutterPercent, stutterPct);
	}
}
