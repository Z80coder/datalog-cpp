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
struct SymbolOrValue : public variant<T, Symbol<T> *>
{
	typedef Symbol<T> *SymbolType;

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
static SymbolOrValue<T> sym(Symbol<T> &symbol)
{
	return SymbolOrValue<T>{&symbol};
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
	out << "[";
	apply([&out](auto &&... args) { ((out << "[" << args << "]"), ...); }, s);
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
template <typename VALUE_TYPE>
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
	bool success = ((bind(get<Is>(atom), get<Is>(fact))) and ...);
	if (success)
	{
		#if 0
		{
			cout << "bound ";
			print(cout, atom);
			cout << " to ";
			print(cout, fact);
			cout << endl;
		}
		#endif
	}
	else
	{
		#if 0
		cout << "failed to bind ";
		print(cout, atom);
		cout << " with ";
		print(cout, fact);
		cout << endl;
		#endif
	}
	return success;
}

template <typename RELATION_TYPE>
static bool bind(typename RELATION_TYPE::Atom &atom, const RELATION_TYPE &fact)
{
	//unbind<RELATION_TYPE>(atom);
	constexpr size_t tupleSize = tuple_size<typename RELATION_TYPE::TupleType>::value;
	return bind<typename RELATION_TYPE::Atom, typename RELATION_TYPE::TupleType>(atom, fact, make_index_sequence<tupleSize>{});
}

template <typename HEAD_RELATION, typename... BODY_RELATIONs>
struct Rule
{
	typedef Rule Define;
	typedef HEAD_RELATION HeadRelationType;
	typedef typename HEAD_RELATION::Atom HeadType;
	HeadType head;
	typedef tuple<BODY_RELATIONs...> BodyRelations;
	typedef tuple<typename BODY_RELATIONs::Atom...> BodyType;
	BodyType body;

	// TODO:
	// You must sub-class to define rules
	private:
//	virtual void forceSublassing() = 0;
};

// TODO: push this through to avoid reinterpret_casts
template<typename RELATION_TYPE>
struct Set {
	typename RELATION_TYPE::Set set;

	ostream & print(ostream &out)
	{
		out << "\"" << typeid(*this).name() << "\"" << endl;
		for (const auto& tuple : set) {
			datalog::print<typename RELATION_TYPE::TupleType>(out, tuple);
			out << endl;
		}
		return out;
	}

};

template <typename... RELATIONs>
struct State
{
	typedef tuple<Set<RELATIONs>...> SetsOfRelationsType;
	SetsOfRelationsType relations;
	typedef tuple<const RELATIONs *...> SliceType;
	typedef tuple<typename RELATIONs::Set::const_iterator...> IteratorsArrayType;

	size_t size() const {
		size_t totalSize = 0;
		auto inc = [](size_t& size, size_t inc) { size += inc; };
		apply([&totalSize, &inc](auto &&... args) { ((inc(totalSize, args.set.size())), ...); }, relations);
		return totalSize;
	}

	ostream & print(ostream &out)
	{
		out << "[";
		apply([&out](auto &&... args) { ((args.print(out)), ...); }, relations);
		out << "] ";
		return out;
	}

	// Next 2 functions for printing slices. TODO: consolidate printing functions
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
		Iterator(const SetsOfRelationsType &relations) : relations(relations), iterators(initIterators(relations))
		{
		}

	private:
		template <size_t I, typename RELATION_TYPE>
		void pick(const typename RELATION_TYPE::Set &relation,
				  const RELATION_TYPE *&sliceElement)
		{
			const auto &it = get<I>(iterators);
			if (it != relation.end())
			{
				// TODO: avoid cast if possible
				sliceElement = reinterpret_cast<const RELATION_TYPE *>(&*it);
			}
			else
			{
				sliceElement = nullptr;
			}
		}

