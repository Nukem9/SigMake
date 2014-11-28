#include "stdafx.h"

HWND g_SigMakeDialog;

void MakeSigDialogInit(HWND hwndDlg)
{
	//
	// Get the debugger window's selection
	//
	SELECTIONDATA selection;

	if (!GuiSelectionGet(GUI_DISASSEMBLY, &selection))
		return;

	//
	// Generate the signature
	//
	SIG_DESCRIPTOR *desc = GenerateSigFromCode(selection.start, selection.end);

	if (!desc)
		return;

	//
	// SIG_DESCRIPTOR -> String
	//
	char *data = nullptr;
	char *mask = nullptr;

	switch (Settings::LastType)
	{
	case SIG_CODE:
		DescriptorToCode(desc, &data, &mask);
		break;

	case SIG_IDA:
		DescriptorToIDA(desc, &data);
		break;

	case SIG_CRC:
		//DescriptorToCRC(desc, )
		break;
	}

	//
	// Set the edit box text and clean up
	//
	if (data)
	{
		SetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1), data);
		BridgeFree(data);
	}

	if (mask)
	{
		SetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2), mask);
		BridgeFree(mask);
	}

	BridgeFree(desc);
}

void MakeSigDialogConvert(HWND hwndDlg, SIGNATURE_TYPE To, SIGNATURE_TYPE From)
{
	if (To == From)
		return;

	int dataLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1)) + 1;
	int maskLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2)) + 1;

	char *data = (char *)BridgeAlloc(dataLen);
	char *mask = (char *)BridgeAlloc(maskLen);

	GetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1), data, dataLen);
	GetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2), mask, maskLen);

	if (To == SIG_CODE && From == SIG_IDA)
	{
		SIG_DESCRIPTOR *desc = DescriptorFromIDA(data);

		//
		// Buffer is larger and needs reallocation
		//
		BridgeFree(data);
		BridgeFree(mask);
		DescriptorToCode(desc, &data, &mask);
		BridgeFree(desc);
	}
	else if (To == SIG_IDA && From == SIG_CODE)
	{
		SIG_DESCRIPTOR *desc = DescriptorFromCode(data, mask);

		//
		// Buffer is reallocated
		//
		BridgeFree(data);
		DescriptorToIDA(desc, &data);
		BridgeFree(desc);

		//
		// Zero out old data
		//
		memset(mask, 0, maskLen);
	}

	SetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1), data);
	SetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2), mask);

	BridgeFree(data);
	BridgeFree(mask);
}

void MakeSigDialogExecute(HWND hwndDlg)
{
	int dataLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1)) + 1;
	int maskLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2)) + 1;

	char *data = (char *)BridgeAlloc(dataLen);
	char *mask = (char *)BridgeAlloc(maskLen);

	GetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1), data, dataLen);
	GetWindowText(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2), mask, maskLen);

	std::vector<duint> results;
	SIG_DESCRIPTOR *desc = nullptr;

	//
	// Convert the string to a code descriptor
	//
	if (Settings::LastType == SIG_CODE)
		desc = DescriptorFromCode(data, mask);
	else if (Settings::LastType == SIG_IDA)
		desc = DescriptorFromIDA(data);
	else if (Settings::LastType == SIG_CRC)
		desc = DescriptorFromCRC(data);

	//
	// Scan
	//
	PatternScan(desc, results);

	for (auto& it : results)
		_plugin_printf("Result: 0x%X\n", it);

	//
	// Cleanup
	//
	BridgeFree(data);
	BridgeFree(mask);
	BridgeFree(desc);
}

INT_PTR CALLBACK MakeSigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		//
		// Check if the user has any code selected
		//
		MakeSigDialogInit(hwndDlg);

		//
		// TODO: CRC disabled until I can find a good piece of code for it
		//
		EnableWindow(GetDlgItem(hwndDlg, IDC_SIGMAKE_CRC), FALSE);

		// Update the initial signature type selection button
		switch (Settings::LastType)
		{
		case SIG_CODE:
			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_CODE), BM_SETCHECK, BST_CHECKED, 0);
			break;

		case SIG_IDA:
			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_IDA), BM_SETCHECK, BST_CHECKED, 0);
			break;

		case SIG_CRC:
			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_CRC), BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	}
	break;

	case WM_CLOSE:
	{
		CLOSE_WINDOW(hwndDlg, g_SigMakeDialog);
		return TRUE;
	}
	break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_SIGMAKE_SCAN:
			// Scan for the signature
			MakeSigDialogExecute(hwndDlg);

			// Close
			CLOSE_WINDOW(hwndDlg, g_SigMakeDialog);
			break;

		case IDC_SIGMAKE_CANCEL:
			// Close
			CLOSE_WINDOW(hwndDlg, g_SigMakeDialog);
			break;

		case IDC_SIGMAKE_CODE:
			// Convert sig
			MakeSigDialogConvert(hwndDlg, SIG_CODE, Settings::LastType);

			// Uncheck the other radio button and update last set variable
			Settings::LastType = SIG_CODE;

			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_IDA), BM_SETCHECK, BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_CRC), BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case IDC_SIGMAKE_IDA:
			// Convert sig
			MakeSigDialogConvert(hwndDlg, SIG_IDA, Settings::LastType);

			// Uncheck the other radio button and update last set variable
			Settings::LastType = SIG_IDA;

			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_CODE), BM_SETCHECK, BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_CRC), BM_SETCHECK, BST_UNCHECKED, 0);
			break;

		case IDC_SIGMAKE_CRC:
			// Convert sig
			MakeSigDialogConvert(hwndDlg, SIG_CRC, Settings::LastType);

			// Uncheck the other radio button and update last set variable
			Settings::LastType = SIG_CRC;

			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_CODE), BM_SETCHECK, BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SIGMAKE_IDA), BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		}
	}
	break;
	}

	return FALSE;
}

void OpenSigMakeDialog()
{
	g_SigMakeDialog = CreateDialog(g_LocalDllHandle, MAKEINTRESOURCE(IDD_MAKESIG), GuiGetWindowHandle(), MakeSigDialogProc);

	if (!g_SigMakeDialog)
	{
		_plugin_printf("Failed to create signature view window\n");
		return;
	}

	ShowWindow(g_SigMakeDialog, SW_SHOW);
}

void DestroySigMakeDialog()
{
	if (g_SigMakeDialog)
		SendMessage(g_SigMakeDialog, WM_CLOSE, 0, 0);
}