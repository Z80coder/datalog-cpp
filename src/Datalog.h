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

template<typename TUPLE_TYPE>
static ostream& print(ostream& out, const TUPLE_TYPE& s)
{
    out << " [";
    apply([&out](auto&&... args) {(( out << " [" << args << "] "), ...);}, s);
    out << "] ";
    return out;
}

template<typename RELATION_TYPE, size_t ... Is>
static void unbind(const typename RELATION_TYPE::AtomicType& tuple, index_sequence<Is...>) {
    // TODO: why can't we use the apply pattern everywhere?
    ((get<Is>(tuple).getSym()->unbind()), ...);
}

template<typename RELATION_TYPE>
static void unbind(const typename RELATION_TYPE::AtomicType& tuple) {
    unbind<RELATION_TYPE>(tuple, make_index_sequence<tuple_size<typename RELATION_TYPE::AtomicType>::value> { });
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
            print(cout, atom);
            cout << " to ";
            print(cout, boundAtom);
            cout << endl;
        }
        return boundAtom;
    }
    {
        cout << "failed to bind ";
        print(cout, atom);
        cout << " with ";
        print(cout, fact);
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
    return bind<RELATION_TYPE>(atom, fact, make_index_sequence<tuple_size<typename RELATION_TYPE::GroundType>::value> { });
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

template<typename HEAD_RELATION, typename ... BODY_RELATIONs>
struct Rule {
    const typename HEAD_RELATION::AtomicType head;
    typedef tuple<typename BODY_RELATIONs::AtomicType...> BodyType;
    const BodyType body;
};

template<typename ... RELATIONs>
struct State {
   typedef tuple<RELATIONs...> RelationsType;
   RelationsType relations;
   typedef tuple<typename RELATIONs::GroundType const* ...> SliceType;

   static ostream& print(ostream& out, const SliceType& slice) {
       apply([&out](auto&&... args) {(( datalog::print(out, *args), ...));}, slice);
       return out;
   }

   struct Iterator {
      Iterator(const RelationsType& relations) : sizes(relationSizes(relations)) {
      }

      bool hasNext(const State& state) const {
          return not iterationFinished;
      }

      template<typename RELATION_TYPE>
      void pick(
          const RELATION_TYPE& relation,
          typename RELATION_TYPE::GroundType const*& sliceElement,
          unsigned int relationIndex
      ) {
         //cout << "pick relation " << relationIndex << " with " << index[relationIndex] << endl;
         // TODO: inefficient
         sliceElement = &*std::next(relation.tuples.begin(), index[relationIndex]);
      }

      template<size_t ... Is>
      void pick(const RelationsType& relations, SliceType& slice, index_sequence<Is...>) {
          //cout << "[" << endl;
         ((pick(get<Is>(relations), get<Is>(slice), Is)), ...);
         //cout << "]" << endl;
      }

      // Returns true if no more combinations of facts to iterate over
      bool next(size_t masterIndex, bool exhausted) {
         if (masterIndex >= tuple_size<RelationsType>::value) {
             // no more relations
             return exhausted and true;
         }
         // increase index of this relation
         index[masterIndex]++;
         // have we exhausted this relation?
         if (index[masterIndex] >= sizes[masterIndex]) {
            // then reset this relation, and increase index of next relation
            index[masterIndex] = 0;
            return exhausted and next(masterIndex + 1, exhausted);
         }
         return false;
      }

      SliceType next(const State& state) {
         SliceType slice;
         pick(state.relations, slice, make_index_sequence<tuple_size<RelationsType>::value> { });
         iterationFinished = next(0, true);
         cout << "slice = ";
         print(cout, slice);
         cout << endl;
         return slice;
      }

   private:
      const array<unsigned int, tuple_size<RelationsType>::value> sizes = {0};
      array<unsigned int, tuple_size<RelationsType>::value> index = {0};
      bool iterationFinished = false;

      template<size_t ... Is>
      static void relationSizes(
              const RelationsType& relations,
              array<unsigned int, tuple_size<RelationsType>::value>& sizes,
              index_sequence<Is...>
      ) {
          ((sizes[Is] = get<Is>(relations).tuples.size()), ...);
      }

      static array<unsigned int, tuple_size<RelationsType>::value> relationSizes(const RelationsType& relations) {
          array<unsigned int, tuple_size<RelationsType>::value> sizes;
          relationSizes(relations, sizes, make_index_sequence<tuple_size<RelationsType>::value> { });
          return sizes;
      }

   };

   Iterator iterator() const {
       Iterator it{relations};
       return it;
   }


};

template<typename RELATION_TYPE>
static bool apply(
      const typename RELATION_TYPE::AtomicType& atom,
      const RELATION_TYPE& relation) {
   // we have the atom, and some corresponding facts, now bind them
   // but we don't want the ground atoms
   // we want the bound atoms

   // to avoid copying symbols we should try and find a complete binding across all relations,
   // and then move to next complete binding etc.

   // so basically we want to stream all the combinations of facts, where each combination
   // consists of selecting 1 fact from each relation in the state
   return true;
}

template<typename RULE_TYPE, typename ... RELATIONs, size_t ... Is>
static void apply(
      const typename RULE_TYPE::BodyType& body,
      const typename State<RELATIONs...>::RelationsType& relations,
      index_sequence<Is...>
) {
   /*
    * Bind n atoms (of different relation types, such as the body of a rule)
    * with state consisting of
    * n_{i} facts (of different relation types i = 1 ... m)
    */
   // for each atom in body find and bind with corresponding relation
   bool success = ((
         apply<typename tuple_element<Is, typename State<RELATIONs...>::RelationsType>::type>(
               get<Is>(body),
               get<typename tuple_element<Is, typename State<RELATIONs...>::RelationsType>::type>(relations)
               )) and ...);
}

template<typename RULE_TYPE, typename ... RELATIONs>
static void apply(
      const typename RULE_TYPE::BodyType& body,
      const typename State<RELATIONs...>::RelationsType& relations
) {
   apply<RULE_TYPE, RELATIONs...>(body, relations, make_index_sequence<tuple_size<typename RULE_TYPE::BodyType>::value> { });
}

template<typename RULE_TYPE, typename ... RELATIONs>
static void apply(const RULE_TYPE& rule, const State<RELATIONs...>& state) {
   apply<RULE_TYPE, RELATIONs...>(rule.body, state.relations);
}


#if 0
template<typename RELATION_TYPE>
static RELATION_TYPE merge(const RELATION_TYPE& r1, const RELATION_TYPE& r2) {
    set<typename RELATION_TYPE::GroundType> tuples { r1.tuples };
    tuples.insert(r2.tuples.begin(), r2.tuples.end());
    return RELATION_TYPE { tuples };
}
#endif

}

#endif /* SRC_DATALOG_H_ */
