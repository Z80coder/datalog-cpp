#ifndef SRC_DATALOG_H_
#define SRC_DATALOG_H_

#include <tuple>
#include <set>
#include <unordered_set>
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

template<typename T>
VariableOrValue<T> atomElement(Variable<T>& t) { 
    return VariableOrValue<T>{&t};
};

template<typename T>
VariableOrValue<T> atomElement(const T& t) { 
    return VariableOrValue<T>{t};
};

template<typename RELATION_TYPE, typename ... Ts>
typename RELATION_TYPE::Atom atom(Ts&& ... elements) {
	return typename RELATION_TYPE::Atom(forward_as_tuple(atomElement(elements)...));
}

template <typename... Ts>
struct Relation                                                                                                        
{
	typedef tuple<Ts...> Ground;
	typedef tuple<VariableOrValue<Ts>...> Atom;

	// TODO: this should be an unordered_set
	typedef set<Ground> Set;
	typedef pair<size_t, Ground> TrackedGround;
	// TODO: this should be an unordered_set
	typedef set<TrackedGround> TrackedSet;
};

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
	return ((bind(get<Is>(atom), get<Is>(fact))) and ...);
}

template <typename ATOM_TYPE, typename GROUND_TYPE>
static bool bind(ATOM_TYPE &atom, const GROUND_TYPE &fact)
{
	constexpr size_t tupleSize = tuple_size<GROUND_TYPE>::value;
	return bind<ATOM_TYPE, GROUND_TYPE>(atom, fact, make_index_sequence<tupleSize>{});
}

template <typename HEAD_RELATION, typename... BODY_RELATIONs>
struct Rule
{
	typedef HEAD_RELATION HeadRelationType;
	typedef typename HEAD_RELATION::Atom HeadType;
	HeadType head;

	typedef tuple<typename BODY_RELATIONs::Atom...> BodyType;
	BodyType body;
	typedef tuple<BODY_RELATIONs...> BodyRelations;
	typedef tuple<typename BODY_RELATIONs::TrackedSet::const_iterator...> BodyRelationsIteratorType;
	typedef tuple<const typename BODY_RELATIONs::TrackedGround *...> SliceType;
};

template <typename RELATION_TYPE>
static ostream& operator<<(ostream& out, const typename RELATION_TYPE::Ground& t) {
	out << "[";
	apply([&out](auto &&... args) { ((out << " " << args << " "), ...); }, t);
	out << "]";
	return out;
}

template<typename RELATION_TYPE>
struct RelationSet {
	typename RELATION_TYPE::TrackedSet set;
};

template<typename RELATION_TYPE>
static ostream & operator<<(ostream &out, const RelationSet<RELATION_TYPE>& relationSet)
{
	out << "\"" << typeid(relationSet).name() << "\"" << endl;
	for (const auto& tuple : relationSet.set) {
		operator<< <RELATION_TYPE>(out, tuple.second);
		out << endl;
	}
	return out;
}

template <typename... RELATIONs>
struct State
{
	typedef tuple<RelationSet<RELATIONs>...> StateRelationsType;
	StateRelationsType stateRelations;

	State() {}

	template <std::size_t N = sizeof...(RELATIONs), typename std::enable_if<(N>0), int>::type = 0>
	State(const typename RELATIONs::Set&... stateRelations) : stateRelations(convert(stateRelations...)) {
	}

	template <typename RELATION_TYPE>
	const typename RELATION_TYPE::Set getSet() const {
		return convert<RELATION_TYPE>(get<RelationSet<RELATION_TYPE>>(stateRelations).set);
	}

	size_t size() const {
		size_t totalSize = 0;
		auto inc = [](size_t& size, size_t inc) { size += inc; };
		apply([&totalSize, &inc](auto &&... args) { ((inc(totalSize, args.set.size())), ...); }, stateRelations);
		return totalSize;
	}

