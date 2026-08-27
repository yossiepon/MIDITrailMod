//******************************************************************************
//
// RTDiagLib / RDAppMetrics
//
// Application frame metrics collector (FPS, NPS, frame timing).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "RDInterfaces.h"
#include <vector>
#include <cstdint>

class RDAppMetrics : public IRDFrameComponent
{
public:
	RDAppMetrics();
	virtual ~RDAppMetrics() = default;

	void CollectFrame() override;
	void Reset() override;

	static const size_t DEFAULT_RING_BUFFER_SIZE = 600;
	static const size_t FPS_WINDOW_FRAMES = 60;

private:
	void _ComputeStatistics();

	std::vector<float> m_frameTimeRing;
	size_t m_ringHead;
	size_t m_ringCount;

	std::vector<float> m_sortBuffer;

	std::vector<int> m_activationRing;
	size_t m_activationRingHead;
	size_t m_activationRingCount;
};
