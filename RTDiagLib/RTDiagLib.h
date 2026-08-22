#pragma once

#ifdef RTDIAGLIB_EXPORTS
#define RTDIAGLIB_API __declspec(dllexport)
#else
#define RTDIAGLIB_API __declspec(dllimport)
#endif