	template<typename RULE_TYPE>
	struct Iterator
	{
		typedef typename RULE_TYPE::SliceType SliceType;
		typedef typename RULE_TYPE::BodyRelationsIteratorType RelationsIteratorType;
		
		Iterator(const StateRelationsType &relations) : relations(relations), iterators(initIterators(relations))
		{
		}

	private:
		template <size_t I>
		void pick(const StateRelationsType &relations, SliceType &slice)
		{
			typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type RelationType;
			const auto &it = get<I>(iterators);
			auto& sliceElement = get<I>(slice);
			const auto& relation = get<RelationSet<RelationType>>(relations);
			if (it != relation.set.end())
			{
				// TODO: avoid cast if possible
				sliceElement = reinterpret_cast<const typename RelationType::TrackedGround *>(&*it);
			}
			else
			{
				sliceElement = nullptr;
			}
		}

		template <size_t... Is>
		void pick(const StateRelationsType &relations, SliceType &slice,
				  index_sequence<Is...>)
		{
			((pick<Is>(relations, slice)), ...);
		}

		template <size_t I>
		bool next(const StateRelationsType &relations, RelationsIteratorType &iterators,
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
		bool next(const StateRelationsType &relations, RelationsIteratorType &iterators,
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
			return slice;
		}

	private:
		const StateRelationsType &relations;
		RelationsIteratorType iterators;
		bool iterationFinished = false;

		template <size_t I>
		static void initIterator(const StateRelationsType &relations, RelationsIteratorType &iterators)
		{
			typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type RelationType;
			auto& it = get<I>(iterators);
			const auto& relation = get<RelationSet<RelationType>>(relations);
			it = relation.set.begin();
		}

		template <size_t... Is>
		static void initIterators(const StateRelationsType &relations,
								  RelationsIteratorType &iterators, index_sequence<Is...>)
		{
			((initIterator<Is>(relations, iterators)), ...);
		}

		static RelationsIteratorType initIterators(const StateRelationsType &relations)
		{
			RelationsIteratorType iterators;
			initIterators(relations, iterators, make_index_sequence<tuple_size<RelationsIteratorType>::value>{});
			return iterators;
		}
	};

	template <typename RULE_TYPE>
	Iterator<RULE_TYPE> it() const
	{
		Iterator<RULE_TYPE> it{stateRelations};
		return it;
	}

private:
	typedef tuple<RELATIONs...> RelationsType;
	typedef tuple<typename RELATIONs::Set...> TupleType;

	template <typename RELATION_TYPE>
	static typename RELATION_TYPE::Set convert(const typename RELATION_TYPE::TrackedSet& trackedSet) {
		typename RELATION_TYPE::Set set;
		for (const auto& relation : trackedSet) {
			set.insert(relation.second);
		}
		return set;
	}

	template <typename RELATION_TYPE>
	static typename RELATION_TYPE::TrackedSet convert(const typename RELATION_TYPE::Set& set) {
		typename RELATION_TYPE::TrackedSet trackedSet;
		for (const auto& relation : set) {
			trackedSet.insert({1, relation});
		}
		return trackedSet;
	}

	template<size_t I>
	static void convert(const TupleType& tuple, StateRelationsType& stateRelations) {
		typedef typename tuple_element<I, RelationsType>::type RelationType;
		const auto& relationSet = get<I>(tuple);
		get<I>(stateRelations) = RelationSet<RelationType>{convert<RelationType>(relationSet)};
	}

	template <size_t ... Is>
	static StateRelationsType convert(const TupleType& tuple, index_sequence<Is...>) {
		StateRelationsType stateRelations;
		((convert<Is>(tuple, stateRelations)), ...);
		return stateRelations;
	}

