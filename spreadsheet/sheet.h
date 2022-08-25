#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    struct PositionHasher {
        size_t operator()(const Position p) const {
            size_t h_row = i_hasher_(p.row);
            size_t h_col = i_hasher_(p.col);
            return h_row + h_col * 37;
        }
    private:
        std::hash<int> i_hasher_;
	};

	std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> spreadsheet_;
};
