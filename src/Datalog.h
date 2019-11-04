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

namespace datalog
{

using namespace std;

template <typename T>
struct Symbol : optional<T>
{
	//typedef shared_ptr<Symbol> Type;

	void bind(const T &value)
	{
		this->emplace(value);
	}

	void unbind()
	{
		this->reset();
	}

	bool isBound() const
	{
		return this->has_value();
	}

	const T &
	value() const
	{
		return this->optional<T>::value();
	}
};

template <typename T>
struct SymbolOrValue : public variant<T, Symbol<T>*>
{
	typedef Symbol<T>* SymbolType;

	bool isSym() const
	{
		return holds_alternative<SymbolType>(*this);
	}

	const SymbolType
	getSym() const
	{
		return get<SymbolType>(*this);
	}

	const T &
	getVal() const
	{
		return get<T>(*this);
	}
};

template <typename T>
static SymbolOrValue<T> sym(Symbol<T>& symbol) {
	return SymbolOrValue<T>{ &symbol };
}

template <typename T>
static ostream &
operator<<(ostream &out, const SymbolOrValue<T> &s)
{
	if (s.isSym())
	{
		out << s.getSym();
	}
	else
	{
		out << s.getVal();
	}
	return out;
}

template <typename... Ts>
struct Relation : tuple<Ts...>
{
	typedef tuple<Ts...> TupleType;
	typedef tuple<SymbolOrValue<Ts>...> Atom;
	using tuple<Ts...>::tuple;
	struct compare
	{
		bool operator()(const Relation &lhs, const Relation &rhs) const
		{
			return lhs < rhs;
		}
	};
	typedef set<Relation, compare> Set;
};

#if 0
template <std::size_t I, typename ...Ts>
decltype(auto) get(Relation<Ts...>&& v)
{
    return std::get<I>(static_cast<std::tuple<Ts...>&&>(v));
}
template <std::size_t I, typename ...Ts>
decltype(auto) get(Relation<Ts...>& v)
{
    return std::get<I>(static_cast<std::tuple<Ts...>&>(v));
}
template <std::size_t I, typename ...Ts>
decltype(auto) get(Relation<Ts...> const& v)
{
    return std::get<I>(static_cast<std::tuple<Ts...> const&>(v));
}
#endif

template <typename TUPLE_TYPE>
static ostream &
print(ostream &out, const TUPLE_TYPE &s)
{
	out << " [";
	apply([&out](auto &&... args) { ((out << " [" << args << "] "), ...); }, s);
	out << "] ";
	return out;
}

template <typename RELATION_TYPE, size_t... Is>
static void unbind(const typename RELATION_TYPE::Atom &tuple,
				   index_sequence<Is...>)
{
	// TODO: why can't we use the apply pattern everywhere?
	((get<Is>(tuple).getSym()->unbind()), ...);
}

template <typename... Ts>
static void unbind(const tuple<SymbolOrValue<Ts>...> &tuple)
{
	auto indexSequence = make_index_sequence<
		tuple_size<typename Relation<Ts...>::Atom>::value>{};
	unbind<Relation<Ts...>>(tuple, indexSequence);
}

// bind 1 SymbolOrValue with 1 Value
template <unsigned int I, typename VALUE_TYPE>
bool bind(SymbolOrValue<VALUE_TYPE> &s, const VALUE_TYPE &v)
{
	if (s.isSym())
	{
		Symbol<VALUE_TYPE> &symbol = *s.getSym();
		// has the symbol already been bound?
		if (symbol.isBound())
		{
			// is it a consistent binding?
			if (!(symbol.value() == v))
			{
				return false;
			}
		}
		symbol.bind(v);
	}
	return true;
}

template <typename ATOM_TYPE, typename GROUND_TYPE, size_t... Is>
static bool bind(ATOM_TYPE &atom, const GROUND_TYPE &fact, index_sequence<Is...>)
{
	bool success = ((bind<Is>(get<Is>(atom), get<Is>(fact))) and ...);
	if (success)
	{
		{
			cout << "bound ";
			print(cout, atom);
			cout << " to ";
			print(cout, fact);
			cout << endl;
		}
	}
	else
	{
		cout << "failed to bind ";
		print(cout, atom);
		cout << " with ";
		print(cout, fact);
		cout << endl;
	}
	return success;
}

// bind 1 atom with 1 fact (of the same relation type)
#if 0
template <typename ATOM_TYPE, typename GROUND_TYPE>
static bool bind(ATOM_TYPE &atom, const GROUND_TYPE &fact)
{
	//unbind<RELATION_TYPE>(atom);
	constexpr size_t tupleSize = tuple_size<typename GROUND_TYPE::TupleType>::value;
	return bind<ATOM_TYPE, typename GROUND_TYPE::TupleType>(atom, fact, make_index_sequence<tupleSize>{});
}
#else
template <typename RELATION_TYPE>
static bool bind(typename RELATION_TYPE::Atom &atom, const RELATION_TYPE &fact)
{
	//unbind<RELATION_TYPE>(atom);
	constexpr size_t tupleSize = tuple_size<typename RELATION_TYPE::TupleType>::value;
	return bind<typename RELATION_TYPE::Atom, typename RELATION_TYPE::TupleType>(atom, fact, make_index_sequence<tupleSize>{});
}
#endif

#if 0
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
#endif

template <typename HEAD_RELATION, typename... BODY_RELATIONs>
struct Rule
{
	typedef Rule Define;
	typedef typename HEAD_RELATION::Atom HeadType;
	HeadType head;
	typedef tuple<BODY_RELATIONs...> BodyRelations;
	typedef tuple<typename BODY_RELATIONs::Atom...> BodyType;
	BodyType body;
};

template <typename... RELATIONs>
struct State
{
	typedef tuple<typename RELATIONs::Set...> RelationsType;
	RelationsType relations;
	typedef tuple<const RELATIONs*...> SliceType;
	typedef tuple<typename RELATIONs::Set::const_iterator...> IteratorsArrayType;

