#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "DataTable.h"   // for DataRow

class SpreadsheetStorage {
public:
    // Save rows to CSV file
    static bool SaveToCSV(
        const std::wstring& filePath,
        const std::vector<DataRow>& rows
    )
    {
        std::wofstream file(filePath);
        if (!file.is_open())
            return false;

        // Optional header row
        file << L"Category,Item,Material,Description,Quantity,Unit Cost,Cost,Notes\n";

        for (const auto& row : rows) {
            file
                << Escape(row.category)    << L","
                << Escape(row.item)        << L","
                << Escape(row.material)    << L","
                << Escape(row.description) << L","
                << Escape(row.quantity)    << L","
                << Escape(row.unitCost)    << L","
                << Escape(row.cost)        << L","
                << Escape(row.notes)
                << L"\n";
        }

        return true;
    }

    // Load rows from CSV file
    static bool LoadFromCSV(
        const std::wstring& filePath,
        std::vector<DataRow>& outRows
    )
    {
        std::wifstream file(filePath);
        if (!file.is_open())
            return false;

        outRows.clear();

        std::wstring line;

        // Skip header
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::vector<std::wstring> fields = ParseCSVLine(line);
            if (fields.size() != 8)
                continue;

            DataRow row;
            row.category    = fields[0];
            row.item        = fields[1];
            row.material    = fields[2];
            row.description = fields[3];
            row.quantity    = fields[4];
            row.unitCost    = fields[5];
            row.cost        = fields[6];
            row.notes       = fields[7];

            outRows.push_back(row);
        }

        return true;
    }

private:
    // Escape CSV fields that contain commas or quotes
    static std::wstring Escape(const std::wstring& field)
    {
        if (field.find(L',') == std::wstring::npos &&
            field.find(L'"') == std::wstring::npos)
            return field;

        std::wstring escaped = L"\"";
        for (wchar_t ch : field) {
            if (ch == L'"')
                escaped += L"\"\"";
            else
                escaped += ch;
        }
        escaped += L"\"";
        return escaped;
    }

    // Parse a CSV line into fields
    static std::vector<std::wstring> ParseCSVLine(const std::wstring& line)
    {
        std::vector<std::wstring> result;
        std::wstring field;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); i++) {
            wchar_t ch = line[i];

            if (ch == L'"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == L'"') {
                    field += L'"';
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            }
            else if (ch == L',' && !inQuotes) {
                result.push_back(field);
                field.clear();
            }
            else {
                field += ch;
            }
        }

        result.push_back(field);
        return result;
    }
};
