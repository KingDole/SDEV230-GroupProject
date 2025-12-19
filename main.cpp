#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <commdlg.h>
#include "DataTable.h"
#include "SpreadsheetStorage.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")

// Control IDs
#define ID_BTN_ADD 2001
#define ID_BTN_DELETE 2002
#define ID_BTN_EDIT 2003
#define ID_BTN_SAVE  2005
#define ID_BTN_LOAD  2006
#define ID_BTN_SUMMARY 2004
#define ID_STATIC_SUMMARY 3001

// Dialog control IDs
#define IDC_EDIT_CATEGORY 4001
#define IDC_EDIT_ITEM 4002
#define IDC_EDIT_MATERIAL 4003
#define IDC_EDIT_DESCRIPTION 4004
#define IDC_EDIT_QUANTITY 4005
#define IDC_EDIT_UNITCOST 4006
#define IDC_EDIT_NOTES 4007
#define IDC_BTN_OK 4008
#define IDC_BTN_CANCEL 4009

// Global variables
DataTable* g_dataTable = nullptr;
HWND g_hBtnAdd = NULL;
HWND g_hBtnDelete = NULL;
HWND g_hBtnEdit = NULL;
HWND g_hBtnSave = NULL;
HWND g_hBtnLoad = NULL;
HWND g_hBtnSummary = NULL;
HWND g_hStaticSummary = NULL;

// Dialog data
DataRow g_dialogData;
bool g_dialogResult = false;

// Layout constants
const int MARGIN = 10;
const int BUTTON_HEIGHT = 30;
const int BUTTON_WIDTH = 120;
const int SUMMARY_HEIGHT = 60;
const int BUTTON_SPACING = 10;

// --- Helper: dialogue box for saving ---
bool ShowSaveCSVDialog(HWND hwnd, std::wstring& outPath)
{
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = hwnd;
    ofn.lpstrFilter =
        L"CSV Files (*.csv)\0*.csv\0"
        L"All Files (*.*)\0*.*\0";
    ofn.lpstrFile   = fileName;
    ofn.nMaxFile    = MAX_PATH;
    ofn.lpstrDefExt = L"csv";
    ofn.Flags       = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        outPath = fileName;
        return true;
    }
    return false;
}

// --- Helper: dialogue box for loading ---
bool ShowOpenCSVDialog(HWND hwnd, std::wstring& outPath)
{
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"csv";

    if (GetOpenFileName(&ofn))
    {
        outPath = fileName;
        return true;
    }

    return false;
}

// --- Helper: calculate total cost ---
std::wstring CalculateCost(const std::wstring& quantity, const std::wstring& unitCost) {
    try {
        std::wstring ucStr = unitCost;
        ucStr.erase(std::remove(ucStr.begin(), ucStr.end(), L'$'), ucStr.end());
        ucStr.erase(std::remove(ucStr.begin(), ucStr.end(), L','), ucStr.end());

        double qty = std::stod(quantity);
        double uc = std::stod(ucStr);
        double total = qty * uc;

        std::wostringstream oss;
        oss << L"$" << std::fixed << std::setprecision(2) << total;
        return oss.str();
    } catch (...) {
        return L"$0.00";
    }
}

