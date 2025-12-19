#pragma once
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>

struct DataRow {
    std::wstring category;
    std::wstring item;
    std::wstring material;
    std::wstring description;
    std::wstring quantity;
    std::wstring unitCost;
    std::wstring cost;
    std::wstring notes;
};

class DataTable {
public:
    DataTable(HWND parent, int x, int y, int width, int height);
    ~DataTable();

    void AddRow(const DataRow& row);
    void UpdateRow(int index, const DataRow& row);
    void DeleteSelectedRow();

    bool GetSelectedRow(DataRow& outRow) const;
    int  GetSelectedIndex() const;

    int GetRowCount() const;
    double CalculateTotalCost() const;

    const std::vector<DataRow>& GetAllRows() const;
    void Clear();

    HWND GetHandle() const;

private:
    void InitializeColumns();
    void RefreshList();

    HWND hParent = nullptr;
    HWND hListView = nullptr;
    std::vector<DataRow> rows;
};
