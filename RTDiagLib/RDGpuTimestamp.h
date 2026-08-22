#pragma once

#include "RTDiagLib.h"
#include <d3d11.h>

class RTDIAGLIB_API RDGpuTimestamp
{
public:
	RDGpuTimestamp();
	~RDGpuTimestamp();

	int Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Terminate();

	void BeginFrame();
	void EndFrame();

	bool IsAvailable() const { return m_isAvailable; }

private:
	static const int FRAME_COUNT = 2;

	ID3D11DeviceContext* m_pContext;
	ID3D11Query* m_pDisjoint[FRAME_COUNT];
	ID3D11Query* m_pTimestampBegin[FRAME_COUNT];
	ID3D11Query* m_pTimestampEnd[FRAME_COUNT];

	int m_currentFrame;
	int m_framesSinceInit;
	bool m_isAvailable;
};