	static StateRelationsType convert(const typename RELATIONs::Set&... stateRelations) {
		const TupleType& tuple = make_tuple(stateRelations...);
		return convert(tuple, make_index_sequence<tuple_size<TupleType>::value>{});
	}


};

template <typename... RELATIONs>
ostream & operator<<(ostream &out, const State<RELATIONs...>& state) {
	out << "[";
	apply([&out](auto &&... args) { ((operator<<(out, args)), ...); }, state.stateRelations);
	out << "] ";
	return out;
}

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
		success = bind(atom, fact.second);
	}
	return success;
}

template <typename RULE_TYPE, size_t... Is>
static bool bindBodyAtomsToSlice(typename RULE_TYPE::BodyType &atoms,
				 const typename RULE_TYPE::SliceType &slice, index_sequence<Is...>)
{
	return ((bindBodyAtomsToSlice<Is, RULE_TYPE>(atoms, slice)) and ...);
}

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
static void ground(const typename RELATION_TYPE::Atom &atom, typename RELATION_TYPE::Ground &groundAtom, index_sequence<Is...>)
{
	((ground(get<Is>(atom), get<Is>(groundAtom))), ...);
}

template <typename RELATION_TYPE>
static typename RELATION_TYPE::Ground ground(const typename RELATION_TYPE::Atom &atom)
{
	typename RELATION_TYPE::Ground groundAtom;
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
			//derivedFacts.set.insert(derivedFact);
			derivedFacts.set.insert({1, derivedFact});
		}
		else
		{
			//cout << "failed to bind body" << endl;
		}
	}
	return derivedFacts;
}

template <typename RELATION_TYPE>
static void merge(RelationSet<RELATION_TYPE>& s1, RelationSet<RELATION_TYPE>&s2)
{
	s2.set.merge(s1.set);
}

template<size_t I, typename STATE_RELATIONS_TYPE>
static void merge(STATE_RELATIONS_TYPE& newState, STATE_RELATIONS_TYPE& state) {
	auto& newSet = get<I>(newState.stateRelations);
	auto& set = get<I>(state.stateRelations);
	merge(newSet, set);
}

template <size_t ... Is, typename STATE_RELATIONS_TYPE>
static void merge(STATE_RELATIONS_TYPE& newState, STATE_RELATIONS_TYPE& state, index_sequence<Is...>) {
	((merge<Is>(newState, state)), ...);
}

template<typename ... RELATIONs>
static void merge(State<RELATIONs...> &newState, State<RELATIONs...> &state) {
	typedef typename State<RELATIONs...>::StateRelationsType StateRelationsType;
	return merge(newState, state, make_index_sequence<tuple_size<StateRelationsType>::value>{});
}

template <typename RELATION_TYPE, typename ... RELATIONs>
static void assign(RelationSet<RELATION_TYPE>&& facts, State<RELATIONs...> &state) {
	typedef RelationSet<RELATION_TYPE> SetType;
	merge(facts, get<SetType>(state.stateRelations));
}

template <typename ... RULE_TYPEs>
struct RuleSet {
	tuple<RULE_TYPEs...> rules;
};

template <typename ... RULE_TYPEs, typename... RELATIONs>
static void applyRuleSet(RuleSet<RULE_TYPEs...> &ruleSet, State<RELATIONs...> &state) {
	// compute new state
	State<RELATIONs...> newState;
	apply([&state, &newState](auto &&... args) { ((assign(applyRule(args, state), newState)), ...); }, ruleSet.rules);
	// merge new state
	merge(newState, state);
}

template <typename ... RULE_TYPEs, typename... RELATIONs>
static State<RELATIONs...> fixPoint(RuleSet<RULE_TYPEs...> &ruleSet, const State<RELATIONs...> &state) {
	typedef State<RELATIONs...> StateType;
	StateType newState{state};
	size_t inSize = 0;
	size_t outSize = 0;
	do {
		inSize = newState.size();
		applyRuleSet(ruleSet, newState);
		outSize = newState.size();
	} while (inSize != outSize);
	return newState;
}

} // namespace datalog

#endif /* SRC_DATALOG_H_ */
