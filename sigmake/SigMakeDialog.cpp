#include "stdafx.h"

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

	size_t dataLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT1)) + 1;
	size_t maskLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SIGMAKE_EDIT2)) + 1;

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
	}
	else if (To == SIG_IDA && From == SIG_CODE)
	{
		SIG_DESCRIPTOR *desc = DescriptorFromCode(data, mask);

		//
		// Buffer is reallocated
		//
		BridgeFree(data);

		DescriptorToIDA(desc, &data);

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
		EndDialog(hwndDlg, NULL);
		return TRUE;
	}
	break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_SIGMAKE_SCAN:
			// Scan for the signature
			break;

		case IDC_SIGMAKE_CANCEL:
			// Close
			EndDialog(hwndDlg, NULL);
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
	HWND wnd = CreateDialog(g_LocalDllHandle, MAKEINTRESOURCE(IDD_MAKESIG), GuiGetWindowHandle(), MakeSigDialogProc);

	if (!wnd)
	{
		_plugin_printf("Failed to create signature view window\n");
		return;
	}

	ShowWindow(wnd, SW_SHOW);
}