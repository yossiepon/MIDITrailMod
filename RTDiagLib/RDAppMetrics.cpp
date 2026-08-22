#include "stdafx.h"
#include "RDAppMetrics.h"

void RDAppMetrics::CollectFrame()
{
	// App metrics are set externally via RDDiagManager::SetFloat.
	// This component exists as a FrameComponent registration point
	// for future per-frame processing (e.g., statistics aggregation in Phase 1B).
}
