#include "calculator_engine.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <cctype>

namespace calc {

Value Value::Num(double x){ Value v; v.type=ValueType::Number; v.number=x; return v; }
Value Value::Seq(std::vector<double> x){ Value v; v.type=ValueType::Sequence; v.sequence=std::move(x); return v; }
Value Value::SetV(std::vector<double> x){ Value v; v.type=ValueType::Set; v.set=std::move(x); return v; }
Value Value::Mat(Matrix x){ Value v; v.type=ValueType::Matrix; v.matrix=std::move(x); return v; }

Value CalculatorEngine::calc(const std::string& input){
    if(input.empty()) throw std::runtime_error("입력이 비어 있습니다");
    tokens=tokenize(input); tp=0;
    Node n=parseExpression(); expect(TokenType::Eof);
    return evaluate(n);
}

std::vector<CalculatorEngine::Token> CalculatorEngine::tokenize(const std::string& x){
    std::vector<Token> t;
    size_t i=0;
    while(i<x.size()){
        unsigned char c=(unsigned char)x[i];
        if(std::isspace(c)){++i;continue;}
        if(std::isdigit(c)||c=='.'){
            size_t j=i; int dots=0; bool digit=false;
            while(j<x.size() && (std::isdigit((unsigned char)x[j])||x[j]=='.')){
                if(x[j]=='.') ++dots; else digit=true; ++j;
            }
            if(dots>1||!digit) throw std::runtime_error("숫자 형식이 올바르지 않습니다");
            t.push_back({TokenType::Num,std::stod(x.substr(i,j-i))}); i=j; continue;
        }
        if(c=='*'){
            if(i+1<x.size()&&x[i+1]=='*'){t.push_back({TokenType::Pow});i+=2;}
            else {t.push_back({TokenType::Mul});++i;}
            continue;
        }
        TokenType tt;
        switch(c){
            case '+':tt=TokenType::Plus;break; case '-':tt=TokenType::Minus;break;
            case '/':tt=TokenType::Div;break; case '(':tt=TokenType::LParen;break;
            case ')':tt=TokenType::RParen;break; case '{':tt=TokenType::LBrace;break;
            case '}':tt=TokenType::RBrace;break; case '[':tt=TokenType::LBrack;break;
            case ']':tt=TokenType::RBrack;break; case ',':tt=TokenType::Comma;break;
            default: throw std::runtime_error("알 수 없는 문자입니다");
        }
        t.push_back({tt}); ++i;
    }
    t.push_back({TokenType::Eof}); return t;
}

const CalculatorEngine::Token& CalculatorEngine::peek() const{return tokens.at(tp);}
CalculatorEngine::Token CalculatorEngine::next(){return tokens.at(tp++);}
void CalculatorEngine::expect(TokenType type){auto z=next();if(z.type!=type)throw std::runtime_error("괄호 또는 구분자가 올바르지 않습니다");}

CalculatorEngine::Node CalculatorEngine::parseExpression(){
    Node n=parseTerm();
    while(peek().type==TokenType::Plus||peek().type==TokenType::Minus){
        auto op=next().type==TokenType::Plus?"+":"-"; Node r=parseTerm();
        Node b; b.kind=Node::Kind::Binary;b.op=op;b.children={std::move(n),std::move(r)};n=std::move(b);
    } return n;
}
CalculatorEngine::Node CalculatorEngine::parseTerm(){
    Node n=parseUnary();
    while(peek().type==TokenType::Mul||peek().type==TokenType::Div){
        auto op=next().type==TokenType::Mul?"*":"/"; Node r=parseUnary();
        Node b; b.kind=Node::Kind::Binary;b.op=op;b.children={std::move(n),std::move(r)};n=std::move(b);
    } return n;
}
CalculatorEngine::Node CalculatorEngine::parseUnary(){
    if(peek().type==TokenType::Minus){next();Node n; n.kind=Node::Kind::Neg;n.children={parseUnary()};return n;}
    return parsePower();
}
CalculatorEngine::Node CalculatorEngine::parsePower(){
    Node b=parsePrimary();
    if(peek().type==TokenType::Pow){next();Node r=parseUnary();Node n;n.kind=Node::Kind::Binary;n.op="**";n.children={std::move(b),std::move(r)};return n;}
    return b;
}
CalculatorEngine::Node CalculatorEngine::parsePrimary(){
    auto t=peek();
    if(t.type==TokenType::Num){next();Node n;n.kind=Node::Kind::Number;n.number=t.value;return n;}
    if(t.type==TokenType::LParen){
        next();std::vector<Node> e{parseExpression()};
        while(peek().type==TokenType::Comma){next();e.push_back(parseExpression());}
        expect(TokenType::RParen);
        Node n;n.kind=e.size()==1?Node::Kind::Group:Node::Kind::Sequence;n.children=std::move(e);return n;
    }
    if(t.type==TokenType::LBrace){
        next();std::vector<Node> e{parseExpression()};
        while(peek().type==TokenType::Comma){next();e.push_back(parseExpression());}
        expect(TokenType::RBrace);Node n;n.kind=Node::Kind::Set;n.children=std::move(e);return n;
    }
    if(t.type==TokenType::LBrack){
        next();std::vector<std::vector<Node>> rows;
        if(peek().type==TokenType::LBrack){
            rows.push_back(parseRow());
            while(peek().type==TokenType::Comma){next();rows.push_back(parseRow());}
        } else {
            std::vector<Node> row{parseExpression()};
            while(peek().type==TokenType::Comma){next();row.push_back(parseExpression());}
            rows.push_back(std::move(row));
        }
        expect(TokenType::RBrack);Node n;n.kind=Node::Kind::Matrix;n.rows=std::move(rows);return n;
    }
    throw std::runtime_error("예상치 못한 입력입니다");
}
std::vector<CalculatorEngine::Node> CalculatorEngine::parseRow(){
    expect(TokenType::LBrack);std::vector<Node> row{parseExpression()};
    while(peek().type==TokenType::Comma){next();row.push_back(parseExpression());}
    expect(TokenType::RBrack);return row;
}

Value CalculatorEngine::evaluate(const Node& n){
    switch(n.kind){
        case Node::Kind::Number:return Value::Num(n.number);
        case Node::Kind::Group:return evaluate(n.children[0]);
        case Node::Kind::Neg:return neg(evaluate(n.children[0]));
        case Node::Kind::Sequence:{std::vector<double> v;for(auto&x:n.children)v.push_back(numberOnly(evaluate(x)));return Value::Seq(v);}
        case Node::Kind::Set:{std::vector<double> v;for(auto&x:n.children)v.push_back(numberOnly(evaluate(x)));return Value::SetV(dedupe(v));}
        case Node::Kind::Matrix:{Matrix m;for(auto&r:n.rows){std::vector<double> row;for(auto&x:r)row.push_back(numberOnly(evaluate(x)));m.v.push_back(std::move(row));}if(m.v.empty())throw std::runtime_error("빈 행렬입니다");for(auto&r:m.v)if(r.size()!=m.v[0].size())throw std::runtime_error("행렬의 각 행 길이가 동일해야 합니다");return Value::Mat(m);}
        case Node::Kind::Binary:return apply(n.op,evaluate(n.children[0]),evaluate(n.children[1]));
    } throw std::runtime_error("알 수 없는 노드입니다");
}
double CalculatorEngine::numberOnly(const Value&v){if(v.type!=ValueType::Number)throw std::runtime_error("원소는 숫자여야 합니다");return v.number;}
Value CalculatorEngine::neg(const Value&v){
    if(v.type==ValueType::Number)return Value::Num(-v.number);
    if(v.type==ValueType::Sequence){auto x=v.sequence;for(auto&z:x)z=-z;return Value::Seq(x);}
    if(v.type==ValueType::Set){auto x=v.set;for(auto&z:x)z=-z;return Value::SetV(dedupe(x));}
    return Value::Mat(scale(v.matrix,-1));
}
Value CalculatorEngine::apply(const std::string&o,const Value&a,const Value&b){
    if(o=="+")return add(a,b);if(o=="-")return sub(a,b);if(o=="*")return mul(a,b);if(o=="/")return divv(a,b);return powv(a,b);
}
Value CalculatorEngine::add(const Value&a,const Value&b){
    if(a.type==ValueType::Number&&b.type==ValueType::Number)return Value::Num(a.number+b.number);
    if(a.type==ValueType::Matrix&&b.type==ValueType::Matrix)return Value::Mat(matAdd(a.matrix,b.matrix));
    if(a.type==ValueType::Sequence&&b.type==ValueType::Sequence){if(a.sequence.size()!=b.sequence.size())throw std::runtime_error("수열의 길이가 서로 다릅니다");std::vector<double>x(a.sequence.size());for(size_t i=0;i<x.size();++i)x[i]=a.sequence[i]+b.sequence[i];return Value::Seq(x);}
    if(a.type==ValueType::Set&&b.type==ValueType::Set){auto x=a.set;x.insert(x.end(),b.set.begin(),b.set.end());return Value::SetV(dedupe(x));}
    throw std::runtime_error("덧셈은 같은 타입끼리만 가능합니다");
}
Value CalculatorEngine::sub(const Value&a,const Value&b){
    if(a.type==ValueType::Number&&b.type==ValueType::Number)return Value::Num(a.number-b.number);
    if(a.type==ValueType::Matrix&&b.type==ValueType::Matrix)return Value::Mat(matSub(a.matrix,b.matrix));
    if(a.type==ValueType::Sequence&&b.type==ValueType::Sequence){if(a.sequence.size()!=b.sequence.size())throw std::runtime_error("수열의 길이가 서로 다릅니다");std::vector<double>x(a.sequence.size());for(size_t i=0;i<x.size();++i)x[i]=a.sequence[i]-b.sequence[i];return Value::Seq(x);}
    if(a.type==ValueType::Set&&b.type==ValueType::Set){std::vector<double>x;for(double z:a.set)if(std::none_of(b.set.begin(),b.set.end(),[z](double y){return std::abs(y-z)<1e-9;}))x.push_back(z);return Value::SetV(x);}
    throw std::runtime_error("뺄셈은 같은 타입끼리만 가능합니다");
}
Value CalculatorEngine::mul(const Value&a,const Value&b){
    if(a.type==ValueType::Number&&b.type==ValueType::Number)return Value::Num(a.number*b.number);
    if(a.type==ValueType::Number&&b.type==ValueType::Matrix)return Value::Mat(scale(b.matrix,a.number));
    if(b.type==ValueType::Number&&a.type==ValueType::Matrix)return Value::Mat(scale(a.matrix,b.number));
    if(a.type==ValueType::Number&&b.type==ValueType::Set){auto x=b.set;for(auto&z:x)z*=a.number;return Value::SetV(dedupe(x));}
    if(b.type==ValueType::Number&&a.type==ValueType::Set){auto x=a.set;for(auto&z:x)z*=b.number;return Value::SetV(dedupe(x));}
    if(a.type==ValueType::Number&&b.type==ValueType::Sequence){auto x=b.sequence;for(auto&z:x)z*=a.number;return Value::Seq(x);}
    if(b.type==ValueType::Number&&a.type==ValueType::Sequence){auto x=a.sequence;for(auto&z:x)z*=b.number;return Value::Seq(x);}
    if(a.type==ValueType::Matrix&&b.type==ValueType::Matrix)return Value::Mat(matMul(a.matrix,b.matrix));
    if(a.type==ValueType::Sequence&&b.type==ValueType::Sequence){if(a.sequence.size()!=b.sequence.size())throw std::runtime_error("수열의 길이가 서로 다릅니다");std::vector<double>x(a.sequence.size());for(size_t i=0;i<x.size();++i)x[i]=a.sequence[i]*b.sequence[i];return Value::Seq(x);}
    if(a.type==ValueType::Set&&b.type==ValueType::Set){std::vector<double>x;for(double z:a.set)if(std::any_of(b.set.begin(),b.set.end(),[z](double y){return std::abs(y-z)<1e-9;}))x.push_back(z);return Value::SetV(x);}
    throw std::runtime_error("곱할 수 없는 타입 조합입니다");
}
Value CalculatorEngine::divv(const Value&a,const Value&b){
    if(a.type==ValueType::Number&&b.type==ValueType::Number){if(std::abs(b.number)<1e-15)throw std::runtime_error("0으로 나눌 수 없습니다");return Value::Num(a.number/b.number);}
    if(a.type==ValueType::Matrix&&b.type==ValueType::Matrix)return Value::Mat(matMul(a.matrix,inverse(b.matrix)));
    if(a.type==ValueType::Matrix&&b.type==ValueType::Number){if(std::abs(b.number)<1e-15)throw std::runtime_error("0으로 나눌 수 없습니다");return Value::Mat(scale(a.matrix,1.0/b.number));}
    if(a.type==ValueType::Sequence&&b.type==ValueType::Number){if(std::abs(b.number)<1e-15)throw std::runtime_error("0으로 나눌 수 없습니다");auto x=a.sequence;for(auto&z:x)z/=b.number;return Value::Seq(x);}
    if(a.type==ValueType::Sequence&&b.type==ValueType::Sequence){if(a.sequence.size()!=b.sequence.size())throw std::runtime_error("수열의 길이가 서로 다릅니다");std::vector<double>x(a.sequence.size());for(size_t i=0;i<x.size();++i){if(std::abs(b.sequence[i])<1e-15)throw std::runtime_error("0으로 나눌 수 없습니다");x[i]=a.sequence[i]/b.sequence[i];}return Value::Seq(x);}
    throw std::runtime_error("나눌 수 없는 타입 조합입니다");
}
Value CalculatorEngine::powv(const Value&a,const Value&b){
    if(b.type!=ValueType::Number)throw std::runtime_error("지수는 숫자여야 합니다");
    if(a.type==ValueType::Number)return Value::Num(std::pow(a.number,b.number));
    if(a.type==ValueType::Matrix)return Value::Mat(matPow(a.matrix,b.number));
    if(a.type==ValueType::Sequence){auto x=a.sequence;for(auto&z:x)z=std::pow(z,b.number);return Value::Seq(x);}
    throw std::runtime_error("제곱을 지원하지 않는 타입입니다");
}
Matrix CalculatorEngine::matAdd(const Matrix&a,const Matrix&b){
    if(a.v.size()!=b.v.size()||a.v[0].size()!=b.v[0].size())throw std::runtime_error("행렬 크기가 맞지 않습니다");
    Matrix m=a;for(size_t i=0;i<a.v.size();++i)for(size_t j=0;j<a.v[0].size();++j)m.v[i][j]=a.v[i][j]+b.v[i][j];return m;
}
Matrix CalculatorEngine::matSub(const Matrix&a,const Matrix&b){
    if(a.v.size()!=b.v.size()||a.v[0].size()!=b.v[0].size())throw std::runtime_error("행렬 크기가 맞지 않습니다");
    Matrix m=a;for(size_t i=0;i<a.v.size();++i)for(size_t j=0;j<a.v[0].size();++j)m.v[i][j]=a.v[i][j]-b.v[i][j];return m;
}
Matrix CalculatorEngine::matMul(const Matrix&a,const Matrix&b){
    if(a.v[0].size()!=b.v.size())throw std::runtime_error("행렬 크기가 맞지 않습니다");
    Matrix m;m.v.assign(a.v.size(),std::vector<double>(b.v[0].size(),0));
    for(size_t i=0;i<a.v.size();++i)for(size_t j=0;j<b.v[0].size();++j)for(size_t k=0;k<b.v.size();++k)m.v[i][j]+=a.v[i][k]*b.v[k][j];return m;
}
Matrix CalculatorEngine::scale(const Matrix&a,double s){Matrix m=a;for(auto&r:m.v)for(auto&x:r)x*=s;return m;}
Matrix CalculatorEngine::identity(int n){Matrix m;m.v.assign(n,std::vector<double>(n,0));for(int i=0;i<n;++i)m.v[i][i]=1;return m;}
Matrix CalculatorEngine::inverse(const Matrix&m){
    int n=(int)m.v.size();if(n==0||m.v[0].size()!=n)throw std::runtime_error("정사각행렬만 역행렬을 구할 수 있습니다");
    Matrix a=m, inv=identity(n);
    for(int c=0;c<n;++c){int p=c;for(int r=c;r<n;++r)if(std::abs(a.v[r][c])>std::abs(a.v[p][c]))p=r;
        if(std::abs(a.v[p][c])<1e-12)throw std::runtime_error("역행렬이 존재하지 않습니다");
        std::swap(a.v[c],a.v[p]);std::swap(inv.v[c],inv.v[p]);double d=a.v[c][c];
        for(int j=0;j<n;++j){a.v[c][j]/=d;inv.v[c][j]/=d;}
        for(int r=0;r<n;++r)if(r!=c){double f=a.v[r][c];for(int j=0;j<n;++j){a.v[r][j]-=f*a.v[c][j];inv.v[r][j]-=f*inv.v[c][j];}}
    } return inv;
}
Matrix CalculatorEngine::matPow(const Matrix&m,double power){
    if(power!=std::trunc(power))throw std::runtime_error("행렬의 지수는 정수여야 합니다");
    int e=(int)power;if(e<0)return matPow(inverse(m),-e);Matrix r=identity((int)m.v.size()),b=m;
    while(e){if(e&1)r=matMul(r,b);b=matMul(b,b);e>>=1;}return r;
}
std::vector<double> CalculatorEngine::dedupe(const std::vector<double>&a){std::vector<double>r;for(double x:a)if(std::none_of(r.begin(),r.end(),[x](double y){return std::abs(x-y)<1e-9;}))r.push_back(x);return r;}
std::string CalculatorEngine::typeName(ValueType t){switch(t){case ValueType::Number:return"실수";case ValueType::Set:return"집합";case ValueType::Matrix:return"행렬";case ValueType::Sequence:return"수열";}return"";}
static std::string F(double x){if(std::abs(x)<1e-12)x=0;std::ostringstream o;o<<std::setprecision(9)<<std::defaultfloat<<x;return o.str();}
std::string CalculatorEngine::formatMatrix(const Matrix&m){if(m.v.size()==1){std::string s="[";for(size_t j=0;j<m.v[0].size();++j){if(j)s+=", ";s+=F(m.v[0][j]);}return s+"]";}std::string s="[";for(size_t i=0;i<m.v.size();++i){if(i)s+=", ";s+="[";for(size_t j=0;j<m.v[i].size();++j){if(j)s+=", ";s+=F(m.v[i][j]);}s+="]";}return s+"]";}
std::string CalculatorEngine::format(const Value&v){
    if(v.type==ValueType::Number)return F(v.number);
    if(v.type==ValueType::Sequence){std::string s="(";for(size_t i=0;i<v.sequence.size();++i){if(i)s+=", ";s+=F(v.sequence[i]);}return s+")";}
    if(v.type==ValueType::Set){std::string s="{";for(size_t i=0;i<v.set.size();++i){if(i)s+=", ";s+=F(v.set[i]);}return s+"}";}
    return formatMatrix(v.matrix);
}

// Correct matrix addition separately.
}
