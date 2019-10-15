#ifndef SRC_DATALOG_H_
#define SRC_DATALOG_H_

#include <tuple>
#include <set>
#include <variant>
#include <vector>
#include <limits>
#include <cassert>

namespace datalog {

using namespace std;

struct Id {
    const string value;
};

template<typename T>
struct Value {

    Value(const T& value) :
            isVariable(false), value(value), id(Id { "" }) {
    }

    Value(const Id& id) :
            isVariable(true), id(id) {
    }

    bool isVar() const {
        return this->isVariable;
    }

    const T& getVal() const {
        assert(!isVariable);
        return value;
    }

    const unsigned int getVar() const {
        assert(isVariable);
        return id;
    }

    bool operator <(const Value &other) const {
        return value < other.value;
    }

private:
    bool isVariable;
    T value;
    Id id;
};

typedef Value<int> Int;
typedef Value<string> String;

template<typename ... Ts>
struct Relation {
    typedef tuple<Ts...> TupleType;

    Relation(const set<TupleType>& tuples) :
            tuples(tuples) {
    }

    set<TupleType> tuples;

};

template<typename ... Ts>
static Relation<Ts...> merge(const Relation<Ts...>& r1, const Relation<Ts...>& r2) {
    set<tuple<Ts...>> tuples { r1.tuples };
    tuples.insert(r2.tuples.begin(), r2.tuples.end());
    return Relation<Ts...> { tuples };
}

template<typename RELATION>
struct Atom: RELATION {

    Atom(const typename RELATION::TupleType& tuple) :
            RELATION(set<typename RELATION::TupleType> { tuple }) {
    }
};

template<typename ... ATOMs>
struct Body {
    typedef variant<ATOMs...> AtomsType;

    vector<AtomsType> atoms;
};

template<typename HEAD_ATOM, typename ... BODY_ATOMs>
struct Rule {
    HEAD_ATOM head;
    Body<BODY_ATOMs...> body;
};

}

#endif /* SRC_DATALOG_H_ */
