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
    enum Type { And, Or, Imp, Eq } type;
    FormulaPtr l, r;
};

FormulaPtr ptr(Formula f) { return std::make_shared<Formula>(f); }

template<typename T>
bool is(FormulaPtr formula) { return std::holds_alternative<T>(*formula); }

template<typename T>
T& as(FormulaPtr formula) { return std::get<T>(*formula); }

template<typename R, typename... Args>
struct Visitor {
    // Mozemo imati i drugacije potpise, tipa
    // visitA(FormulaPtr, Args&&...)
    // ili
    // visitA(std::string, Args&&...)
    // sve u zavisnosti od toga sta nam je zgodnije
    virtual R visitF(False,  Args&&...) = 0;
    virtual R visitT(True,   Args&&...) = 0;
    virtual R visitA(Atom,   Args&&...) = 0;
    virtual R visitN(Not,    Args&&...) = 0;
    virtual R visitB(Binary, Args&&...) = 0;

    R visit(FormulaPtr formula, Args&&... args) {
        if(is<False>(formula))
            return visitF(as<False>(formula), std::forward<Args>(args)...);
        if(is<True>(formula))
            return visitT(as<True>(formula), std::forward<Args>(args)...);
        if(is<Atom>(formula))
            return visitA(as<Atom>(formula), std::forward<Args>(args)...);
        if(is<Not>(formula))
            return visitN(as<Not>(formula), std::forward<Args>(args)...);
        if(is<Binary>(formula))
            return visitB(as<Binary>(formula), std::forward<Args>(args)...);
        return R();
    }
};

/*

Ako se ikad pojave komplikovanije funkcije gde je switch-case nezgodan, mozemo imati i visitora za binary

template<typename R, typename... Args>
struct BinaryVisitor {
    virtual R visitA(FormulaPtr, FormulaPtr, Args&&...) = 0;
    virtual R visitO(FormulaPtr, FormulaPtr, Args&&...) = 0;
    virtual R visitI(FormulaPtr, FormulaPtr, Args&&...) = 0;
    virtual R visitE(FormulaPtr, FormulaPtr, Args&&...) = 0;

    R visit(Binary formula, Args&&... args) {
        switch(formula.type) {
            case Binary::And: return visitA(formula.l, formula.r, std::forward<Args>(args)...);
            case Binary::Or:  return visitO(formula.l, formula.r, std::forward<Args>(args)...);
            case Binary::Imp: return visitI(formula.l, formula.r, std::forward<Args>(args)...);
            case Binary::Eq:  return visitE(formula.l, formula.r, std::forward<Args>(args)...);
        }
        return R();
    }
}
*/

struct Complexity : Visitor<unsigned> {
    unsigned visitF(False)       { return 0; }
    unsigned visitT(True)        { return 0; }
    unsigned visitA(Atom)        { return 0; }
    unsigned visitN(Not node)    { return 1 + visit(node.f); }
    unsigned visitB(Binary node) { return 1 + visit(node.l) + visit(node.r); }
};
unsigned complexity(FormulaPtr formula) {
    Complexity visitor;
    return visitor.visit(formula);
}

struct Varcount : Visitor<void, int&> {
    void visitF(False, int&)         { }
    void visitT(True, int&)          { }
    void visitA(Atom, int& c)        { c++; }
    void visitN(Not node, int& c)    { visit(node.f, c); }
    void visitB(Binary node, int& c) { visit(node.l, c); visit(node.r, c); }
};
void varcount(FormulaPtr formula, int& c) {
    Varcount visitor;
    visitor.visit(formula, c);
}

struct Print : Visitor<void> {
    std::string sign(Binary::Type op) {
        switch(op) {
            case Binary::And:  return " & ";
            case Binary::Or:   return " | ";
            case Binary::Impl: return " -> ";
            case Binary::Eql:  return " <-> ";
        }
        return "";
    }

    void visitF(False)       { std::cout << "F"; }
    void visitT(True)        { std::cout << "T"; }
    void visitA(Atom node)   { std::cout << node.name; }
    void visitN(Not node)    { std::cout << "~"; visit(node.f); }
    void visitB(Binary node) {
        std::cout << "(";
        visit(node.l);
        std::cout << sign(node.type);
        visit(node.r);
        std::cout << ")";
    }
};
void print(FormulaPtr formula) {
    Print visitor;
    visitor.visit(formula);
}

/*
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
*/

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
