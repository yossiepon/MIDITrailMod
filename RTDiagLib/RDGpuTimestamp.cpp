#include "stdafx.h"
#include "RDGpuTimestamp.h"
#include "RDDiagManager.h"
#include <spdlog/spdlog.h>

RDGpuTimestamp::RDGpuTimestamp()
	: m_pContext(nullptr)
	, m_currentFrame(0)
	, m_framesSinceInit(0)
	, m_isAvailable(false)
{
	for (int i = 0; i < FRAME_COUNT; i++) {
		m_pDisjoint[i] = nullptr;
		m_pTimestampBegin[i] = nullptr;
		m_pTimestampEnd[i] = nullptr;
	}
}

RDGpuTimestamp::~RDGpuTimestamp()
{
	Terminate();
}

int RDGpuTimestamp::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (pDevice == nullptr || pContext == nullptr) {
		return 0;
	}

	m_pContext = pContext;

	D3D11_QUERY_DESC disjointDesc = {};
	disjointDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

	D3D11_QUERY_DESC timestampDesc = {};
	timestampDesc.Query = D3D11_QUERY_TIMESTAMP;

	for (int i = 0; i < FRAME_COUNT; i++) {
		HRESULT hr = pDevice->CreateQuery(&disjointDesc, &m_pDisjoint[i]);
		if (FAILED(hr)) goto FAIL;

		hr = pDevice->CreateQuery(&timestampDesc, &m_pTimestampBegin[i]);
		if (FAILED(hr)) goto FAIL;

		hr = pDevice->CreateQuery(&timestampDesc, &m_pTimestampEnd[i]);
		if (FAILED(hr)) goto FAIL;
	}

	m_isAvailable = true;
	m_currentFrame = 0;
	m_framesSinceInit = 0;

	{
		auto logger = spdlog::get("RD");
		if (logger) {
			logger->debug("GPU Timestamp Query initialized");
		}
	}

	return 0;

FAIL:
	{
		auto logger = spdlog::get("RD");
		if (logger) {
			logger->warn("GPU Timestamp Query not supported, falling back to QPC only");
		}
	}
	Terminate();
	return 0;
}

void RDGpuTimestamp::Terminate()
{
	for (int i = 0; i < FRAME_COUNT; i++) {
		if (m_pDisjoint[i]) { m_pDisjoint[i]->Release(); m_pDisjoint[i] = nullptr; }
		if (m_pTimestampBegin[i]) { m_pTimestampBegin[i]->Release(); m_pTimestampBegin[i] = nullptr; }
		if (m_pTimestampEnd[i]) { m_pTimestampEnd[i]->Release(); m_pTimestampEnd[i] = nullptr; }
	}
	m_pContext = nullptr;
	m_isAvailable = false;
}

void RDGpuTimestamp::BeginFrame()
{
	if (!m_isAvailable) return;

	int readFrame = 1 - m_currentFrame;

	if (m_framesSinceInit >= FRAME_COUNT) {
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData = {};
		HRESULT hr = m_pContext->GetData(
			m_pDisjoint[readFrame], &disjointData, sizeof(disjointData), 0);

		if (hr == S_OK && !disjointData.Disjoint && disjointData.Frequency > 0) {
			UINT64 tsBegin = 0, tsEnd = 0;
			HRESULT hrBegin = m_pContext->GetData(
				m_pTimestampBegin[readFrame], &tsBegin, sizeof(tsBegin), 0);
			HRESULT hrEnd = m_pContext->GetData(
				m_pTimestampEnd[readFrame], &tsEnd, sizeof(tsEnd), 0);

			if (hrBegin == S_OK && hrEnd == S_OK && tsEnd >= tsBegin) {
				double gpuTimeMs = static_cast<double>(tsEnd - tsBegin)
					/ static_cast<double>(disjointData.Frequency) * 1000.0;
				RDDiagManager::SetFloat(RDMetricId::AppGpuRenderTimeMs, gpuTimeMs);
			}
		}
	}

	m_pContext->Begin(m_pDisjoint[m_currentFrame]);
	m_pContext->End(m_pTimestampBegin[m_currentFrame]);
}

void RDGpuTimestamp::EndFrame()
{
	if (!m_isAvailable) return;

	m_pContext->End(m_pTimestampEnd[m_currentFrame]);
	m_pContext->End(m_pDisjoint[m_currentFrame]);

	m_currentFrame = 1 - m_currentFrame;
	m_framesSinceInit++;
}
