#include "DataTable.h"
#include <algorithm>

#pragma comment(lib, "comctl32.lib")

//--------------------------------------------------
// Constructor
//--------------------------------------------------
DataTable::DataTable(HWND parent, int x, int y, int width, int height)
    : hParent(parent)
{
    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    hListView = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEWW,
        L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        x, y, width, height,
        hParent,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
    );

    // Enable full-row select and grid lines
    ListView_SetExtendedListViewStyle(
        hListView,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    InitializeColumns();
}

//--------------------------------------------------
// Destructor
//--------------------------------------------------
DataTable::~DataTable() {
    if (hListView) {
        DestroyWindow(hListView);
        hListView = nullptr;
    }
}

//--------------------------------------------------
// Column setup
//--------------------------------------------------
void DataTable::InitializeColumns() {
    const wchar_t* headers[] = {
        L"Category", L"Item", L"Material", L"Description",
        L"Quantity", L"Unit Cost", L"Cost", L"Notes"
    };

    int widths[] = { 100, 120, 120, 200, 70, 90, 90, 200 };

    for (int i = 0; i < 8; ++i) {
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = const_cast<LPWSTR>(headers[i]);
        col.cx = widths[i];
        col.iSubItem = i;
        ListView_InsertColumn(hListView, i, &col);
    }
}

//--------------------------------------------------
// Refresh ListView
//--------------------------------------------------
void DataTable::RefreshList() {
    ListView_DeleteAllItems(hListView);

    for (size_t i = 0; i < rows.size(); ++i) {
        const DataRow& r = rows[i];

        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(i);
        item.iSubItem = 0;
        item.pszText = const_cast<LPWSTR>(r.category.c_str());
        ListView_InsertItem(hListView, &item);

        ListView_SetItemText(hListView, item.iItem, 1, const_cast<LPWSTR>(r.item.c_str()));
        ListView_SetItemText(hListView, item.iItem, 2, const_cast<LPWSTR>(r.material.c_str()));
        ListView_SetItemText(hListView, item.iItem, 3, const_cast<LPWSTR>(r.description.c_str()));
        ListView_SetItemText(hListView, item.iItem, 4, const_cast<LPWSTR>(r.quantity.c_str()));
        ListView_SetItemText(hListView, item.iItem, 5, const_cast<LPWSTR>(r.unitCost.c_str()));
        ListView_SetItemText(hListView, item.iItem, 6, const_cast<LPWSTR>(r.cost.c_str()));
        ListView_SetItemText(hListView, item.iItem, 7, const_cast<LPWSTR>(r.notes.c_str()));
    }
}

//--------------------------------------------------
// Add Row
//--------------------------------------------------
void DataTable::AddRow(const DataRow& row) {
    rows.push_back(row);
    RefreshList();
}

//--------------------------------------------------
// Update Row
//--------------------------------------------------
void DataTable::UpdateRow(int index, const DataRow& row) {
    if (index < 0 || index >= static_cast<int>(rows.size())) return;
    rows[index] = row;
    RefreshList();
}

//--------------------------------------------------
// Delete Selected Row
//--------------------------------------------------
void DataTable::DeleteSelectedRow() {
    int index = GetSelectedIndex();
    if (index < 0 || index >= static_cast<int>(rows.size())) return;

    rows.erase(rows.begin() + index);
    RefreshList();
}

//--------------------------------------------------
// Get Selected Row
//--------------------------------------------------
bool DataTable::GetSelectedRow(DataRow& outRow) const {
    int index = GetSelectedIndex();
    if (index < 0 || index >= static_cast<int>(rows.size())) return false;

    outRow = rows[index];
    return true;
}

//--------------------------------------------------
// Get Selected Index
//--------------------------------------------------
int DataTable::GetSelectedIndex() const {
    return ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
}

//--------------------------------------------------
// Get Row Count
//--------------------------------------------------
int DataTable::GetRowCount() const {
    return static_cast<int>(rows.size());
}

//--------------------------------------------------
// Get Handle
//--------------------------------------------------
HWND DataTable::GetHandle() const {
    return hListView;
}

//--------------------------------------------------
// Clear Table
//--------------------------------------------------
void DataTable::Clear() {
    rows.clear();
    ListView_DeleteAllItems(hListView);
}

//--------------------------------------------------
// Calculate Total Cost
//--------------------------------------------------
double DataTable::CalculateTotalCost() const {
    double total = 0.0;

    for (const auto& r : rows) {
        std::wstring temp = r.cost;
        temp.erase(std::remove(temp.begin(), temp.end(), L'$'), temp.end());
        temp.erase(std::remove(temp.begin(), temp.end(), L','), temp.end());

        try {
            total += std::stod(temp);
        } catch (...) {}
    }
    return total;
}

//--------------------------------------------------
// Get All Rows (Save / Load)
//--------------------------------------------------
const std::vector<DataRow>& DataTable::GetAllRows() const {
    return rows;
}
