#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <set>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell*> ref_from_cells_;

    bool IsCyclicDependencies(std::unique_ptr<Impl>& impl) const;
    void InvalidateCache();
    void SetFormulaCell(const std::string& text);

    class Impl {
    public:
        virtual ~Impl() = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;
        virtual void ResetCache();
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(const std::string& text);
        Value GetValue() const override;
        std::string GetText() const override;
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string expression, const SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void ResetCache() override;
    private:
        const SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> ptr_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };
};