	template <typename TUPLE_TYPE>
	static ostream &
	print(ostream &out, TUPLE_TYPE const *p)
	{
		if (p)
		{
			datalog::print<typename TUPLE_TYPE::TupleType>(out, *p);
		}
		return out;
	}

	static ostream &
	print(ostream &out, const SliceType &slice)
	{
		apply([&out](auto &&... args) { ((print(out, args), ...)); }, slice);
		return out;
	}

	struct Iterator
	{
		Iterator(const RelationsType &relations) : relations(relations), iterators(initIterators(relations))
		{
		}

	private:
		template <size_t I, typename RELATION_TYPE>
		void pick(const typename RELATION_TYPE::Set &relation,
				  const RELATION_TYPE*&sliceElement)
		{
			const auto &it = get<I>(iterators);
			if (it != relation.end())
			{
				// TODO: avoid cast if possible
				sliceElement = reinterpret_cast<const RELATION_TYPE*>(&*it);
			}
			else
			{
				sliceElement = nullptr;
			}
		}

		template <size_t... Is>
		void pick(const RelationsType &relations, SliceType &slice,
				  index_sequence<Is...>)
		{
			//cout << "[" << endl;
			((pick<Is>(get<Is>(relations), get<Is>(slice))), ...);
			//cout << "]" << endl;
		}

		template <size_t I>
		bool next(const RelationsType &relations, IteratorsArrayType &iterators,
				  bool &stop)
		{
			bool iterationFinished = false;
			if (not stop)
			{
				auto &it = get<I>(iterators);
				const auto &end = get<I>(relations).end();
				if (it != end)
					it++;
				if (it == end)
				{
					it = get<I>(relations).begin();
					if (I == tuple_size<RelationsType>::value - 1)
					{
						iterationFinished = true;
					}
				}
				else
				{
					stop = true;
				}
			}
			return iterationFinished;
		}

		template <size_t... Is>
		bool next(const RelationsType &relations, IteratorsArrayType &iterators,
				  index_sequence<Is...>)
		{
			bool stop = false;
			return ((next<Is>(relations, iterators, stop)) or ...);
		}

	public:
		bool hasNext() const
		{
			return not iterationFinished;
		}

		SliceType next()
		{
			SliceType slice;
			auto indexSequence = make_index_sequence<tuple_size<RelationsType>::value>{};
			pick(relations, slice, indexSequence);
			iterationFinished = next(relations, iterators, indexSequence);
			{
				cout << "slice = ";
				print(cout, slice);
				cout << endl;
			}
			return slice;
		}

	private:
		const RelationsType &relations;
		IteratorsArrayType iterators;
		bool iterationFinished = false;

		// RELATION_TYPE is really the set
		template <typename RELATION_TYPE, typename ITERATOR_TYPE>
		static void initIterator(const RELATION_TYPE &relation, ITERATOR_TYPE &iterator)
		{
			iterator = relation.begin();
		}

		template <size_t... Is>
		static void initIterators(const RelationsType &relations,
								  IteratorsArrayType &iterators, index_sequence<Is...>)
		{
			((initIterator(get<Is>(relations), get<Is>(iterators))), ...);
		}

