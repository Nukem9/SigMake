#include "stdafx.h"

int g_MenuHandle;
int g_PluginHandle;

HMODULE g_LocalDllHandle;

duint DbgGetCurrentModule()
{
	if (!DbgIsDebugging())
	{
		_plugin_printf("The debugger is not currently running!\n");
		return 0;
	}

	// First get the current code location
	SELECTIONDATA selection;

	if (!GuiSelectionGet(GUI_DISASSEMBLY, &selection))
	{
		_plugin_printf("GuiSelectionGet(GUI_DISASSEMBLY) failed\n");
		return 0;
	}

	// Convert the selected address to a module base
	duint moduleBase = DbgFunctions()->ModBaseFromAddr(selection.start);

	if (moduleBase <= 0)
	{
		_plugin_printf("Failed to resolve module base at address '0x%llX'\n", (ULONGLONG)selection.start);
		return 0;
	}

	return moduleBase;
}

void MenuEntryCallback(CBTYPE Type, PLUG_CB_MENUENTRY *Info)
{
	switch (Info->hEntry)
	{
	case PLUGIN_MENU_MAKESIG:
		OpenSigMakeDialog();
		break;

	case PLUGIN_MENU_SETTINGS:
		OpenSettingsDialog();
		break;

	case PLUGIN_MENU_ABOUT:
		MessageBoxA(GuiGetWindowHandle(), "Plugin created by Nukem.\n\nRequest features or view the source code at:\nhttps://bitbucket.org/Nukem9/XXXXXXX/", "About", 0);
		break;
	}
}

DLL_EXPORT bool pluginit(PLUG_INITSTRUCT *InitStruct)
{
	InitStruct->pluginVersion	= PLUGIN_VERSION;
	InitStruct->sdkVersion		= PLUG_SDKVERSION;
	g_PluginHandle				= InitStruct->pluginHandle;
	strcpy_s(InitStruct->pluginName, PLUGIN_NAME);

	// Add any of the callbacks
	_plugin_registercallback(g_PluginHandle, CB_MENUENTRY, (CBPLUGIN)MenuEntryCallback);

	// Update all checkbox settings
	Settings::InitIni();
	Settings::Load();
	return true;
}

DLL_EXPORT bool plugstop()
{
	// Close dialogs
	DestroySigMakeDialog();
	DestroySettingsDialog();

	// Clear the menu
	_plugin_menuclear(g_MenuHandle);

	// Remove callbacks
	_plugin_unregistercallback(g_PluginHandle, CB_MENUENTRY);
	return true;
}

DLL_EXPORT void plugsetup(PLUG_SETUPSTRUCT *SetupStruct)
{
	g_MenuHandle = SetupStruct->hMenu;

	// Initialize the menu
	_plugin_menuaddentry(g_MenuHandle, PLUGIN_MENU_MAKESIG, "&Create signature");
	_plugin_menuaddseparator(g_MenuHandle);
	_plugin_menuaddentry(g_MenuHandle, PLUGIN_MENU_SETTINGS, "&Settings");
	_plugin_menuaddentry(g_MenuHandle, PLUGIN_MENU_ABOUT, "&About");
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		g_LocalDllHandle = hinstDLL;

	return TRUE;
}