#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")

// Control IDs
#define ID_LISTVIEW 1001
#define ID_BTN_ADD 2001
#define ID_BTN_DELETE 2002
#define ID_BTN_EDIT 2003
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

// Structure to hold row data
struct DataRow {
    std::string category;
    std::string item;
    std::string material;
    std::string description;
    std::string quantity;
    std::string unitCost;
    std::string cost;
    std::string notes;
};

// Global variables for dialog
DataRow g_dialogData;
bool g_dialogResult = false;

// DataTable class
class DataTable {
private:
    HWND hListView;
    std::vector<DataRow> rows;

public:
    DataTable(HWND hwnd, int x, int y, int width, int height) {
        hListView = CreateWindowEx(0, WC_LISTVIEW, "",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS | WS_BORDER,
            x, y, width, height,
            hwnd, (HMENU)ID_LISTVIEW, GetModuleHandle(NULL), NULL);

        ListView_SetExtendedListViewStyle(hListView, 
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMN lvc = {};
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        const char* headers[] = {
            "Category", "Item", "Material", "Description",
            "Quantity", "Unit Cost", "Cost", "Notes"
        };
        int widths[] = { 100, 120, 100, 150, 80, 90, 90, 150 };

        for (int i = 0; i < 8; i++) {
            lvc.iSubItem = i;
            lvc.pszText = (LPSTR)headers[i];
            lvc.cx = widths[i];
            ListView_InsertColumn(hListView, i, &lvc);
        }
    }

    void AddRow(const DataRow& row) {
        rows.push_back(row);
        int index = rows.size() - 1;

        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = index;
        lvi.iSubItem = 0;
        lvi.pszText = (LPSTR)row.category.c_str();
        ListView_InsertItem(hListView, &lvi);

        ListView_SetItemText(hListView, index, 1, (LPSTR)row.item.c_str());
        ListView_SetItemText(hListView, index, 2, (LPSTR)row.material.c_str());
        ListView_SetItemText(hListView, index, 3, (LPSTR)row.description.c_str());
        ListView_SetItemText(hListView, index, 4, (LPSTR)row.quantity.c_str());
        ListView_SetItemText(hListView, index, 5, (LPSTR)row.unitCost.c_str());
        ListView_SetItemText(hListView, index, 6, (LPSTR)row.cost.c_str());
        ListView_SetItemText(hListView, index, 7, (LPSTR)row.notes.c_str());
    }

    void UpdateRow(int index, const DataRow& row) {
        if (index >= 0 && index < (int)rows.size()) {
            rows[index] = row;
            
            ListView_SetItemText(hListView, index, 0, (LPSTR)row.category.c_str());
            ListView_SetItemText(hListView, index, 1, (LPSTR)row.item.c_str());
            ListView_SetItemText(hListView, index, 2, (LPSTR)row.material.c_str());
            ListView_SetItemText(hListView, index, 3, (LPSTR)row.description.c_str());
            ListView_SetItemText(hListView, index, 4, (LPSTR)row.quantity.c_str());
            ListView_SetItemText(hListView, index, 5, (LPSTR)row.unitCost.c_str());
            ListView_SetItemText(hListView, index, 6, (LPSTR)row.cost.c_str());
            ListView_SetItemText(hListView, index, 7, (LPSTR)row.notes.c_str());
        }
    }

    void DeleteSelectedRow() {
        int index = GetSelectedIndex();
        if (index >= 0 && index < (int)rows.size()) {
            rows.erase(rows.begin() + index);
            ListView_DeleteItem(hListView, index);
        }
    }

    int GetSelectedIndex() {
        return ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    }

    bool GetSelectedRow(DataRow& outRow) {
        int index = GetSelectedIndex();
        if (index == -1 || index >= (int)rows.size()) {
            return false;
        }
        outRow = rows[index];
        return true;
    }

    double CalculateTotalCost() {
        double total = 0.0;
        for (const auto& row : rows) {
            std::string costStr = row.cost;
            costStr.erase(std::remove(costStr.begin(), costStr.end(), '$'), costStr.end());
            costStr.erase(std::remove(costStr.begin(), costStr.end(), ','), costStr.end());
            
            try {
                total += std::stod(costStr);
            } catch (...) {
                // Skip invalid values
            }
        }
        return total;
    }

    int GetRowCount() { return rows.size(); }
    HWND GetHandle() { return hListView; }
};

// Global variables
DataTable* g_dataTable = nullptr;
HWND g_hBtnAdd = NULL;
HWND g_hBtnDelete = NULL;
HWND g_hBtnEdit = NULL;
HWND g_hBtnSummary = NULL;
HWND g_hStaticSummary = NULL;

// Layout constants
const int MARGIN = 10;
const int BUTTON_HEIGHT = 30;
const int BUTTON_WIDTH = 120;
const int SUMMARY_HEIGHT = 60;
const int BUTTON_SPACING = 10;

// Helper function to calculate cost from quantity and unit cost
std::string CalculateCost(const std::string& quantity, const std::string& unitCost) {
    try {
        double qty = std::stod(quantity);
        std::string ucStr = unitCost;
        ucStr.erase(std::remove(ucStr.begin(), ucStr.end(), '$'), ucStr.end());
        ucStr.erase(std::remove(ucStr.begin(), ucStr.end(), ','), ucStr.end());
        double uc = std::stod(ucStr);
        
        double total = qty * uc;
        std::ostringstream oss;
        oss << "$" << std::fixed << std::setprecision(2) << total;
        return oss.str();
    } catch (...) {
        return "$0.00";
    }
}

// Dialog procedure for Add/Edit window
INT_PTR CALLBACK EntryDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG: {
            // Set window text based on whether we're adding or editing
            if (g_dialogData.category.empty() && g_dialogData.item.empty()) {
                SetWindowText(hwndDlg, "Add New Entry");
            } else {
                SetWindowText(hwndDlg, "Edit Entry");
                // Populate fields with existing data
                SetDlgItemText(hwndDlg, IDC_EDIT_CATEGORY, g_dialogData.category.c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_ITEM, g_dialogData.item.c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_MATERIAL, g_dialogData.material.c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_DESCRIPTION, g_dialogData.description.c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_QUANTITY, g_dialogData.quantity.c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_UNITCOST, g_dialogData.unitCost.c_str());
                SetDlgItemText(hwndDlg, IDC_EDIT_NOTES, g_dialogData.notes.c_str());
            }
            return TRUE;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDC_BTN_OK: {
                    // Get text from all fields
                    char buffer[256];
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_CATEGORY, buffer, 256);
                    g_dialogData.category = buffer;
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_ITEM, buffer, 256);
                    g_dialogData.item = buffer;
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_MATERIAL, buffer, 256);
                    g_dialogData.material = buffer;
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_DESCRIPTION, buffer, 256);
                    g_dialogData.description = buffer;
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_QUANTITY, buffer, 256);
                    g_dialogData.quantity = buffer;
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_UNITCOST, buffer, 256);
                    g_dialogData.unitCost = buffer;
                    
                    GetDlgItemText(hwndDlg, IDC_EDIT_NOTES, buffer, 256);
                    g_dialogData.notes = buffer;
                    
                    // Calculate cost
                    g_dialogData.cost = CalculateCost(g_dialogData.quantity, g_dialogData.unitCost);
                    
                    g_dialogResult = true;
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
                }

                case IDC_BTN_CANCEL: {
                    g_dialogResult = false;
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
                }
            }
            break;
        }

        case WM_CLOSE: {
            g_dialogResult = false;
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

// Dialog window procedure
LRESULT CALLBACK DialogWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BTN_OK) {
                // Get all the data
                char buffer[256];
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
            } else if (LOWORD(wParam) == IDC_BTN_CANCEL) {
                g_dialogResult = false;
                DestroyWindow(hwnd);
                return 0;
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

// Function to create the entry dialog dynamically
HWND CreateEntryDialog(HWND hwndParent) {
    // Register dialog window class
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = DialogWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "EntryDialogClass";
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        classRegistered = true;
    }

    // Create a modal dialog programmatically
    HWND hwndDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "EntryDialogClass",
        "Entry Dialog",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        hwndParent,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Create labels and edit controls
    int yPos = 20;
    int labelWidth = 100;
    int editWidth = 350;
    int rowHeight = 35;
    int xLabel = 20;
    int xEdit = xLabel + labelWidth + 10;

    const char* labels[] = {
        "Category:", "Item:", "Material:", "Description:",
        "Quantity:", "Unit Cost:", "Notes:"
    };
    int editIDs[] = {
        IDC_EDIT_CATEGORY, IDC_EDIT_ITEM, IDC_EDIT_MATERIAL,
        IDC_EDIT_DESCRIPTION, IDC_EDIT_QUANTITY, IDC_EDIT_UNITCOST,
        IDC_EDIT_NOTES
    };

    for (int i = 0; i < 7; i++) {
        // Create label
        CreateWindow("STATIC", labels[i],
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            xLabel, yPos + 3, labelWidth, 20,
            hwndDlg, NULL, GetModuleHandle(NULL), NULL);

        // Create edit control
        CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            xEdit, yPos, editWidth, 25,
            hwndDlg, (HMENU)(LONG_PTR)editIDs[i], GetModuleHandle(NULL), NULL);

        yPos += rowHeight;
    }

    // Create OK and Cancel buttons
    CreateWindow("BUTTON", "OK",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        150, yPos + 20, 80, 30,
        hwndDlg, (HMENU)IDC_BTN_OK, GetModuleHandle(NULL), NULL);

    CreateWindow("BUTTON", "Cancel",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        250, yPos + 20, 80, 30,
        hwndDlg, (HMENU)IDC_BTN_CANCEL, GetModuleHandle(NULL), NULL);

    return hwndDlg;
}

