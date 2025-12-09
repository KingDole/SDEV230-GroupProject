#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")

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

// DataTable class
class DataTable {
private:
    HWND hListView;
    std::vector<DataRow> rows;

public:
    DataTable(HWND hwnd, int x, int y, int width, int height) {
        // Create ListView control
        hListView = CreateWindowEx(0, WC_LISTVIEW, "",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS | WS_BORDER,
            x, y, width, height,
            hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);

        // Set extended styles for better appearance
        ListView_SetExtendedListViewStyle(hListView, 
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // Add columns
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

        // Set subitems
        ListView_SetItemText(hListView, index, 1, (LPSTR)row.item.c_str());
        ListView_SetItemText(hListView, index, 2, (LPSTR)row.material.c_str());
        ListView_SetItemText(hListView, index, 3, (LPSTR)row.description.c_str());
        ListView_SetItemText(hListView, index, 4, (LPSTR)row.quantity.c_str());
        ListView_SetItemText(hListView, index, 5, (LPSTR)row.unitCost.c_str());
        ListView_SetItemText(hListView, index, 6, (LPSTR)row.cost.c_str());
        ListView_SetItemText(hListView, index, 7, (LPSTR)row.notes.c_str());
    }

    // Get the currently selected row index (-1 if none)
    int GetSelectedIndex() {
        return ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
    }

    // Get the data from the selected row
    bool GetSelectedRow(DataRow& outRow) {
        int index = GetSelectedIndex();
        if (index == -1 || index >= (int)rows.size()) {
            return false;
        }
        outRow = rows[index];
        return true;
    }

    HWND GetHandle() { return hListView; }
};

// Global pointer to DataTable
DataTable* g_dataTable = nullptr;

// This is the function that will receive the selected row data
void ProcessSelectedRow(const DataRow& row) {
    // Build a message with the row details
    std::string message = "Selected Row Details:\n\n";
    message += "Category: " + row.category + "\n";
    message += "Item: " + row.item + "\n";
    message += "Material: " + row.material + "\n";
    message += "Description: " + row.description + "\n";
    message += "Quantity: " + row.quantity + "\n";
    message += "Unit Cost: " + row.unitCost + "\n";
    message += "Cost: " + row.cost + "\n";
    message += "Notes: " + row.notes;

    MessageBox(NULL, message.c_str(), "Row Data", MB_OK | MB_ICONINFORMATION);
    
    // Here you would do whatever processing you need with the row data
    // For example: SaveToDatabase(row), GenerateReport(row), etc.
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Initialize common controls
            INITCOMMONCONTROLSEX icex = {};
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_LISTVIEW_CLASSES;
            InitCommonControlsEx(&icex);

            // Create DataTable
            g_dataTable = new DataTable(hwnd, 10, 10, 880, 350);

            // Add sample data
            g_dataTable->AddRow({"Electronics", "Laptop", "Aluminum", 
                "15-inch display", "5", "$899.99", "$4499.95", 
                "Bulk order discount"});
            
            g_dataTable->AddRow({"Office", "Desk Chair", "Mesh/Steel", 
                "Ergonomic office chair", "10", "$249.50", "$2495.00", 
                "Free shipping"});
            
            g_dataTable->AddRow({"Supplies", "Paper Reams", "Paper", 
                "500 sheets per ream", "50", "$4.99", "$249.50", 
                "Recycled paper"});

            // Create "Process Selected Row" button
            CreateWindow("BUTTON", "Process Selected Row",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                10, 370, 200, 30,
                hwnd, (HMENU)2001, GetModuleHandle(NULL), NULL);

            return 0;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == 2001) { // Button clicked
                DataRow selectedRow;
                if (g_dataTable->GetSelectedRow(selectedRow)) {
                    // Pass the selected row to your processing function
                    ProcessSelectedRow(selectedRow);
                } else {
                    MessageBox(hwnd, "Please select a row first!", 
                        "No Selection", MB_OK | MB_ICONWARNING);
                }
            }
            return 0;
        }

        case WM_SIZE: {
            // Resize ListView with window
            if (g_dataTable) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                SetWindowPos(g_dataTable->GetHandle(), NULL, 10, 10, 
                    rc.right - 20, rc.bottom - 60, SWP_NOZORDER);
            }
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

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "DataTable Example",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        920, 480, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}