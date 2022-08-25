#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <iostream>
#include <stack>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }

    if (spreadsheet_.count(pos)) {
        spreadsheet_.at(pos)->Set(std::move(text));
    } else {
        spreadsheet_[pos] = std::make_unique<Cell>(*this);
        spreadsheet_[pos]->Set(std::move(text));
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }

    return (spreadsheet_.count(pos)) ? spreadsheet_.at(pos).get() : nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }

    return (spreadsheet_.count(pos)) ? spreadsheet_.at(pos).get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position"s);
    }

    if (spreadsheet_.count(pos)) {
        spreadsheet_.at(pos).reset();
        spreadsheet_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    int max_row = 0;
    int max_col = 0;

    for (const auto& [pos, _] : spreadsheet_) {
        max_row = std::max(max_row, pos.row + 1);
        max_col = std::max(max_col, pos.col + 1);
    }

    return {max_row, max_col};
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col != 0) {
                output << "\t"s;
            }

            if (spreadsheet_.count({row, col}) && spreadsheet_.at({row, col})) {
                std::visit(
                    [&output] (const auto& value) { output << value; },
                    spreadsheet_.at({row, col})->GetValue()
                );
            }
        }
        output << "\n"s;
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col != 0) {
                output << "\t"s;
            }

            if (spreadsheet_.count({row, col}) && spreadsheet_.at({row, col})) {
                output << spreadsheet_.at({row, col})->GetText();
            }
        }
        output << "\n"s;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}


