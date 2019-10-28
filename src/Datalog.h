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

template<typename T>
static ostream& operator<<(ostream& out, const SymbolOrValue<T>& s)
{
    if (s.isSym()) {
        out << s.getSym();
    } else {
        out << s.getVal();
    }
    return out;
}

template<typename ... Ts>
struct Relation {
    typedef tuple<Ts...> GroundType;
    typedef tuple<SymbolOrValue<Ts> ...> AtomicType;
    const set<GroundType> tuples;
};

template<typename RELATION_TYPE>
static ostream& print(ostream& out, const typename RELATION_TYPE::GroundType& s)
{
    apply([&out](auto&&... args) {(( out << "[" << args << "] "), ...);}, s);
    return out;
}

template<typename RELATION_TYPE>
static ostream& print(ostream& out, const typename RELATION_TYPE::AtomicType& s)
{
    apply([&out](auto&&... args) {(( out << "[" << args << "] "), ...);}, s);
    return out;
}

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
bool bind(SymbolOrValue<VALUE_TYPE>& s, const VALUE_TYPE& v, VALUE_TYPE& b) {
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
        b = v;
    }
    return true;
}

template<typename RELATION_TYPE, size_t ... Is>
static optional<typename RELATION_TYPE::GroundType> bind(
        typename RELATION_TYPE::AtomicType& atom,
        const typename RELATION_TYPE::GroundType& fact,
        index_sequence<Is...>
) {
    unbind<RELATION_TYPE>(atom);
    typename RELATION_TYPE::GroundType boundAtom;
    bool successfulBinding = ((bind<Is>(get<Is>(atom), get<Is>(fact), get<Is>(boundAtom))) and ...);
    if (successfulBinding) {
        {
            cout << "bound ";
            print<RELATION_TYPE>(cout, atom);
            cout << " to ";
            print<RELATION_TYPE>(cout, boundAtom);
            cout << endl;
        }
        return boundAtom;
    }
    {
        cout << "failed to bind ";
        print<RELATION_TYPE>(cout, atom);
        cout << " with ";
        print<RELATION_TYPE>(cout, fact);
        cout << endl;
    }
    return nullopt;
}

// bind 1 atom with 1 fact
template<typename RELATION_TYPE>
static optional<typename RELATION_TYPE::GroundType> bind(
        typename RELATION_TYPE::AtomicType& atom,
        const typename RELATION_TYPE::GroundType& fact
) {
    auto indexSequence = make_index_sequence<tuple_size<typename RELATION_TYPE::GroundType>::value> { };
    return bind<RELATION_TYPE>(atom, fact, indexSequence);
}

// bind 1 atom with a relation (n facts)
template<typename RELATION_TYPE>
static RELATION_TYPE bind(
        typename RELATION_TYPE::AtomicType& atom,
        const RELATION_TYPE& relation
) {
    set<typename RELATION_TYPE::GroundType> outputTuples;
    for (auto& fact : relation.tuples) {
        const auto& boundAtom = bind<RELATION_TYPE>(atom, fact);
        if (boundAtom.has_value()) {
            outputTuples.insert(boundAtom.value());
        }
    }
    return RELATION_TYPE{outputTuples};
}

template<typename RELATION_TYPE>
static RELATION_TYPE merge(const RELATION_TYPE& r1, const RELATION_TYPE& r2) {
    set<typename RELATION_TYPE::GroundType> tuples { r1.tuples };
    tuples.insert(r2.tuples.begin(), r2.tuples.end());
    return RELATION_TYPE { tuples };
}

}

#endif /* SRC_DATALOG_H_ */
