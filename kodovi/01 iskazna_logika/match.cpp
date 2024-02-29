#include <iostream>
#include <memory>
#include <map>
#include <variant>

struct False; struct True; struct Atom; struct Not; struct Binary;
using Formula = std::variant<False, True, Atom, Not, Binary>;
using FormulaPtr = std::shared_ptr<Formula>;

struct False  { };
struct True   { };
struct Atom   { std::string name; };
struct Not    { FormulaPtr f; };
struct Binary {
    enum Type { And, Or, Impl, Eql } type;
    FormulaPtr l, r;
};

FormulaPtr ptr(Formula f) { return std::make_shared<Formula>(f); }

template<typename T>
bool is(FormulaPtr formula) { return std::holds_alternative<T>(*formula); }

template<typename T>
T& as(FormulaPtr formula) { return std::get<T>(*formula); }

template<typename R,
         typename FalseT,
         typename TrueT,
         typename AtomT,
         typename NotT,
         typename BinaryT,
         typename... ArgsT>
R match(FormulaPtr formula,
        FalseT falseF,
        TrueT trueF,
        AtomT atomF,
        NotT notF,
        BinaryT binaryF,
        ArgsT&&... args)
{
    if(is<False>(formula))
        return falseF(as<False>(formula), std::forward<ArgsT>(args)...);
    if(is<True>(formula))
        return trueF(as<True>(formula), std::forward<ArgsT>(args)...);
    if(is<Atom>(formula))
        return atomF(as<Atom>(formula), std::forward<ArgsT>(args)...);
    if(is<Not>(formula))
        return notF(as<Not>(formula), std::forward<ArgsT>(args)...);
    if(is<Binary>(formula))
        return binaryF(as<Binary>(formula), std::forward<ArgsT>(args)...);
    return R();
}

unsigned complexity(FormulaPtr);
unsigned complexityF(False)    { return 0; }
unsigned complexityT(True)     { return 0; }
unsigned complexityA(Atom)     { return 0; }
unsigned complexityN(Not d)    { return 1 + complexity(d.f); }
unsigned complexityB(Binary d) { return 1 + complexity(d.l) + complexity(d.r); }
unsigned complexity(FormulaPtr formula) {
    return match<unsigned>(formula, complexityF, complexityT, complexityA, complexityN, complexityB);
}

void varcount(FormulaPtr, int&);
void varcountF(False, int&) { }
void varcountT(True, int&)  { }
void varcountA(Atom, int& c)  { c++; }
void varcountN(Not d, int& c) { varcount(d.f, c); }
void varcountB(Binary d, int& c) { varcount(d.l, c); varcount(d.r, c); }
void varcount(FormulaPtr formula, int& c) {
    return match<void>(formula, varcountF, varcountT, varcountA, varcountN, varcountB, c);
}

std::string sign(Binary::Type op) {
    switch(op) {
        case Binary::And:  return " & ";
        case Binary::Or:   return " | ";
        case Binary::Impl: return " -> ";
        case Binary::Eql:  return " <-> ";
    }
    return "";
}
void print(FormulaPtr);
void printF(False)    { std::cout << "F"; }
void printT(True)     { std::cout << "T"; }
void printA(Atom f)   { std::cout << f.name; }
void printN(Not f)    { std::cout << "~"; print(f.f); }
void printB(Binary f) {
    std::cout << "(";
    print(f.l);
    std::cout << sign(f.type);
    print(f.r);
    std::cout << ")";
}
void print(FormulaPtr formula) {
    match<void>(formula, printF, printT, printA, printN, printB);
}

bool equal(FormulaPtr, FormulaPtr);
bool equalF(False, FormulaPtr g)  { return is<False>(g); }
bool equalT(True,  FormulaPtr g)  { return is<True>(g); }
bool equalA(Atom f, FormulaPtr g) {
    return is<Atom>(g) && f.name == as<Atom>(g).name;
}
bool equalN(Not f, FormulaPtr g) {
    return is<Not>(g) && equal(f.f, as<Not>(g).f);
}
bool equalB(Binary f, FormulaPtr g) {
    if(!is<Binary>(g))
        return false;
    return f.type == as<Binary>(g).type &&
           equal(f.l, as<Binary>(g).l) &&
           equal(f.r, as<Binary>(g).r);
}
bool equal(FormulaPtr f, FormulaPtr g) {
    return match<bool>(f, equalF, equalT, equalA, equalN, equalB, g);
}

// (p0 & p1) -> ~p2
int main()
{
    FormulaPtr p0 = ptr(Atom{"p0"});
    FormulaPtr p1 = ptr(Atom{"p1"});
    FormulaPtr p2 = ptr(Atom{"p2"});
    FormulaPtr leftF = ptr(Binary{Binary::And, p0, p1});
    FormulaPtr rightF = ptr(Not{p0});
    FormulaPtr F = ptr(Binary{Binary::Impl, leftF, rightF});

    print(F);
    std::cout << std::endl;

    std::cout << complexity(F) << std::endl;
    int c = 0;
    varcount(F, c);
    std::cout << c << std::endl;

    return 0;
}
