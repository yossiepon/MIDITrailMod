//******************************************************************************
//
// MIDITrail / MTFormatUtil
//
// String formatting utilities.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <cstdio>
#include <cstring>


//******************************************************************************
// Format unsigned long with comma separators (e.g. 1234567 -> "1,234,567")
//******************************************************************************
static inline void MTFormatWithCommas(
		char* buf,
		size_t bufSize,
		unsigned long value
	)
{
	char raw[32];
	snprintf(raw, sizeof(raw), "%lu", value);
	int len = (int)strlen(raw);
	int commas = (len - 1) / 3;
	int total = len + commas;
	if (total < 0 || (size_t)total >= bufSize) {
		snprintf(buf, bufSize, "%lu", value);
		return;
	}
	buf[total] = '\0';
	int src = len - 1, dst = total - 1, cnt = 0;
	while (src >= 0) {
		buf[dst--] = raw[src--];
		if (++cnt == 3 && src >= 0) {
			buf[dst--] = ',';
			cnt = 0;
		}
	}
}
