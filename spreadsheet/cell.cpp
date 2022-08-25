#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

std::string EraseFirstChar(std::string str) {
    return str.substr(1);
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

void Cell::Impl::ResetCache() {}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return {};
}

std::string Cell::EmptyImpl::GetText() const {
    return {};
}

Cell::TextImpl::TextImpl(const std::string& text)
    : text_(std::move(text)) {
}

Cell::Value Cell::TextImpl::GetValue() const{
    if (text_.front() == ESCAPE_SIGN) {
        return std::move(EraseFirstChar(text_));
    }

    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string expression, const SheetInterface& sheet)
    : sheet_(sheet)
    , ptr_(ParseFormula(std::move(EraseFirstChar(expression)))) {
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_.has_value()) {
        cache_ = ptr_->Evaluate(sheet_);
    }

    if (std::holds_alternative<double>(*cache_)) {
        return std::get<double>(*cache_);
    } else {
        return std::get<FormulaError>(*cache_);
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + ptr_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return ptr_->GetReferencedCells();
}

void Cell::FormulaImpl::ResetCache() {
    cache_.reset();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

Cell::Cell(Sheet& sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet) {
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();

    } else if (text.front() == FORMULA_SIGN && text.size() != 1) {
        SetFormulaCell(text);

    } else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

bool Cell::IsReferenced() const {
    return GetReferencedCells().empty();
}

void Cell::SetFormulaCell(const std::string& text) {
    using namespace std::literals;
    std::unique_ptr<Impl> tmp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);

    if (IsCyclicDependencies(tmp_impl)) {
        throw CircularDependencyException("cyclic dependence in the spreadsheet"s);
    } else {
        std::swap(impl_, tmp_impl);
    }

    for (const auto& pos : impl_->GetReferencedCells()) {
        auto* cell = sheet_.GetCell(pos);

        if (!cell) {
            sheet_.SetCell(pos, {});
            cell = sheet_.GetCell(pos);
        }
        static_cast<Cell*>(cell)->ref_from_cells_.insert(this);
    }

    InvalidateCache();
}

bool Cell::IsCyclicDependencies(std::unique_ptr<Impl>& impl) const {
    std::unordered_set<const Cell*> ref_cells;

    for (const Position& pos : impl->GetReferencedCells()) {
        ref_cells.insert(static_cast<Cell*>(sheet_.GetCell(pos)));
    }

    std::unordered_set<const Cell*> visited_cells;
    std::stack<const Cell*> cells;
    cells.push(this);

    while (!cells.empty()) {
        const Cell* curr_cell = cells.top();
        if (ref_cells.count(curr_cell)) {
            return true;
        }

        cells.pop();
        visited_cells.insert(curr_cell);

        for (const auto* cell : curr_cell->ref_from_cells_) {
            if (!visited_cells.count(cell)) {
                cells.push(cell);
            }
        }
    }
    return false;
}

void Cell::InvalidateCache() {
    impl_->ResetCache();

    for (Cell* cell : ref_from_cells_) {
        cell->InvalidateCache();
    }
}