// Function to show the entry dialog
bool ShowEntryDialog(HWND hwndParent, DataRow& data, bool isEdit) {
    g_dialogData = data;
    g_dialogResult = false;

    HWND hwndDlg = CreateEntryDialog(hwndParent);
    
    // Set title
    if (isEdit) {
        SetWindowText(hwndDlg, "Edit Entry");
        // Populate fields
        SetDlgItemText(hwndDlg, IDC_EDIT_CATEGORY, data.category.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_ITEM, data.item.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_MATERIAL, data.material.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_DESCRIPTION, data.description.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_QUANTITY, data.quantity.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_UNITCOST, data.unitCost.c_str());
        SetDlgItemText(hwndDlg, IDC_EDIT_NOTES, data.notes.c_str());
    } else {
        SetWindowText(hwndDlg, "Add New Entry");
    }

    // Make parent window disabled
    EnableWindow(hwndParent, FALSE);

    // Message loop for dialog
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage((int)msg.wParam);
            break;
        }
        
        // Check if dialog is destroyed
        if (!IsWindow(hwndDlg)) {
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up
    if (IsWindow(hwndDlg)) {
        DestroyWindow(hwndDlg);
    }
    EnableWindow(hwndParent, TRUE);
    SetForegroundWindow(hwndParent);

    if (g_dialogResult) {
        data = g_dialogData;
    }

    return g_dialogResult;
}

