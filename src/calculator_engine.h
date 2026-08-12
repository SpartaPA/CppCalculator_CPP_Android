#pragma once
#include <string>
#include <vector>
#include <variant>

namespace calc {

enum class ValueType { Number, Set, Matrix, Sequence };

struct Matrix {
    std::vector<std::vector<double>> v;
};

struct Value {
    ValueType type = ValueType::Number;
    double number = 0.0;
    std::vector<double> sequence;
    std::vector<double> set;
    Matrix matrix;

    static Value Num(double x);
    static Value Seq(std::vector<double> x);
    static Value SetV(std::vector<double> x);
    static Value Mat(Matrix x);
};

class CalculatorEngine {
public:
    Value calc(const std::string& input);
    static std::string format(const Value& value);

private:
    enum class TokenType { Num, Pow, Mul, Plus, Minus, Div, LParen, RParen,
                           LBrace, RBrace, LBrack, RBrack, Comma, Eof };
    struct Token { TokenType type; double value = 0.0; };

    struct Node {
        enum class Kind { Number, Group, Neg, Sequence, Set, Matrix, Binary };
        Kind kind;
        double number = 0;
        std::string op;
        std::vector<Node> children;
        std::vector<std::vector<Node>> rows;
    };

    std::vector<Token> tokens;
    size_t tp = 0;

    std::vector<Token> tokenize(const std::string&);
    const Token& peek() const;
    Token next();
    void expect(TokenType);

    Node parseExpression();
    Node parseTerm();
    Node parseUnary();
    Node parsePower();
    Node parsePrimary();
    std::vector<Node> parseRow();

    Value evaluate(const Node&);
    static double numberOnly(const Value&);
    static Value neg(const Value&);
    static Value apply(const std::string&, const Value&, const Value&);
    static Value add(const Value&, const Value&);
    static Value sub(const Value&, const Value&);
    static Value mul(const Value&, const Value&);
    static Value divv(const Value&, const Value&);
    static Value powv(const Value&, const Value&);

    static Matrix matAdd(const Matrix&, const Matrix&);
    static Matrix matSub(const Matrix&, const Matrix&);
    static Matrix matMul(const Matrix&, const Matrix&);
    static Matrix scale(const Matrix&, double);
    static Matrix identity(int);
    static Matrix inverse(const Matrix&);
    static Matrix matPow(const Matrix&, double);

    static std::vector<double> dedupe(const std::vector<double>&);
    static std::string typeName(ValueType);
    static std::string formatMatrix(const Matrix&);
};

}
