//******************************************************************************
//
// MIDITrail / MTLogManager
//
// Log manager class.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTConfFile.h"
#include "MTLogManager.h"
#include "MTParam.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <shlobj.h>

using namespace YNBaseLib;

DWORD MTLogManager::s_runtimeLogIntervalMs = 10000;

//******************************************************************************
// Initialize
//******************************************************************************
int MTLogManager::Initialize(
		const WCHAR* pLogLevelOverride
	)
{
	int result = 0;
	WCHAR logDirPath[_MAX_PATH] = {0};
	WCHAR exeBaseName[_MAX_PATH] = {0};
	std::string logFileBasePath;
	MTConfFile confFile;
	std::string levelStr;
	int maxFiles = 7;
	bool msvcSinkEnabled = false;
	spdlog::level::level_enum defaultLevel = spdlog::level::warn;
	DWORD runtimeLogIntervalMs = 10000;
	int flushIntervalSec = 1;

#ifdef _DEBUG
	defaultLevel = spdlog::level::debug;
	msvcSinkEnabled = true;
#endif

	// exe base name for log file prefix
	result = _GetExeBaseName(exeBaseName, _MAX_PATH);
	if (result != 0) goto EXIT;

	// log output directory (primary -> fallback)
	result = _DetermineLogDir(logDirPath, _MAX_PATH);
	if (result != 0) goto EXIT;

	// build log file base path: "logs/{exename}"
	{
		char dirPathA[_MAX_PATH] = {0};
		char baseNameA[_MAX_PATH] = {0};
		WideCharToMultiByte(CP_UTF8, 0, logDirPath, -1, dirPathA, _MAX_PATH, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, exeBaseName, -1, baseNameA, _MAX_PATH, NULL, NULL);
		logFileBasePath = std::string(dirPathA) + baseNameA + ".log";
	}

	{
		int confResult = confFile.Initialize(_T("Logging"));
		if (confResult == 0) {
			TCHAR buf[256] = {0};

			confFile.SetCurSection(_T("Logging"));

			if (confFile.GetStr(_T("Level"), buf, 256, _T("")) == 0 && _tcslen(buf) > 0) {
				levelStr = buf;
			}

			int val = 0;
			if (confFile.GetInt(_T("MaxFiles"), &val, 7) == 0) {
				maxFiles = val;
			}
			if (confFile.GetInt(_T("MsvcSink"), &val, msvcSinkEnabled ? 1 : 0) == 0) {
				msvcSinkEnabled = (val != 0);
			}
			if (confFile.GetInt(_T("RuntimeLogInterval"), &val, 10000) == 0) {
				runtimeLogIntervalMs = static_cast<DWORD>(val);
			}
			if (confFile.GetInt(_T("FlushInterval"), &val, 1) == 0) {
				flushIntervalSec = val;
			}
		}
	}

	// apply config file level
	if (!levelStr.empty()) {
		defaultLevel = spdlog::level::from_str(levelStr);
	}

	// command-line override takes priority
	if (pLogLevelOverride != nullptr && wcslen(pLogLevelOverride) > 0) {
		char levelA[64] = {0};
		WideCharToMultiByte(CP_UTF8, 0, pLogLevelOverride, -1, levelA, 64, NULL, NULL);
		defaultLevel = spdlog::level::from_str(levelA);
	}

	// create sinks
	{
		try {
			auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
				logFileBasePath, 0, 0, false, static_cast<uint16_t>(maxFiles));

			std::vector<spdlog::sink_ptr> sinks = { file_sink };

			if (msvcSinkEnabled) {
				auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
				sinks.push_back(msvc_sink);
			}

			// create named loggers with shared sinks
			auto yn_logger = std::make_shared<spdlog::logger>("YN", sinks.begin(), sinks.end());
			auto sm_logger = std::make_shared<spdlog::logger>("SM", sinks.begin(), sinks.end());
			auto mt_logger = std::make_shared<spdlog::logger>("MT", sinks.begin(), sinks.end());
			auto rd_logger = std::make_shared<spdlog::logger>("RD", sinks.begin(), sinks.end());

			spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

			spdlog::register_logger(yn_logger);
			spdlog::register_logger(sm_logger);
			spdlog::register_logger(mt_logger);
			spdlog::register_logger(rd_logger);

			spdlog::set_default_logger(mt_logger);

			// startup log at info level (before applying configured level)
			mt_logger->set_level(spdlog::level::info);
			spdlog::info("=== MIDITrail Log Started ===");
			spdlog::info("Log level: {}", spdlog::level::to_string_view(defaultLevel));
			spdlog::info(L"Log directory: {}", logDirPath);
#ifdef _DEBUG
			spdlog::info("Build: Debug");
#else
			spdlog::info("Build: Release");
#endif
			mt_logger->flush();

			// flush on info level and above (crash-safe for important messages)
			yn_logger->flush_on(spdlog::level::info);
			sm_logger->flush_on(spdlog::level::info);
			mt_logger->flush_on(spdlog::level::info);
			rd_logger->flush_on(spdlog::level::info);

			// periodic flush for debug/trace messages
			if (flushIntervalSec > 0) {
				spdlog::flush_every(std::chrono::seconds(flushIntervalSec));
			}

			// apply configured level to all loggers
			yn_logger->set_level(defaultLevel);
			sm_logger->set_level(defaultLevel);
			mt_logger->set_level(defaultLevel);
			rd_logger->set_level(defaultLevel);

			s_runtimeLogIntervalMs = runtimeLogIntervalMs;
		}
		catch (const spdlog::spdlog_ex& ex) {
			// logging initialization failure is non-fatal
			(void)ex;
			goto EXIT;
		}
	}