// --- Dialog Window Procedure ---
LRESULT CALLBACK DialogWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BTN_OK: {
                    wchar_t buffer[256];

                    GetDlgItemText(hwnd, IDC_EDIT_CATEGORY, buffer, 256);
                    g_dialogData.category = buffer;

                    GetDlgItemText(hwnd, IDC_EDIT_ITEM, buffer, 256);
                    g_dialogData.item = buffer;

                    GetDlgItemText(hwnd, IDC_EDIT_MATERIAL, buffer, 256);
                    g_dialogData.material = buffer;

                    GetDlgItemText(hwnd, IDC_EDIT_DESCRIPTION, buffer, 256);
                    g_dialogData.description = buffer;

                    GetDlgItemText(hwnd, IDC_EDIT_QUANTITY, buffer, 256);
                    g_dialogData.quantity = buffer;

                    GetDlgItemText(hwnd, IDC_EDIT_UNITCOST, buffer, 256);
                    g_dialogData.unitCost = buffer;

                    GetDlgItemText(hwnd, IDC_EDIT_NOTES, buffer, 256);
                    g_dialogData.notes = buffer;

                    g_dialogData.cost = CalculateCost(g_dialogData.quantity, g_dialogData.unitCost);

                    g_dialogResult = true;
                    DestroyWindow(hwnd);
                    return 0;
                }

                case IDC_BTN_CANCEL: {
                    g_dialogResult = false;
                    DestroyWindow(hwnd);
                    return 0;
                }
            }
            break;
        }

        case WM_CLOSE:
            g_dialogResult = false;
            DestroyWindow(hwnd);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// --- Create entry dialog programmatically ---
HWND CreateEntryDialog(HWND hwndParent, bool isEdit) {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DialogWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"EntryDialogClass";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        classRegistered = true;
    }

    HWND hwndDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"EntryDialogClass",
        isEdit ? L"Edit Entry" : L"Add New Entry",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );

    int yPos = 20;
    int labelWidth = 100;
    int editWidth = 350;
    int rowHeight = 35;
    int xLabel = 20;
    int xEdit = xLabel + labelWidth + 10;

    const wchar_t* labels[] = {
        L"Category:", L"Item:", L"Material:", L"Description:",
        L"Quantity:", L"Unit Cost:", L"Notes:"
    };
    int editIDs[] = {
        IDC_EDIT_CATEGORY, IDC_EDIT_ITEM, IDC_EDIT_MATERIAL,
        IDC_EDIT_DESCRIPTION, IDC_EDIT_QUANTITY, IDC_EDIT_UNITCOST,
        IDC_EDIT_NOTES
    };

    for (int i = 0; i < 7; ++i) {
        CreateWindow(L"STATIC", labels[i],
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            xLabel, yPos + 3, labelWidth, 20,
            hwndDlg, NULL, GetModuleHandle(NULL), NULL);

        CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            xEdit, yPos, editWidth, 25,
            hwndDlg, (HMENU)(LONG_PTR)editIDs[i], GetModuleHandle(NULL), NULL);

        yPos += rowHeight;
    }

    CreateWindow(L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        150, yPos + 20, 80, 30,
        hwndDlg, (HMENU)IDC_BTN_OK, GetModuleHandle(NULL), NULL);

    CreateWindow(L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        250, yPos + 20, 80, 30,
        hwndDlg, (HMENU)IDC_BTN_CANCEL, GetModuleHandle(NULL), NULL);

    // Populate fields if editing
    if (isEdit) {
        SetDlgItemText(hwndDlg, IDC_EDIT_CATEGORY, g_dialogData.category.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_ITEM, g_dialogData.item.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_MATERIAL, g_dialogData.material.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_DESCRIPTION, g_dialogData.description.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_QUANTITY, g_dialogData.quantity.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_UNITCOST, g_dialogData.unitCost.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_NOTES, g_dialogData.notes.c_str());
    }

    return hwndDlg;
}