// Function to update layout on resize
void UpdateLayout(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    int clientWidth = rc.right - rc.left;
    int clientHeight = rc.bottom - rc.top;
    
    int listViewHeight = clientHeight - (MARGIN * 3) - BUTTON_HEIGHT - SUMMARY_HEIGHT;
    int buttonY = MARGIN + listViewHeight + MARGIN;
    int summaryY = buttonY + BUTTON_HEIGHT + MARGIN;
    
    if (g_dataTable) {
        SetWindowPos(g_dataTable->GetHandle(), NULL, 
            MARGIN, MARGIN, 
            clientWidth - (MARGIN * 2), listViewHeight, 
            SWP_NOZORDER);
    }
    
    int buttonX = MARGIN;
    if (g_hBtnAdd) {
        SetWindowPos(g_hBtnAdd, NULL, buttonX, buttonY, 
            BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
        buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    }
    if (g_hBtnDelete) {
        SetWindowPos(g_hBtnDelete, NULL, buttonX, buttonY, 
            BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
        buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    }
    if (g_hBtnEdit) {
        SetWindowPos(g_hBtnEdit, NULL, buttonX, buttonY, 
            BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER);
        buttonX += BUTTON_WIDTH + BUTTON_SPACING;
    }
    if (g_hBtnSummary) {
        SetWindowPos(g_hBtnSummary, NULL, buttonX, buttonY, 
            BUTTON_WIDTH + 20, BUTTON_HEIGHT, SWP_NOZORDER);
    }
    
    if (g_hStaticSummary) {
        SetWindowPos(g_hStaticSummary, NULL, 
            MARGIN, summaryY, 
            clientWidth - (MARGIN * 2), SUMMARY_HEIGHT, 
            SWP_NOZORDER);
    }
}

// Function to update summary display
void UpdateSummary() {
    if (!g_dataTable || !g_hStaticSummary) return;
    
    int rowCount = g_dataTable->GetRowCount();
    double totalCost = g_dataTable->CalculateTotalCost();
    
    std::ostringstream oss;
    oss << "Total Entries: " << rowCount << "     |     "
        << "Total Cost: $" << std::fixed << std::setprecision(2) << totalCost;
    
    SetWindowText(g_hStaticSummary, oss.str().c_str());
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icex = {};
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_LISTVIEW_CLASSES;
            InitCommonControlsEx(&icex);

            g_dataTable = new DataTable(hwnd, 0, 0, 100, 100);

            g_dataTable->AddRow({"Electronics", "Laptop", "Aluminum", 
                "15-inch display", "5", "$899.99", "$4499.95", 
                "Bulk order discount"});
            
            g_dataTable->AddRow({"Office", "Desk Chair", "Mesh/Steel", 
                "Ergonomic office chair", "10", "$249.50", "$2495.00", 
                "Free shipping"});
            
            g_dataTable->AddRow({"Supplies", "Paper Reams", "Paper", 
                "500 sheets per ream", "50", "$4.99", "$249.50", 
                "Recycled paper"});

            g_hBtnAdd = CreateWindow("BUTTON", "Add Entry",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                0, 0, 100, 30,
                hwnd, (HMENU)ID_BTN_ADD, GetModuleHandle(NULL), NULL);

            g_hBtnDelete = CreateWindow("BUTTON", "Delete Entry",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                0, 0, 100, 30,
                hwnd, (HMENU)ID_BTN_DELETE, GetModuleHandle(NULL), NULL);

            g_hBtnEdit = CreateWindow("BUTTON", "Edit Entry",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                0, 0, 100, 30,
                hwnd, (HMENU)ID_BTN_EDIT, GetModuleHandle(NULL), NULL);

            g_hBtnSummary = CreateWindow("BUTTON", "Calculate Summary",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                0, 0, 100, 30,
                hwnd, (HMENU)ID_BTN_SUMMARY, GetModuleHandle(NULL), NULL);

            g_hStaticSummary = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "",
                WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
                0, 0, 100, 50,
                hwnd, (HMENU)ID_STATIC_SUMMARY, GetModuleHandle(NULL), NULL);

            UpdateLayout(hwnd);
            UpdateSummary();

            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_BTN_ADD: {
                    DataRow newRow = {"", "", "", "", "1", "$0.00", "$0.00", ""};
                    if (ShowEntryDialog(hwnd, newRow, false)) {
                        g_dataTable->AddRow(newRow);
                        UpdateSummary();
                    }
                    break;
                }

                case ID_BTN_DELETE: {
                    DataRow selectedRow;
                    if (g_dataTable->GetSelectedRow(selectedRow)) {
                        int result = MessageBox(hwnd, 
                            "Are you sure you want to delete the selected entry?", 
                            "Confirm Delete", MB_YESNO | MB_ICONQUESTION);
                        if (result == IDYES) {
                            g_dataTable->DeleteSelectedRow();
                            UpdateSummary();
                        }
                    } else {
                        MessageBox(hwnd, "Please select an entry to delete!", 
                            "No Selection", MB_OK | MB_ICONWARNING);
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
                        MessageBox(hwnd, "Please select an entry to edit!", 
                            "No Selection", MB_OK | MB_ICONWARNING);
                    }
                    break;
                }

                case ID_BTN_SUMMARY: {
                    UpdateSummary();
                    int rowCount = g_dataTable->GetRowCount();
                    double totalCost = g_dataTable->CalculateTotalCost();
                    
                    std::ostringstream oss;
                    oss << "Summary Report\n\n"
                        << "Total Number of Entries: " << rowCount << "\n"
                        << "Total Cost: $" << std::fixed << std::setprecision(2) << totalCost << "\n"
                        << "Average Cost per Entry: $" 
                        << (rowCount > 0 ? totalCost / rowCount : 0.0);
                    
                    MessageBox(hwnd, oss.str().c_str(), 
                        "Cost Summary", MB_OK | MB_ICONINFORMATION);
                    break;
                }
            }
            return 0;
        }

        case WM_SIZE: {
            UpdateLayout(hwnd);
            return 0;
        }

        case WM_DESTROY:
            delete g_dataTable;
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "DataTableWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Cost Tracker - Spreadsheet",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        950, 550, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}