EXIT:;
	return 0;
}

//******************************************************************************
// Terminate
//******************************************************************************
void MTLogManager::Terminate()
{
	static bool s_terminated = false;
	if (s_terminated) return;
	s_terminated = true;

	auto logger = spdlog::default_logger();
	if (logger) {
		logger->info("=== MIDITrail Log Ended ===");
		logger->flush();
	}
	spdlog::shutdown();
}

//******************************************************************************
// Determine log output directory
//******************************************************************************
int MTLogManager::_DetermineLogDir(
		WCHAR* pLogDirPath,
		unsigned long bufSize
	)
{
	int result = 0;
	TCHAR exeDirPathT[_MAX_PATH] = {0};
	WCHAR exeDirPath[_MAX_PATH] = {0};

	// primary: exe/logs/
	result = YNPathUtil::GetModuleDirPath(exeDirPathT, _MAX_PATH);
	if (result != 0) goto EXIT;

	// TCHAR -> WCHAR conversion (MultiByte mode: TCHAR=char)
#ifdef _UNICODE
	wcscpy_s(exeDirPath, _MAX_PATH, exeDirPathT);
#else
	MultiByteToWideChar(CP_ACP, 0, exeDirPathT, -1, exeDirPath, _MAX_PATH);
#endif

	wcscpy_s(pLogDirPath, bufSize, exeDirPath);
	wcscat_s(pLogDirPath, bufSize, L"logs\\");

	{
		int shr = SHCreateDirectoryExW(NULL, pLogDirPath, NULL);
		if (shr == ERROR_SUCCESS) {
			goto EXIT;
		}
		if (shr == ERROR_ALREADY_EXISTS || shr == ERROR_FILE_EXISTS) {
			DWORD attr = GetFileAttributesW(pLogDirPath);
			if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
				goto EXIT;
			}
		}
	}

	// fallback: %APPDATA%\yknk\MIDITrail\logs
	{
		TCHAR appDataPath[_MAX_PATH] = {0};
		result = YNPathUtil::GetAppDataDirPath(appDataPath, _MAX_PATH);
		if (result != 0) goto EXIT;

		WCHAR fallbackPath[_MAX_PATH] = {0};
#ifdef _UNICODE
		wcscpy_s(fallbackPath, _MAX_PATH, appDataPath);
#else
		MultiByteToWideChar(CP_ACP, 0, appDataPath, -1, fallbackPath, _MAX_PATH);
#endif
		wcscat_s(fallbackPath, _MAX_PATH, MT_USER_CONFFILE_DIR_W);
		wcscat_s(fallbackPath, _MAX_PATH, L"logs\\");

		int shr = SHCreateDirectoryExW(NULL, fallbackPath, NULL);
		if (shr == ERROR_SUCCESS || shr == ERROR_ALREADY_EXISTS || shr == ERROR_FILE_EXISTS) {
			wcscpy_s(pLogDirPath, bufSize, fallbackPath);
			goto EXIT;
		}
	}

	// both failed - use exe directory directly (no logs/ subdirectory)
	wcscpy_s(pLogDirPath, bufSize, exeDirPath);

EXIT:;
	return 0;
}

//******************************************************************************
// Get exe base name (without extension)
//******************************************************************************
int MTLogManager::_GetExeBaseName(
		WCHAR* pBaseName,
		unsigned long bufSize
	)
{
	WCHAR exePath[_MAX_PATH] = {0};
	WCHAR fname[_MAX_FNAME] = {0};

	GetModuleFileNameW(NULL, exePath, _MAX_PATH);
	_wsplitpath_s(exePath, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0);
	wcscpy_s(pBaseName, bufSize, fname);

	return 0;
}