		static IteratorsArrayType initIterators(const RelationsType &relations)
		{
			IteratorsArrayType iterators;
			initIterators(relations, iterators,
						  make_index_sequence<tuple_size<IteratorsArrayType>::value>{});
			return iterators;
		}
	};

	Iterator iterator() const
	{
		Iterator it{relations};
		return it;
	}
};

template <typename RULE_TYPE, size_t... Is>
static void unbind(const typename RULE_TYPE::BodyType &atoms,
				   index_sequence<Is...>)
{
	((unbind(get<Is>(atoms))), ...);
}

template <typename RULE_TYPE>
static void unbind(const typename RULE_TYPE::BodyType &atoms)
{
	unbind<RULE_TYPE>(atoms,
					  make_index_sequence<tuple_size<typename RULE_TYPE::BodyType>::value>{});
}

#if 0
// bind 1 atom with 1 fact (of the same relation type)
template <typename RELATION_TYPE>
static bool bind(typename RELATION_TYPE::Atom &atom,
				 const RELATION_TYPE &fact)
{
	//unbind<RELATION_TYPE>(atom);
	//constexpr size_t tupleSize = tuple_size<typename RELATION_TYPE::Type>::value;
	constexpr size_t tupleSize = tuple_size<typename RELATION_TYPE::TupleType>::value;
	return bind<RELATION_TYPE>(atom, fact, make_index_sequence<tupleSize>{});
}
#endif

template <size_t I, typename RULE_TYPE, typename... RELATIONs>
static void bind(typename RULE_TYPE::BodyType &atoms, // typedef tuple<typename BODY_RELATIONs::Atom...> BodyType;
				 const tuple<RELATIONs const *...> &slice)
{
	// get the fact in the slice that can potentially bind
	typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type BodyRelationI;
	auto factPtr = get<BodyRelationI const *>(slice);
	const BodyRelationI& fact = *factPtr;
	// get the atom
	typename BodyRelationI::Atom &atom = get<I>(atoms);
	// try to bind the atom with the fact
	bind(atom, fact);
}

template <typename RULE_TYPE, typename... RELATIONs, size_t... Is>
static void bind(typename RULE_TYPE::BodyType &atoms,
				 const tuple<RELATIONs const *...> &slice, index_sequence<Is...>)
{
	((bind<Is, RULE_TYPE, RELATIONs...>(atoms, slice)), ...);
}

// bind n atoms (of multiple relation types) with 1 slice (of multiple relation types)
template <typename RULE_TYPE, typename... RELATIONs>
static bool bind(typename RULE_TYPE::BodyType &atoms,
				 const tuple<RELATIONs const *...> &slice)
{
	// unbind all the symbols
	unbind<RULE_TYPE>(atoms);
	// for each atom, bind with corresponding relation type in slice
	bind<RULE_TYPE, RELATIONs...>(atoms, slice,
								  make_index_sequence<tuple_size<typename RULE_TYPE::BodyType>::value>{});
	return false;
}

// apply a rule to state
template <typename RULE_TYPE, typename... RELATIONs>
static void apply(RULE_TYPE &rule, const State<RELATIONs...> &state)
{
	auto it = state.iterator();
	while (it.hasNext())
	{
		auto slice = it.next();
		// slice is type typedef tuple<typename RELATIONs::GroundType const* ...> SliceType;
		/*bool success = */ bind<RULE_TYPE, RELATIONs...>(rule.body, slice);
	}
}

#if 0
template<typename RELATION_TYPE>
static bool apply(
const typename RELATION_TYPE::AtomicType& atom,
const RELATION_TYPE& relation)
{
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
)
{
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

// bind a rule body to a tuple of relations
template<typename RULE_TYPE, typename ... RELATIONs>
static void apply(
const typename RULE_TYPE::BodyType& body,
const typename State<RELATIONs...>::RelationsType& relations
)
{
apply<RULE_TYPE, RELATIONs...>(body, relations, make_index_sequence<tuple_size<typename RULE_TYPE::BodyType>::value>
{});
}
#endif

#if 0
template<typename RELATION_TYPE>
static RELATION_TYPE merge(const RELATION_TYPE& r1, const RELATION_TYPE& r2)
{
set<typename RELATION_TYPE::GroundType> tuples
{r1.tuples};
tuples.insert(r2.tuples.begin(), r2.tuples.end());
return RELATION_TYPE
{tuples};
}
#endif

} // namespace datalog

#endif /* SRC_DATALOG_H_ */
