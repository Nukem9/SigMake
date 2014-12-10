#pragma once

#include <windows.h>
#include <stdio.h>
#include <vector>
#include <stdint.h>

//
// DISTORM
//
extern "C"
{
#include "distorm/distorm.h"
#include "distorm/mnemonics.h"
#include "distorm/instructions.h"
#include "distorm/prefix.h"
}

//
// YARA
//
extern "C"
{
#include <yara.h>
}

//
// X64DBG
//
#define PLUGIN_NAME		"SigMake"
#define PLUGIN_VERSION	001

#ifdef _WIN64
#pragma comment(lib, "pluginsdk/x64_dbg.lib")
#pragma comment(lib, "pluginsdk/x64_bridge.lib")
#pragma comment(lib, "pluginsdk/TitanEngine/TitanEngine_x64.lib")
#pragma comment(lib, "pluginsdk/dbghelp/dbghelp_x64.lib")
#else
#pragma comment(lib, "pluginsdk/x32_dbg.lib")
#pragma comment(lib, "pluginsdk/x32_bridge.lib")
#pragma comment(lib, "pluginsdk/TitanEngine/TitanEngine_x86.lib")
#pragma comment(lib, "pluginsdk/dbghelp/dbghelp_x86.lib")
#endif

#define _plugin_printf(Format, ...) _plugin_logprintf("[" PLUGIN_NAME "] " Format, __VA_ARGS__)

#include "pluginsdk/_plugins.h"
#include "pluginsdk/bridgemain.h"
#include "pluginsdk/_dbgfunctions.h"
#include "pluginsdk/TitanEngine/TitanEngine.h"


//
// PLUGIN
//
#define CLOSE_WINDOW(handle, global) { (global) = nullptr; DestroyWindow((handle)); }

#include "resource.h"
#include "Plugin.h"
#include "Descriptor.h"
#include "SigMake.h"
#include "Yara.h"
#include "Dialog/SigMakeDialog.h"
#include "Dialog/Settings.h"
#include "Dialog/SettingsDialog.h"