// --- Show entry dialog ---
bool ShowEntryDialog(HWND hwndParent, DataRow& data, bool isEdit) {
    g_dialogData = data;
    g_dialogResult = false;

    HWND hwndDlg = CreateEntryDialog(hwndParent, isEdit);
    EnableWindow(hwndParent, FALSE);

    MSG msg;
    while (IsWindow(hwndDlg) && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    EnableWindow(hwndParent, TRUE);
    SetForegroundWindow(hwndParent);

    if (g_dialogResult) data = g_dialogData;
    return g_dialogResult;
}

// --- Update summary ---
void UpdateSummary() {
    if (!g_dataTable || !g_hStaticSummary) return;

    int rowCount = g_dataTable->GetRowCount();
    double totalCost = g_dataTable->CalculateTotalCost();

    std::wostringstream oss;
    oss << L"Total Entries: " << rowCount
        << L"     |     Total Cost: $" << std::fixed << std::setprecision(2) << totalCost;

    SetWindowText(g_hStaticSummary, oss.str().c_str());
}

// --- Update layout ---
void UpdateLayout(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int clientWidth = rc.right - rc.left;
    int clientHeight = rc.bottom - rc.top;

    int listViewHeight = clientHeight - (MARGIN * 3) - BUTTON_HEIGHT - SUMMARY_HEIGHT;
    int buttonY = MARGIN + listViewHeight + MARGIN;
    int summaryY = buttonY + BUTTON_HEIGHT + MARGIN;

    if (g_dataTable)
        SetWindowPos(g_dataTable->GetHandle(), NULL, MARGIN, MARGIN,
                     clientWidth - 2 * MARGIN, listViewHeight, SWP_NOZORDER);

    int buttonX = MARGIN;
    if (g_hBtnAdd) SetWindowPos(g_hBtnAdd, NULL, buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
    buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    if (g_hBtnDelete) SetWindowPos(g_hBtnDelete, NULL, buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
    buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    if (g_hBtnEdit) SetWindowPos(g_hBtnEdit, NULL, buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
    buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    if (g_hBtnSummary) SetWindowPos(g_hBtnSummary, NULL, buttonX, buttonY, BUTTON_WIDTH + 20, BUTTON_HEIGHT, SWP_NOZORDER);

    int rightX = clientWidth - MARGIN;

    if (g_hBtnLoad) {
        rightX -= BUTTON_WIDTH;
        SetWindowPos(g_hBtnLoad, NULL, rightX, buttonY,
            BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
        rightX -= BUTTON_SPACING;
    }

    if (g_hBtnSave) {
        rightX -= BUTTON_WIDTH;
        SetWindowPos(g_hBtnSave, NULL, rightX, buttonY,
            BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
    }

    if (g_hStaticSummary) SetWindowPos(g_hStaticSummary, NULL, MARGIN, summaryY, clientWidth - 2 * MARGIN, SUMMARY_HEIGHT, SWP_NOZORDER);
}

// --- Main Window Procedure ---
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex = {};
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_LISTVIEW_CLASSES;
            InitCommonControlsEx(&icex);

            g_dataTable = new DataTable(hwnd, 0, 0, 100, 100);

            g_dataTable->AddRow({L"Electronics", L"Laptop", L"Aluminum", L"15-inch display", L"5", L"$899.99", L"$4499.95", L"Bulk order discount"});
            g_dataTable->AddRow({L"Office", L"Desk Chair", L"Mesh/Steel", L"Ergonomic office chair", L"10", L"$249.50", L"$2495.00", L"Free shipping"});
            g_dataTable->AddRow({L"Supplies", L"Paper Reams", L"Paper", L"500 sheets per ream", L"50", L"$4.99", L"$249.50", L"Recycled paper"});

            g_hBtnAdd = CreateWindowW(L"BUTTON", L"Add Entry", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)ID_BTN_ADD, GetModuleHandle(NULL), NULL);
            g_hBtnDelete = CreateWindowW(L"BUTTON", L"Delete Entry", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)ID_BTN_DELETE, GetModuleHandle(NULL), NULL);
            g_hBtnEdit = CreateWindowW(L"BUTTON", L"Edit Entry", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)ID_BTN_EDIT, GetModuleHandle(NULL), NULL);
            g_hBtnSummary = CreateWindowW(L"BUTTON", L"Calculate Summary", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)ID_BTN_SUMMARY, GetModuleHandle(NULL), NULL);
            g_hBtnSave = CreateWindowW(L"BUTTON", L"Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)ID_BTN_SAVE, GetModuleHandle(NULL), NULL);
            g_hBtnLoad = CreateWindowW(L"BUTTON", L"Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)ID_BTN_LOAD, GetModuleHandle(NULL), NULL);

            g_hStaticSummary = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, 0, 0, 100, 50, hwnd, (HMENU)ID_STATIC_SUMMARY, GetModuleHandle(NULL), NULL);

            UpdateLayout(hwnd);
            UpdateSummary();
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_BTN_ADD: {
                    DataRow newRow = {L"", L"", L"", L"", L"1", L"$0.00", L"$0.00", L""};
                    if (ShowEntryDialog(hwnd, newRow, false)) {
                        g_dataTable->AddRow(newRow);
                        UpdateSummary();
                    }
                    break;
                }

                case ID_BTN_DELETE: {
                    DataRow selectedRow;
                    if (g_dataTable->GetSelectedRow(selectedRow)) {
                        if (MessageBox(hwnd, L"Are you sure you want to delete the selected entry?", L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                            g_dataTable->DeleteSelectedRow();
                            UpdateSummary();
                        }
                    } else {
                        MessageBox(hwnd, L"Please select an entry to delete!", L"No Selection", MB_OK | MB_ICONWARNING);
                    }
                    break;
                }

                case ID_BTN_EDIT: {
                    int selectedIndex = g_dataTable->GetSelectedIndex();
                    DataRow selectedRow;
                    if (g_dataTable->GetSelectedRow(selectedRow)) {
                        if (ShowEntryDialog(hwnd, selectedRow, true)) {
                            g_dataTable->UpdateRow(selectedIndex, selectedRow);
                            UpdateSummary();
                        }
                    } else {
                        MessageBox(hwnd, L"Please select an entry to edit!", L"No Selection", MB_OK | MB_ICONWARNING);
                    }
                    break;
                }

                case ID_BTN_SUMMARY: {
                    UpdateSummary();
                    int rowCount = g_dataTable->GetRowCount();
                    double totalCost = g_dataTable->CalculateTotalCost();

                    std::wostringstream oss;
                    oss << L"Summary Report\n\nTotal Entries: " << rowCount
                        << L"\nTotal Cost: $" << std::fixed << std::setprecision(2) << totalCost
                        << L"\nAverage Cost per Entry: $" << (rowCount > 0 ? totalCost / rowCount : 0.0);

                    MessageBox(hwnd, oss.str().c_str(), L"Cost Summary", MB_OK | MB_ICONINFORMATION);
                    break;
                }

                case ID_BTN_SAVE: {
                    std::wstring filePath;

                    if (ShowSaveCSVDialog(hwnd, filePath))
                    {
                        if (SpreadsheetStorage::SaveToCSV(filePath, g_dataTable->GetAllRows()))
                        {
                            MessageBox(hwnd, L"File saved successfully.",
                                    L"Saved", MB_OK | MB_ICONINFORMATION);
                        }
                        else {
                            MessageBox(hwnd, L"Failed to save file.",
                                    L"Error", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                }


                case ID_BTN_LOAD: {
                    std::wstring filePath;

                    if (!ShowOpenCSVDialog(hwnd, filePath))
                        break;  // User cancelled

                    std::vector<DataRow> rows;

                    if (SpreadsheetStorage::LoadFromCSV(filePath, rows))
                    {
                        g_dataTable->Clear();

                        for (const auto& r : rows)
                            g_dataTable->AddRow(r);

                        InvalidateRect(g_dataTable->GetHandle(), NULL, TRUE);
                        UpdateWindow(g_dataTable->GetHandle());
                    }
                    else {
                        MessageBox(hwnd, L"Load failed.", L"Error", MB_OK | MB_ICONERROR);
                    }
                    break;
                }

            }
            return 0;
        }

        case WM_SIZE:
            UpdateLayout(hwnd);
            return 0;

        case WM_DESTROY:
            delete g_dataTable;
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// --- WinMain ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"DataTableWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Cost Tracker - Spreadsheet",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 950, 550,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
