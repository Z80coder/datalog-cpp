#ifndef SRC_DATALOG_H_
#define SRC_DATALOG_H_

#include <tuple>
#include <set>
#include <variant>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <limits>
#include <cassert>
#include <iostream>

namespace datalog {

using namespace std;

template<typename T>
struct Symbol : optional<T> {

    void bind(const T& value) {
        this->emplace(value);
    }

    void unbind() {
        this->reset();
    }

    bool isBound() const {
        return this->has_value();
    }

    const T& value() const {
        return this->optional<T>::value();
    }
};

template<typename T>
struct SymbolOrValue : public variant<T, shared_ptr<Symbol<T>>> {

    typedef shared_ptr<Symbol<T>> SymbolType;

    bool isSym() const {
        return holds_alternative<SymbolType>(*this);
    }

    const shared_ptr<Symbol<T>>& getSym() const {
        return get<SymbolType>(*this);
    }

    const T& getVal() const {
        return get<T>(*this);
    }
};

template<typename ... Ts>
struct Relation {
    typedef tuple<Ts...> GroundType;
    typedef tuple<SymbolOrValue<Ts> ...> AtomicType;
    Relation(const set<GroundType>& tuples) :
            tuples(tuples) {
    }
    const set<GroundType> tuples;
};

template<typename HEAD_RELATION, typename ... BODY_RELATIONs>
struct Rule {
    const typename HEAD_RELATION::AtomicType head;
    const tuple<typename BODY_RELATIONs::AtomicType...> body;
};

template<typename RELATION_TYPE, size_t ... Is>
static void unbind(const typename RELATION_TYPE::AtomicType& tuple, index_sequence<Is...>) {
    ((get<Is>(tuple).getSym()->unbind()), ...);
}

template<typename RELATION_TYPE>
static void unbind(const typename RELATION_TYPE::AtomicType& tuple) {
    auto indexSequence = make_index_sequence<tuple_size<typename RELATION_TYPE::AtomicType>::value> { };
    unbind<RELATION_TYPE>(tuple, indexSequence);
}

// bind 1 SymbolOrValue with 1 Value
template<unsigned int I, typename VALUE_TYPE>
bool bind(SymbolOrValue<VALUE_TYPE>& s, const VALUE_TYPE& v) {
    if (s.isSym()) {
        Symbol<VALUE_TYPE>& symbol = *s.getSym().get();
        // has the symbol already been bound?
        if (symbol.isBound()) {
            // is it a consistent binding?
            if (!(symbol.value() == v)) {
                return false;
            }
        }
        symbol.bind(v);
        cout << "bound " << s.getSym() << " to " << v << endl;
    }
    return true;
}

template<typename RELATION_TYPE, size_t ... Is>
static optional<typename RELATION_TYPE::AtomicType> bind(
        const typename RELATION_TYPE::AtomicType& atom,
        const typename RELATION_TYPE::GroundType& fact,
        index_sequence<Is...>
) {
    typename RELATION_TYPE::AtomicType boundAtom { atom };
    unbind<RELATION_TYPE>(boundAtom);
    bool successfulBinding = ((bind<Is>(get<Is>(boundAtom), get<Is>(fact))) and ...);
    if (successfulBinding) {
        return boundAtom;
    }
    return nullopt;
}

// bind 1 atom with 1 fact
template<typename RELATION_TYPE>
static optional<typename RELATION_TYPE::AtomicType> bind(
        const typename RELATION_TYPE::AtomicType& atom,
        const typename RELATION_TYPE::GroundType& fact
) {
    auto indexSequence = make_index_sequence<tuple_size<typename RELATION_TYPE::GroundType>::value> { };
    return bind<RELATION_TYPE>(atom, fact, indexSequence);
}

// bind 1 atom with a relation (n facts)
#if 0
template<typename RELATION_TYPE>
static Relation<RELATION_TYPE> bind(const Atom<RELATION_TYPE>& atom, const Relation<RELATION_TYPE>& relation) {
    set<typename Relation<RELATION_TYPE>::GroundType> outputTuples;
    for (const auto& fact : relation.tuples) {
        const auto& boundAtom = bind(atom, fact);
        if (boundAtom.has_value()) {
//            outputTuples.insert(boundAtom.value());
        }
    }
    return Relation<RELATION_TYPE>{outputTuples};
}
#else

#if 0
template<typename RELATION_TYPE>
static Relation<RELATION_TYPE> bind(const typename RELATION_TYPE::AtomicType& atom, const Relation<RELATION_TYPE>& relation) {
    set<typename Relation<RELATION_TYPE>::GroundType> outputTuples;
    for (const auto& fact : relation.tuples) {
//        const auto& boundAtom = bind(atom, fact);
//        if (boundAtom.has_value()) {
//            outputTuples.insert(boundAtom.value());
//        }
    }
    return Relation<RELATION_TYPE>{outputTuples};
}
#endif

#endif

template<typename RELATION_TYPE>
static RELATION_TYPE merge(const RELATION_TYPE& r1, const RELATION_TYPE& r2) {
    set<typename RELATION_TYPE::GroundType> tuples { r1.tuples };
    tuples.insert(r2.tuples.begin(), r2.tuples.end());
    return RELATION_TYPE { tuples };
}

}

#endif /* SRC_DATALOG_H_ */

#if 0
// bind 1 SymbolOrValue with 1 Value
template<unsigned int I, typename VALUE_TYPE, typename VARIANT_TYPE>
bool bind(SymbolOrValue<VALUE_TYPE>& s, const VALUE_TYPE& v, map<string, VARIANT_TYPE>& bindings) {
    if (s.isSym()) {
        const auto& symbolName = s.getSym().name;
        // Has this symbol already been bound?
        if (bindings.count(symbolName) > 0) {
            // Is this a consistent binding?
#if 0
            const auto& boundVariant = bindings.at(symbolName);
            const auto& boundVariantIndex = boundVariant.index();
            const auto& boundValue = get<boundVariantIndex>(boundVariant);
            if (!(boundValue == v)) {
                return false;
            }
#endif
        }
        s.bindVal(v);
        VARIANT_TYPE vt(in_place_index_t<I> { }, v);
        cout << "Recording binding at index " << I << endl;
        bindings.insert( { symbolName, vt });
        cout << "Bound " << s.getSym().name << " to " << v << endl;
    }
    return true;
}

// bind 1 atom with 1 fact
template<typename RELATION_TYPE, size_t ... Is>
static optional<Atom<RELATION_TYPE>> bind(const Atom<RELATION_TYPE>& atom, const typename RELATION_TYPE::GroundType& fact, index_sequence<Is...>) {
    typename RELATION_TYPE::AtomicType boundAtom { atom.tuple };
    map<string, typename RELATION_TYPE::VariantType> bindings;
    bool successfulBinding = ((bind<Is>(get<Is>(boundAtom), get<Is>(fact), bindings)) and ...);
    if (successfulBinding) {
        return Atom<RELATION_TYPE> { boundAtom };
    }
    return nullopt;
}
#endif

#if 0
typedef map<string, set<unsigned int>> SymBindingsType;

template<typename T>
static void bind(const T& t, SymBindingsType& symBindings) {
}

static void noOp() {
}
#endif

#if 0
        unsigned int index = 0;
        apply([this, &index](auto&&... args) {
            ((args.isSym() ? declareSym(args, index++, symIndexMap) : noOp(index++)), ...);}, tuple);
#endif

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

#if 0

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
