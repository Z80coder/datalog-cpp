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

//#define TRACE_ON

namespace datalog
{

using namespace std;

template <typename T>
struct Variable : optional<T>
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
struct VariableOrValue : public variant<T, Variable<T> *>
{
	typedef Variable<T> *VariableType;

	bool isVar() const
	{
		return holds_alternative<VariableType>(*this);
	}

	const VariableType
	getVar() const
	{
		return get<VariableType>(*this);
	}

	const T &
	getVal() const
	{
		return get<T>(*this);
	}
};

template <typename T>
static VariableOrValue<T> var(Variable<T> &Variable)
{
	return VariableOrValue<T>{&Variable};
}

template <typename T>
static ostream &
operator<<(ostream &out, const VariableOrValue<T> &s)
{
	if (s.isVar())
	{
		out << s.getVar();
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
	typedef tuple<VariableOrValue<Ts>...> Atom;
	using tuple<Ts...>::tuple;
	typedef set<Relation> Set;
};

template <typename TUPLE_TYPE>
static ostream &
print(ostream &out, const TUPLE_TYPE &s)
{
	out << "[";
	apply([&out](auto &&... args) { ((out << "[" << args << "]"), ...); }, s);
	out << "] ";
	return out;
}

// TODO: can we avoid unbinding non-Variables by compile-time decisions?
template <typename T>
static void unbind(const VariableOrValue<T> &VariableOrValue)
{
	if (VariableOrValue.isVar()) {
 		VariableOrValue.getVar()->unbind();
	}
}

template <typename... Ts>
static void unbind(const tuple<VariableOrValue<Ts>...> &tuple)
{
	apply([](auto &&... args) { ((unbind(args), ...)); }, tuple);
}

// bind 1 VariableOrValue with 1 Value
template <typename VALUE_TYPE>
bool bind(VariableOrValue<VALUE_TYPE> &s, const VALUE_TYPE &v)
{
	bool success = false;
	if (s.isVar())
	{
		Variable<VALUE_TYPE> &Variable = *s.getVar();
		// has the Variable already been bound?
		if (Variable.isBound())
		{
			// is it a consistent binding?
			success = Variable.value() == v;
		} else {
			Variable.bind(v);
			success = true;
		}
	} else {
		success = s.getVal() == v;
	}
	return success;
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

	typedef tuple<typename BODY_RELATIONs::Atom...> BodyType;
	BodyType body;
	typedef tuple<BODY_RELATIONs...> BodyRelations;
	typedef tuple<typename BODY_RELATIONs::Set::const_iterator...> BodyRelationsIteratorType;
	typedef tuple<const BODY_RELATIONs *...> SliceType;
};

template<typename RELATION_TYPE>
struct RelationSet {
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
	explicit State(const typename RELATIONs::Set&... relations) : relations{{relations}...} {}

	typedef tuple<RelationSet<RELATIONs>...> SetsOfRelationsType;
	SetsOfRelationsType relations;

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

	template<typename RULE_TYPE>
	struct Iterator
	{
		typedef typename RULE_TYPE::SliceType SliceType;
		typedef typename RULE_TYPE::BodyRelationsIteratorType RelationsIteratorType;
		
		Iterator(const SetsOfRelationsType &relations) : relations(relations), iterators(initIterators(relations))
		{
		}

		static ostream &
		print(ostream &out, const SliceType &slice)
		{
			apply([&out](auto &&... args) { ((print(out, args), ...)); }, slice);
			return out;
		}

	private:
		template <size_t I>
		void pick(const SetsOfRelationsType &relations, SliceType &slice)
		{
			typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type RelationType;
			const auto &it = get<I>(iterators);
			auto& sliceElement = get<I>(slice);
			const auto& relation = get<RelationSet<RelationType>>(relations);
			if (it != relation.set.end())
			{
				// TODO: avoid cast if possible
				sliceElement = reinterpret_cast<const RelationType *>(&*it);
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
			((pick<Is>(relations, slice)), ...);
		}

		template <size_t I>
		bool next(const SetsOfRelationsType &relations, RelationsIteratorType &iterators,
				  bool &stop)
		{
			typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type RelationType;

			bool iterationFinished = false;
			if (not stop)
			{
				auto &it = get<I>(iterators);
				const auto &end = get<RelationSet<RelationType>>(relations).set.end();
				if (it != end)
					it++;
				if (it == end)
				{
					it = get<RelationSet<RelationType>>(relations).set.begin();
					if (I == tuple_size<RelationsIteratorType>::value - 1)
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
		bool next(const SetsOfRelationsType &relations, RelationsIteratorType &iterators,
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
			auto indexSequence = make_index_sequence<tuple_size<RelationsIteratorType>::value>{};
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
		RelationsIteratorType iterators;
		bool iterationFinished = false;

		template <size_t I>
		static void initIterator(const SetsOfRelationsType &relations, RelationsIteratorType &iterators)
		{
			typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type RelationType;
			auto& it = get<I>(iterators);
			const auto& relation = get<RelationSet<RelationType>>(relations);
			it = relation.set.begin();
		}

		template <size_t... Is>
		static void initIterators(const SetsOfRelationsType &relations,
								  RelationsIteratorType &iterators, index_sequence<Is...>)
		{
			((initIterator<Is>(relations, iterators)), ...);
		}

		static RelationsIteratorType initIterators(const SetsOfRelationsType &relations)
		{
			RelationsIteratorType iterators;
			initIterators(relations, iterators, make_index_sequence<tuple_size<RelationsIteratorType>::value>{});
			return iterators;
		}
	};

	template <typename RULE_TYPE>
	Iterator<RULE_TYPE> it() const
	{
		Iterator<RULE_TYPE> it{relations};
		return it;
	}
};

template <typename RULE_TYPE>
static void unbind(const typename RULE_TYPE::BodyType &atoms)
{
	apply([](auto &&... args) { ((unbind(args)), ...); }, atoms);
}

template <size_t I, typename RULE_TYPE>
static bool bindBodyAtomsToSlice(typename RULE_TYPE::BodyType &atoms,
				 const typename RULE_TYPE::SliceType &slice)
{
	auto factPtr = get<I>(slice);
	bool success = false;
	if (factPtr)
	{
		const auto &fact = *factPtr;
		// get the atom
		auto &atom = get<I>(atoms);
		// try to bind the atom with the fact
		success = bind(atom, fact);
	}
	return success;
}

template <typename RULE_TYPE, size_t... Is>
static bool bindBodyAtomsToSlice(typename RULE_TYPE::BodyType &atoms,
				 const typename RULE_TYPE::SliceType &slice, index_sequence<Is...>)
{
	return ((bindBodyAtomsToSlice<Is, RULE_TYPE>(atoms, slice)) and ...);
}

// bind n atoms (of multiple relation types) with 1 slice (of multiple relation types)
template <typename RULE_TYPE>
static bool bindBodyAtomsToSlice(typename RULE_TYPE::BodyType &atoms, const typename RULE_TYPE::SliceType &slice)
{
	// unbind all the Variables
	unbind<RULE_TYPE>(atoms);
	// for each atom, bind with corresponding relation type in slice
	return bindBodyAtomsToSlice<RULE_TYPE>(atoms, slice, make_index_sequence<tuple_size<typename RULE_TYPE::BodyType>::value>{});
}

template <typename VALUE_TYPE>
void ground(const VariableOrValue<VALUE_TYPE> &s, VALUE_TYPE &v)
{
	if (s.isVar())
	{
		Variable<VALUE_TYPE> &Variable = *s.getVar();
		assert(Variable.isBound());
		v = Variable.value();
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
static RelationSet<typename RULE_TYPE::HeadRelationType> applyRule(RULE_TYPE &rule, const STATE_TYPE &state)
{
	typedef typename RULE_TYPE::HeadRelationType HeadRelationType;
	RelationSet<HeadRelationType> derivedFacts;
	auto it = state.template it<RULE_TYPE>();
	while (it.hasNext())
	{
		auto slice = it.next();
		if (bindBodyAtomsToSlice<RULE_TYPE>(rule.body, slice))
		{
			// successful bind, therefore add (grounded) head atom to new state
			//cout << "successful bind of body" << endl;
			auto derivedFact = ground<HeadRelationType>(rule.head);
			#ifdef TRACE_ON
			{
				cout << "adding: " << typeid(HeadRelationType).name() << " ";
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
static RelationSet<RELATION_TYPE> merge(const RelationSet<RELATION_TYPE>&s1, const RelationSet<RELATION_TYPE>&s2)
{
	RelationSet<RELATION_TYPE> s3{s1};
	s3.set.insert(s2.set.begin(), s2.set.end());
	return s3;
}

template <typename RELATION_TYPE, typename ... RELATIONs>
static State<RELATIONs...> add(const RelationSet<RELATION_TYPE>& facts, const State<RELATIONs...> &state) {
	typedef State<RELATIONs...> StateType;
	StateType newState{state};
	typedef RelationSet<RELATION_TYPE> SetType;
	auto& relation = get<SetType>(newState.relations);
	relation = merge(relation, facts);
	return newState;
}

template <typename RELATION_TYPE, typename ... RELATIONs>
static void add(const RelationSet<RELATION_TYPE>& facts, const State<RELATIONs...> &inState, State<RELATIONs...> &outState) {
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
#ifdef TRACE_ON
		cout << "in size = " << inStateSize << endl;
#endif
		newState = applyRuleSet(ruleSet, newState);
		outStateSize = newState.size();
#ifdef TRACE_ON
		cout << "out size = " << outStateSize << endl;
#endif
	} while (inStateSize != outStateSize);
	return newState;
}

} // namespace datalog

#endif /* SRC_DATALOG_H_ */
