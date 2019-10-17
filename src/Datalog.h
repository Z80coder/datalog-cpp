#ifndef SRC_DATALOG_H_
#define SRC_DATALOG_H_

#include <tuple>
#include <set>
#include <variant>
#include <vector>
#include <map>
#include <limits>
#include <cassert>
#include <iostream>

namespace datalog {

using namespace std;

struct Symbol {
    string name;
};

template<typename T>
struct SymbolOrValue {

    SymbolOrValue() :
            isSymbol(false), symbol(Symbol { "" }) {
    }

    SymbolOrValue(const T& value) :
            isSymbol(false), symbol(Symbol { "" }), value(value) {
    }

    SymbolOrValue(const Symbol& symbol) :
            isSymbol(true), symbol(symbol), value() {
    }

    SymbolOrValue(const SymbolOrValue& other) :
            isSymbol(other.isSymbol), symbol(other.symbol), value(other.value) {
    }

    bool isSym() const {
        return this->isSymbol;
    }

    const Symbol& getSym() const {
        return this->symbol;
    }

    const T& getVal() const {
        assert(!isSymbol);
        return value;
    }

    void bindVal(const T& value) {
        this->value = value;
    }

    bool operator <(const SymbolOrValue &other) const {
        return value < other.value;
    }

    SymbolOrValue& operator =(const SymbolOrValue &other) {
        this->isSymbol = other.isSymbol;
        this->symbol = other.symbol;
        this->value = other.value;
        return *this;
    }

private:
    bool isSymbol;
    Symbol symbol;
    T value;
};

template<typename ... Ts>
struct RelationType {
    typedef tuple<Ts...> GroundType;
    typedef tuple<SymbolOrValue<Ts> ...> AtomicType;
};

template<typename RELATION_TYPE>
struct Relation {
    typedef typename RELATION_TYPE::GroundType GroundType;
    Relation(const set<GroundType>& tuples) :
            tuples(tuples) {
    }
    const set<GroundType> tuples;
};

template<typename RELATION_TYPE>
struct Atom {
    typedef typename RELATION_TYPE::AtomicType AtomicType;
    Atom(const AtomicType& tuple) :
            tuple { tuple } {
#if 0
        unsigned int index = 0;
        apply([this, &index](auto&&... args) {
            ((args.isSym() ? declareSym(args, index++, symIndexMap) : noOp(index++)), ...);}, tuple);
#endif
    }
    const AtomicType tuple;
#if 0
    typedef map<string, set<unsigned int>> SymIndexMapType;
    SymIndexMapType symIndexMap;
private:
    template<typename T>
    static void declareSym(const T& t, unsigned int index, SymIndexMapType& symIndexMap) {
        auto symbolName = t.getSym().name;
        if (symIndexMap.count(symbolName) == 0) {
            symIndexMap.insert( { symbolName, { } });
        }
        auto it = symIndexMap.find(symbolName);
        it->second.insert(index);
    }
    static void noOp(unsigned int index) {
    }
#endif
};

template<typename ... ATOMs>
struct Body {
    typedef variant<ATOMs...> AtomsType;
    const vector<AtomsType> atoms;
};

template<typename HEAD_RELATION, typename ... BODY_RELATIONs>
struct Rule {
    const Atom<HEAD_RELATION> head;
    const Body<Atom<BODY_RELATIONs> ...> body;
};

#if 0
typedef map<string, set<unsigned int>> SymBindingsType;

template<typename T>
static void bind(const T& t, SymBindingsType& symBindings) {
}

static void noOp() {
}
#endif

#if 0
template<typename RELATION_TYPE>
constexpr typename RELATION_TYPE::AtomicType bind(const typename RELATION_TYPE::AtomicType& atom, const typename RELATION_TYPE::GroundType& fact, size_t i) {
    typename RELATION_TYPE::AtomicType boundAtom{atom};
    if (get<i>(atom)) {

    }
    return boundAtom;
}

// bind 1 atom with 1 fact
template<typename RELATION_TYPE, size_t ... Is>
static Atom<RELATION_TYPE> bind(const Atom<RELATION_TYPE>& atom, const typename RELATION_TYPE::GroundType& fact, index_sequence<Is...>) {
    typename RELATION_TYPE::AtomicType boundAtom { atom.tuple };

    ((boundAtom = bind<RELATION_TYPE>(boundAtom, fact, Is)), ...);
    //((boundAtom = bind<RELATION_TYPE>(boundAtom, fact, Is)), ...);
    //apply([&boundAtom](auto&&... element) {
    //    ((atomicElement.isSym() ? bind(atomicElement, symBindings) : noOp()), ...);}, fact);

    return Atom<RELATION_TYPE> { boundAtom };
}

template<typename RELATION_TYPE>
static Relation<RELATION_TYPE> bind(const Atom<RELATION_TYPE>& atom, const Relation<RELATION_TYPE>& relation) {
    Relation<RELATION_TYPE> boundAtoms;
    auto indexSequence = index_sequence_for<typename RELATION_TYPE::GroundType> { };
    for (const auto& fact : relation.tuples) {
        typename RELATION_TYPE::GroundType boundAtom = bind(atom, fact, indexSequence);
        boundAtoms.tuples.insert(boundAtom);
    }
    return boundAtoms;
}
#endif

template<typename RELATION_TYPE>
static RELATION_TYPE merge(const RELATION_TYPE& r1, const RELATION_TYPE& r2) {
    set<typename RELATION_TYPE::GroundType> tuples { r1.tuples };
    tuples.insert(r2.tuples.begin(), r2.tuples.end());
    return RELATION_TYPE { tuples };
}

}

#endif /* SRC_DATALOG_H_ */