		template <size_t... Is>
		void pick(const SetsOfRelationsType &relations, SliceType &slice,
				  index_sequence<Is...>)
		{
			//cout << "[" << endl;
			//((pick<Is>(get<Is>(relations), get<Is>(slice))), ...);
			((pick<Is>(get<Is>(relations).set, get<Is>(slice))), ...);
			//cout << "]" << endl;
		}

		template <size_t I>
		bool next(const SetsOfRelationsType &relations, IteratorsArrayType &iterators,
				  bool &stop)
		{
			bool iterationFinished = false;
			if (not stop)
			{
				auto &it = get<I>(iterators);
				const auto &end = get<I>(relations).set.end();
				if (it != end)
					it++;
				if (it == end)
				{
					it = get<I>(relations).set.begin();
					if (I == tuple_size<SetsOfRelationsType>::value - 1)
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
		bool next(const SetsOfRelationsType &relations, IteratorsArrayType &iterators,
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
			auto indexSequence = make_index_sequence<tuple_size<SetsOfRelationsType>::value>{};
			pick(relations, slice, indexSequence);
			iterationFinished = next(relations, iterators, indexSequence);
			#if 0
			{
				cout << "slice = ";
				print(cout, slice);
				cout << endl;
			}
			#endif
			return slice;
		}

	private:
		const SetsOfRelationsType &relations;
		IteratorsArrayType iterators;
		bool iterationFinished = false;

		// RELATION_TYPE is really the set
		template <typename RELATION_TYPE, typename ITERATOR_TYPE>
		static void initIterator(const RELATION_TYPE &relation, ITERATOR_TYPE &iterator)
		{
			iterator = relation.begin();
		}

		template <size_t... Is>
		static void initIterators(const SetsOfRelationsType &relations,
								  IteratorsArrayType &iterators, index_sequence<Is...>)
		{
			((initIterator(get<Is>(relations).set, get<Is>(iterators))), ...);
		}

		static IteratorsArrayType initIterators(const SetsOfRelationsType &relations)
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

template <size_t I, typename RULE_TYPE, typename... RELATIONs>
static bool bind(typename RULE_TYPE::BodyType &atoms, // typedef tuple<typename BODY_RELATIONs::Atom...> BodyType;
				 const tuple<RELATIONs const *...> &slice)
{
	// get the fact in the slice that can potentially bind
	typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type BodyRelationI;
	auto factPtr = get<BodyRelationI const *>(slice);
	bool success = false;
	if (factPtr)
	{
		const BodyRelationI &fact = *factPtr;
		// get the atom
		typename BodyRelationI::Atom &atom = get<I>(atoms);
		// try to bind the atom with the fact
		success = bind(atom, fact);
	}
	return success;
}

template <typename RULE_TYPE, typename... RELATIONs, size_t... Is>
static bool bind(typename RULE_TYPE::BodyType &atoms,
				 const tuple<RELATIONs const *...> &slice, index_sequence<Is...>)
{
	return ((bind<Is, RULE_TYPE, RELATIONs...>(atoms, slice)) and ...);
}

// bind n atoms (of multiple relation types) with 1 slice (of multiple relation types)
template <typename RULE_TYPE, typename... RELATIONs>
static bool bind(typename RULE_TYPE::BodyType &atoms,
				 const tuple<RELATIONs const *...> &slice)
{
	// unbind all the symbols
	unbind<RULE_TYPE>(atoms);
	// for each atom, bind with corresponding relation type in slice
	return bind<RULE_TYPE, RELATIONs...>(atoms, slice,
										 make_index_sequence<tuple_size<typename RULE_TYPE::BodyType>::value>{});
}

template <typename VALUE_TYPE>
void ground(const SymbolOrValue<VALUE_TYPE> &s, VALUE_TYPE &v)
{
	if (s.isSym())
	{
		Symbol<VALUE_TYPE> &symbol = *s.getSym();
		assert(symbol.isBound());
		v = symbol.value();
	}
	else
	{
		v = s.getVal();
	}
}

template <typename RELATION_TYPE, size_t... Is>
static void ground(const typename RELATION_TYPE::Atom &atom, RELATION_TYPE &groundAtom, index_sequence<Is...>)
{
	((ground(get<Is>(atom), get<Is>(groundAtom))), ...);
}

template <typename RELATION_TYPE>
static RELATION_TYPE ground(const typename RELATION_TYPE::Atom &atom)
{
	RELATION_TYPE groundAtom;
	ground<RELATION_TYPE>(atom, groundAtom, make_index_sequence<tuple_size<typename RELATION_TYPE::Atom>::value>{});
	return groundAtom;
}


template <typename RULE_TYPE, typename STATE_TYPE>
static Set<typename RULE_TYPE::HeadRelationType> applyRule(RULE_TYPE &rule, const STATE_TYPE &state)
{
	typedef typename RULE_TYPE::HeadRelationType HeadRelationType;
	Set<HeadRelationType> derivedFacts;
	auto it = state.iterator();
	while (it.hasNext())
	{
		auto slice = it.next();
		//if (bind<RULE_TYPE, RELATIONs...>(rule.body, slice))
		if (bind<RULE_TYPE>(rule.body, slice))
		{
			// successful bind, therefore add (grounded) head atom to new state
			//cout << "successful bind of body" << endl;
			auto derivedFact = ground<HeadRelationType>(rule.head);
			#if 1
			{
				cout << "adding: ";
				datalog::print<typename HeadRelationType::TupleType>(cout, derivedFact); // TODO: avoid need for type
				cout << endl;
			}
			#endif
			derivedFacts.set.insert(derivedFact);
		}
		else
		{
			//cout << "failed to bind body" << endl;
		}
	}
	return derivedFacts;
}

template <typename RELATION_TYPE>
static Set<RELATION_TYPE> merge(const Set<RELATION_TYPE>&s1, const Set<RELATION_TYPE>&s2)
{
	Set<RELATION_TYPE> s3{s1};
	s3.set.insert(s2.set.begin(), s2.set.end());
	return s3;
}

template <typename RELATION_TYPE, typename ... RELATIONs>
static State<RELATIONs...> add(const Set<RELATION_TYPE>& facts, const State<RELATIONs...> &state) {
	typedef State<RELATIONs...> StateType;
	StateType newState{state};
	typedef Set<RELATION_TYPE> SetType;
	auto& relation = get<SetType>(newState.relations);
	relation = merge(relation, facts);
	return newState;
}

template <typename RELATION_TYPE, typename ... RELATIONs>
static void add(const Set<RELATION_TYPE>& facts, const State<RELATIONs...> &inState, State<RELATIONs...> &outState) {
	outState = add(facts, inState);
}

template <typename ... RULE_TYPEs>
struct RuleSet {
	tuple<RULE_TYPEs...> rules;
};

template <typename ... RULE_TYPEs, typename... RELATIONs>
static State<RELATIONs...> applyRuleSet(RuleSet<RULE_TYPEs...> &ruleSet, const State<RELATIONs...> &state) {
	typedef State<RELATIONs...> StateType;
	StateType newState{state};
	apply([&state, &newState](auto &&... args) { ((add(applyRule(args, state), newState, newState)), ...); }, ruleSet.rules);
	return newState;
}

template <typename ... RULE_TYPEs, typename... RELATIONs>
static State<RELATIONs...> fixPoint(RuleSet<RULE_TYPEs...> &ruleSet, const State<RELATIONs...> &state) {
	typedef State<RELATIONs...> StateType;
	StateType newState{state};
	size_t inStateSize = 0;
	size_t outStateSize = 0;
	do {
		inStateSize = newState.size();
		newState = applyRuleSet(ruleSet, newState);
		outStateSize = newState.size();
	} while (inStateSize != outStateSize);
	return newState;
}

} // namespace datalog

#endif /* SRC_DATALOG_H_ */
