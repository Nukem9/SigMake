#include "stdafx.h"

bool g_YaraInitialized;

bool OpenSelectionDialog(const char *Title, const char *Filter, char *Buffer, size_t BufferSize)
{
	//
	// Get the currently selected module
	//
	duint moduleBase = DbgGetCurrentModule();

	if (moduleBase <= 0)
		return false;

	//
	// Open a file dialog to select the yara file
	//
	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));

	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner	= GuiGetWindowHandle();
	ofn.lpstrFilter	= Filter;
	ofn.lpstrFile	= Buffer;
	ofn.nMaxFile	= BufferSize;
	ofn.lpstrTitle	= Title;
	ofn.Flags		= OFN_FILEMUSTEXIST;

	if (!GetOpenFileNameA(&ofn))
		return false;

	return true;
}

void YaraOutputCallback(int ErrorLevel, const char *FileName, int LineNumber, const char *Message)
{
	_plugin_logprintf("%s(%d): %d: %s\n", FileName, LineNumber, ErrorLevel, Message);
}

int YaraScanCallback(int Message, void *MessageData, void *UserData)
{
	return CALLBACK_CONTINUE;
}

void OpenYaraDialog()
{
	//
	// Initialize Yara
	//
	if (!g_YaraInitialized)
	{
		if (yr_initialize() != ERROR_SUCCESS)
		{
			_plugin_logprintf("Failed to initialize Yara library\n");
			return;
		}

		g_YaraInitialized = true;
	}

	//
	// Create the code compiler
	//
	YR_COMPILER *compiler	= nullptr;
	YR_RULES *rules			= nullptr;

	if (yr_compiler_create(&compiler) != ERROR_SUCCESS)
	{
		_plugin_logprintf("Yara compiler initialization failed\n");
		return;
	}

	//
	// Set the compiler output callback
	//
	yr_compiler_set_callback(compiler, YaraOutputCallback);

	//
	// Open a selection dialog and get the user input file
	//
	char fileName[MAX_PATH];
	FILE *fileHandle = nullptr;

	{
		if (!OpenSelectionDialog("Open a Yara signature file", "Yara files (*.map)\0*.yara\0\0", fileName, ARRAYSIZE(fileName)))
			goto __cleanup;

		fopen_s(&fileHandle, fileName, "w");
	}

	if (yr_compiler_add_file(compiler, fileHandle, nullptr, strrchr(fileName, '\\') + 1) > 0)
	{
		_plugin_logprintf("Compilation errors found, exiting\n");
		goto __cleanup;
	}

	//
	// Allocate rules
	//
	yr_compiler_get_rules(compiler, &rules);

	//
	// Scan memory
	//
	duint dataVA	= 0;
	BYTE *data		= nullptr;
	size_t dataLen	= 0;

	if (yr_rules_scan_mem(rules, data, dataLen, 0, YaraScanCallback, (void *)dataVA, 0) != ERROR_SUCCESS)
	{
		_plugin_logprintf("An error occurred while scanning\n");
		goto __cleanup;
	}

__cleanup:
	if (compiler)
		yr_compiler_destroy(compiler);

	if (rules)
		yr_rules_destroy(rules);

	if (fileHandle)
		fclose(fileHandle);
}

void DestroyYaraDialog()
{
	if (g_YaraInitialized)
		yr_finalize();
}