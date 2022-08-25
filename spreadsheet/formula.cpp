#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

double StrToDouble(const std::string& value);

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        try : ast_(ParseFormulaAST(expression)) {

        } catch (const FormulaException& f_exp) {
            throw f_exp;
        }

    Value Evaluate(const SheetInterface& sheet) const override {
        // Определяем объект-функцию, которая по индексу ячейки
        // возвращает её значение в конкретной таблице
        const std::function<double(Position)> args = [&sheet](const Position pos)->double {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            } else if (!sheet.GetCell(pos)) {
                return 0;
            }
            const auto& value = sheet.GetCell(pos)->GetValue();

            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);

            } else if (std::holds_alternative<std::string>(value)) {
                return StrToDouble(std::get<std::string>(value));

            } else {
                throw FormulaError(std::get<FormulaError>(value));
            }
        };

        try {
            return ast_.Execute(args);
        } catch (const FormulaError& f_err) {
            return f_err;
        }
    }

    std::vector<Position> GetReferencedCells() const override {
        std::set<Position> cells;

        for (const auto& cell : ast_.GetCells()) {
            if (cell.IsValid()) {
                cells.insert(cell);
            }
        }
        return {cells.begin(), cells.end()};
    }

    std::string GetExpression() const override {
        std::ostringstream expression;
        ast_.PrintFormula(expression);
        return expression.str();
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("incorrect formula"s);
    }
}

double StrToDouble(const std::string& value) {
    if (value.empty()) {
        return 0;
    }

    if (value.find_first_not_of("0123456789"s) != std::string::npos) {
        throw FormulaError(FormulaError::Category::Value);
    }

    try {
        return stod(value);
    } catch (...) {
        throw FormulaError(FormulaError::Category::Value);
    }
}

FormulaError::FormulaError(Category category)
    : category_(category) {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref:
            return "#REF!"s;
        case Category::Value:
            return "#VALUE!"s;
        case Category::Div0:
            return "#DIV/0!"s;
        default:
            return {};
    }
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}


