#ifndef SRC_DATALOG_H_
#define SRC_DATALOG_H_

#include <tuple>
#include <set>
#include <unordered_set>
#include <numeric>
#include <optional>
#include <limits>
#include <functional>
#include <cassert>
#include <iostream>

#include "tuple_hash.h"

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

// TODO: use auto more for return type of functions

template<typename T>
Variable<T>* var() {
	return new Variable<T>();
}

template<typename T>
T val(Variable<T>* t) {
	return t->value();
}

template<typename T>
void deleteVar(Variable<T>* v) {
	delete v;
}

template <typename T>
void unbind(Variable<T>* t) {
    t->unbind();
}

template <typename T>
void unbind(const T& t) {}

template <typename... Ts>
void unbind(const tuple<Ts...> &tuple)
{
	apply([](auto &&... args) { ((unbind(args), ...)); }, tuple);
}

template <typename T>
bool bind(const T& a, const T& b) {
    return a == b;
}

template <typename T>
bool bind(const T& a, Variable<T>* const b) {
    if (b->isBound()) {
        return b->value() == a;
    }
    b->bind(a);
    return true;
}

template <typename GROUND_TYPE, typename ... Ts, size_t... Is>
bool bind(const GROUND_TYPE &fact, const tuple<Ts...> &atom, index_sequence<Is...>)
{
	return ((bind(get<Is>(fact), get<Is>(atom))) and ...);
}

template <typename GROUND_TYPE, typename ... Ts>
bool bind(const GROUND_TYPE &fact, const tuple<Ts...> &atom)
{
	return bind(fact, atom, make_index_sequence<tuple_size<GROUND_TYPE>::value>{});
}

template <typename T>
void ground(const Variable<T>* s, T &v)
{
	// N.B. bad optional access is thrown if th variable isn't bound
    v = s->value();
}

template <typename T>
void ground(const T &s, T &v)
{
    v = s;
}

template <typename RELATION_TYPE, typename ... Ts, size_t... Is>
void ground(const tuple<Ts...> &atom, typename RELATION_TYPE::Ground &groundAtom, index_sequence<Is...>)
{
	((ground(get<Is>(atom), get<Is>(groundAtom))), ...);
}

template <typename RELATION_TYPE, typename ... Ts>
typename RELATION_TYPE::Ground ground(const tuple<Ts...> &atom)
{
	typename RELATION_TYPE::Ground groundAtom;
	ground<RELATION_TYPE>(atom, groundAtom, make_index_sequence<tuple_size<typename RELATION_TYPE::Ground>::value>{});
	return groundAtom;
}

template<typename RELATION_TYPE, typename ... Ts>
struct AtomTypeSpecifier {
	typedef RELATION_TYPE RelationType;
	typedef tuple<Ts...> AtomType;
	AtomType atom;
};

template <typename ... Ts>
tuple<Ts...> atomImpl(Ts&&... args) {
    return tuple<Ts...>{args...};
}

template <typename RELATION_TYPE, typename ... Us>
AtomTypeSpecifier<RELATION_TYPE, Us...> atom(Us&&... args) {
	return AtomTypeSpecifier<RELATION_TYPE, Us...>{atomImpl(args...)};
}

template <typename... Ts>
struct Relation                                                                                                        
{
	typedef tuple<Ts...> Ground;
#if 1
	// set seems faster than unordered_set
	typedef set<Ground> Set;
#else
	typedef unordered_set<Ground> Set;
#endif

	typedef pair<size_t, Ground> TrackedGround;
#if 0
	struct compare {
		bool operator() (const TrackedGround& lhs, const TrackedGround& rhs) const {
			// ignore tracking number
			return lhs.second < rhs.second;
		}
	};

	typedef set<TrackedGround, compare> TrackedSet;
#else
	// unordered set seems faster than set
	struct key_hash : public std::unary_function<TrackedGround, std::size_t>
	{
		size_t operator()(const TrackedGround& k) const
		{
			hash<decltype(k.second)> h;
			return k.first ^ h.operator()(k.second);
		}
	};
	typedef unordered_set<TrackedGround, key_hash> TrackedSet;
#endif

};

template <typename HEAD_RELATION, typename... BODY_RELATIONs>
struct Rule
{
	typedef HEAD_RELATION HeadRelationType;
	typedef tuple<BODY_RELATIONs...> BodyRelations;
	typedef tuple<typename BODY_RELATIONs::TrackedSet::const_iterator...> BodyRelationsIteratorType;
	typedef tuple<const typename BODY_RELATIONs::TrackedGround *...> SliceType;
};

template<typename ... EXTERNAL_TYPEs>
struct Externals {
	typedef tuple<const EXTERNAL_TYPEs...> ExternalsTupleType;
	ExternalsTupleType externals;
};

template<typename ... BODY_ATOM_SPECIFIERs>
struct BodyAtoms {
	tuple<typename BODY_ATOM_SPECIFIERs::AtomType...> body;
};

template <typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
struct RuleInstance {
	typedef Rule<typename HEAD_ATOM_SPECIFIER::RelationType, typename BODY_ATOM_SPECIFIERs::RelationType...> RuleType;
	typedef typename HEAD_ATOM_SPECIFIER::AtomType HeadType;
	const HeadType head;
	typedef tuple<typename BODY_ATOM_SPECIFIERs::AtomType...> BodyType;
	BodyType body;
};

template <typename EXTERNALS_TYPE, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
struct ExternalRuleInstance {
	typedef Rule<typename HEAD_ATOM_SPECIFIER::RelationType, typename BODY_ATOM_SPECIFIERs::RelationType...> RuleType;
	typedef typename HEAD_ATOM_SPECIFIER::AtomType HeadType;
	const HeadType head;
	typedef tuple<typename BODY_ATOM_SPECIFIERs::AtomType...> BodyType;
	BodyType body;
	const EXTERNALS_TYPE externals;
};

template <typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
RuleInstance<HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...> rule(
	const HEAD_ATOM_SPECIFIER& h,
	const BODY_ATOM_SPECIFIERs&... b
) {
	typedef RuleInstance<HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...> RuleInstanceType;
	typename RuleInstanceType::HeadType head{h.atom};
	typename RuleInstanceType::BodyType body{b.atom...};
	return RuleInstanceType{head, body};
}

template <typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
RuleInstance<HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...> rule(
	const HEAD_ATOM_SPECIFIER& h,
	const BodyAtoms<BODY_ATOM_SPECIFIERs...>& b
) {
	typedef RuleInstance<HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...> RuleInstanceType;
	typename RuleInstanceType::HeadType head{h.atom};
	return RuleInstanceType{head, b.body};
}

// Rules with external functions

template<typename T>
struct ExternalFunction {
	Variable<T>* const& bindVariable;
	typedef function<T()> ExternalFunctionType;
	ExternalFunctionType externalFunction;
};

template<typename T>
ExternalFunction<T> lambda(
	Variable<T>* const& bindVariable,
	typename ExternalFunction<T>::ExternalFunctionType externalFunction) {
	return ExternalFunction<T> {bindVariable, externalFunction};
}

template<typename ... BODY_ATOM_SPECIFIERs>
BodyAtoms<BODY_ATOM_SPECIFIERs...> body(BODY_ATOM_SPECIFIERs&&... bodyAtoms) {
	return BodyAtoms<BODY_ATOM_SPECIFIERs...>{{bodyAtoms.atom...}};
}

template <typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs, typename... EXTERNAL_TYPEs>
ExternalRuleInstance<Externals<EXTERNAL_TYPEs...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...> rule(
	const HEAD_ATOM_SPECIFIER& h,
	const BodyAtoms<BODY_ATOM_SPECIFIERs...>& b,
	const EXTERNAL_TYPEs&... externals
) {
	typedef ExternalRuleInstance<Externals<EXTERNAL_TYPEs...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...> RuleInstanceType;
	typename RuleInstanceType::HeadType head{h.atom};
	return RuleInstanceType{head, b.body, Externals<EXTERNAL_TYPEs...>{externals...}};
}

template <typename RELATION_TYPE>
ostream& operator<<(ostream& out, const typename RELATION_TYPE::Ground& t) {
	out << "[";
	apply([&out](auto &&... args) { ((out << " " << args << " "), ...); }, t);
	out << "]";
	return out;
}

template<typename RELATION_TYPE>
ostream & operator<<(ostream &out, const typename RELATION_TYPE::Set& relationSet)
{
	out << "\"" << typeid(relationSet).name() << "\"" << endl;
	for (const auto& tuple : relationSet) {
		datalog::operator<< <RELATION_TYPE>(out, tuple);
		out << endl;
	}
	return out;
}

template<typename RELATION_TYPE>
struct RelationSet {
	typename RELATION_TYPE::TrackedSet set;
};

template<typename RELATION_TYPE>
ostream & operator<<(ostream &out, const RelationSet<RELATION_TYPE>& relationSet)
{
	out << "\"" << typeid(relationSet).name() << "\"" << endl;
	for (const auto& tuple : relationSet.set) {
		datalog::operator<< <RELATION_TYPE>(out, tuple.second);
		out << endl;
	}
	return out;
}

template<typename RELATION_TYPE>
struct RelationSize {
	size_t size = numeric_limits<size_t>::max();
};

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
		return convert<RELATION_TYPE>(getTrackedSet<RELATION_TYPE>());
	}

	template <typename RELATION_TYPE>
	const typename RELATION_TYPE::TrackedSet getTrackedSet() const {
		return get<RelationSet<RELATION_TYPE>>(stateRelations).set;
	}

	typedef tuple<RelationSize<RELATIONs>...> StateSizesType;

	template<size_t I>
	void sizes(StateSizesType& s) const {
		get<I>(s).size = get<I>(stateRelations).set.size();
	}

	template<size_t ... Is>
	void sizes(StateSizesType& s, index_sequence<Is...>) const {
		((sizes<Is>(s)), ...);
	}

	void sizes(StateSizesType& s) const {
		sizes(s, make_index_sequence<tuple_size<StateSizesType>::value>{});
	}

	static size_t size(const StateSizesType& s) {
		size_t sum = 0;
		auto add = [&sum](size_t size) { sum += size; };
		apply([&add](auto &&... args) { ((add(args.size)), ...); }, s);
		return sum;
	}

	template<size_t I>
	static void diff(StateSizesType& a, const StateSizesType& b) {
		get<I>(a).size = get<I>(a).size - get<I>(b).size;
	}

	template<size_t ... Is>
	static void diff(StateSizesType& a, const StateSizesType& b, index_sequence<Is...>) {
		((diff<Is>(a, b)), ...);
	}

	static void diff(StateSizesType& a, const StateSizesType& b) {
		diff(a, b, make_index_sequence<tuple_size<StateSizesType>::value>{});
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
			trackedSet.insert({0, relation});
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

template <typename RULE_INSTANCE_TYPE>
void unbind(const typename RULE_INSTANCE_TYPE::BodyType &atoms)
{
	apply([](auto &&... args) { ((unbind(args)), ...); }, atoms);
}

template <size_t I, typename RULE_INSTANCE_TYPE, typename RULE_TYPE>
bool bindBodyAtomsToSlice(const typename RULE_INSTANCE_TYPE::BodyType &atoms,
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
		success = bind(fact.second, atom);
	}
	return success;
}

template <typename RULE_INSTANCE_TYPE, typename RULE_TYPE, size_t... Is>
bool bindBodyAtomsToSlice(const typename RULE_INSTANCE_TYPE::BodyType &atoms,
				 const typename RULE_TYPE::SliceType &slice, index_sequence<Is...>)
{
	return ((bindBodyAtomsToSlice<Is, RULE_INSTANCE_TYPE, RULE_TYPE>(atoms, slice)) and ...);
}

template <typename RULE_INSTANCE_TYPE, typename RULE_TYPE>
bool bindBodyAtomsToSlice(const typename RULE_INSTANCE_TYPE::BodyType &atoms, const typename RULE_TYPE::SliceType &slice)
{
	// for each atom, bind with corresponding relation type in slice
	return bindBodyAtomsToSlice<RULE_INSTANCE_TYPE, RULE_TYPE>(atoms, slice, make_index_sequence<tuple_size<typename RULE_INSTANCE_TYPE::BodyType>::value>{});
}

template <typename RELATION_TYPE, size_t... Is>
void ground(const typename RELATION_TYPE::Atom &atom, typename RELATION_TYPE::Ground &groundAtom, index_sequence<Is...>)
{
	((ground(get<Is>(atom), get<Is>(groundAtom))), ...);
}

template <typename RELATION_TYPE>
typename RELATION_TYPE::Ground ground(const typename RELATION_TYPE::Atom &atom)
{
	typename RELATION_TYPE::Ground groundAtom;
	ground<RELATION_TYPE>(atom, groundAtom, make_index_sequence<tuple_size<typename RELATION_TYPE::Atom>::value>{});
	return groundAtom;
}

template <typename RELATION_TYPE, typename ... Ts>
typename RELATION_TYPE::Ground ground(const AtomTypeSpecifier<RELATION_TYPE, Ts...> &atomTypeSpecifier)
{
	return ground<RELATION_TYPE>(atomTypeSpecifier.atom);
}

template <size_t I, typename RULE_TYPE>
bool unseenSlice(size_t iteration, const typename RULE_TYPE::SliceType &slice)
{
	auto factPtr = get<I>(slice);
	if (factPtr) {
		const auto &fact = *factPtr;
		return fact.first == iteration;	
	}
	return false;
}

template <typename RULE_TYPE, size_t... Is>
bool unseenSlice(size_t iteration, const typename RULE_TYPE::SliceType &slice, index_sequence<Is...>)
{
	return ((unseenSlice<Is, RULE_TYPE>(iteration, slice)) or ...);
}

template <typename RULE_TYPE>
bool unseenSlice(size_t iteration, const typename RULE_TYPE::SliceType &slice) {
	return unseenSlice<RULE_TYPE>(iteration, slice, make_index_sequence<tuple_size<typename RULE_TYPE::BodyRelations>::value>{});
	return true;
}

template<size_t I, typename RULE_TYPE, typename STATE_TYPE>
bool unseenSlicePossible(const typename STATE_TYPE::StateSizesType& stateSizeDelta) {
	typedef typename tuple_element<I, typename RULE_TYPE::BodyRelations>::type RelationType;
	const auto& sizeDelta = get<RelationSize<RelationType>>(stateSizeDelta);
	return sizeDelta.size > 0;
}

template<typename RULE_TYPE, typename STATE_TYPE, size_t ... Is>
bool unseenSlicePossible(const typename STATE_TYPE::StateSizesType& stateSizeDelta, index_sequence<Is...>) {
	return ((unseenSlicePossible<Is, RULE_TYPE, STATE_TYPE>(stateSizeDelta)) or ...);
}

template<typename RULE_TYPE, typename STATE_TYPE>
bool unseenSlicePossible(const typename STATE_TYPE::StateSizesType& stateSizeDelta) {
	auto indexSequence = make_index_sequence<tuple_size<typename RULE_TYPE::BodyRelations>::value>{};
	return unseenSlicePossible<RULE_TYPE, STATE_TYPE>(stateSizeDelta, indexSequence);
}

template <typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
bool bindExternals(const RuleInstance<HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule) {
	return true;
}

template<size_t I, typename... Ts, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
bool bindExternal(const ExternalRuleInstance<Externals<Ts...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule) {
	auto& external = get<I>(rule.externals.externals);
	auto value = external.externalFunction();
	//cout << "external function returned " << value << endl;
	auto& bindVariable = external.bindVariable;
	return datalog::bind(value, bindVariable);
}

template <typename... Ts, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs, size_t ... Is>
bool bindExternals(const ExternalRuleInstance<Externals<Ts...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule, index_sequence<Is...>) {
	return ((bindExternal<Is>(rule)) and ...);
}

template <typename... Ts, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
bool bindExternals(const ExternalRuleInstance<Externals<Ts...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule) {
	return bindExternals(rule, make_index_sequence<tuple_size<typename Externals<Ts...>::ExternalsTupleType>::value>{});
}

template <typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
void unbindExternals(const RuleInstance<HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule) {}

template<size_t I, typename... Ts, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
void unbindExternal(const ExternalRuleInstance<Externals<Ts...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule) {
	auto& external = get<I>(rule.externals.externals);
	auto& bindVariable = external.bindVariable;
	bindVariable->unbind();
}

template <typename... Ts, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs, size_t ... Is>
void unbindExternals(const ExternalRuleInstance<Externals<Ts...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule, index_sequence<Is...>) {
	return ((unbindExternal<Is>(rule)), ...);
}

template <typename... Ts, typename HEAD_ATOM_SPECIFIER, typename... BODY_ATOM_SPECIFIERs>
void unbindExternals(const ExternalRuleInstance<Externals<Ts...>, HEAD_ATOM_SPECIFIER, BODY_ATOM_SPECIFIERs...>& rule) {
	unbindExternals(rule, make_index_sequence<tuple_size<typename Externals<Ts...>::ExternalsTupleType>::value>{});
}

template <typename RULE_TYPE, typename STATE_TYPE>
RelationSet<typename RULE_TYPE::RuleType::HeadRelationType> applyRule(
	size_t iteration, 
	const typename STATE_TYPE::StateSizesType& stateSizeDelta,
	RULE_TYPE &rule, 
	const STATE_TYPE &state
)
{
	typedef typename RULE_TYPE::RuleType::HeadRelationType HeadRelationType;
	RelationSet<HeadRelationType> derivedFacts;
	// does the body of this rule refer to relations with unseen data?
	if (unseenSlicePossible<typename RULE_TYPE::RuleType, STATE_TYPE>(stateSizeDelta)) {
		// OK, we now exhaustively check all relations for unseen combinations
		auto it = state.template it<typename RULE_TYPE::RuleType>();
		while (it.hasNext())
		{
			auto slice = it.next();
			// does this slice contain an unseen combination of ground atoms?
			if (unseenSlice<typename RULE_TYPE::RuleType>(iteration, slice)) {
				// unbind all the Variables
				unbind<RULE_TYPE>(rule.body);
				unbindExternals(rule);
				// try to bind rule body with slice
				if (bindBodyAtomsToSlice<RULE_TYPE, typename RULE_TYPE::RuleType>(rule.body, slice))
				{
					// run any externals
					if (bindExternals(rule)) {
						// successful bind, therefore add (grounded) head atom to new state
						derivedFacts.set.insert({iteration + 1, ground<HeadRelationType>(rule.head)});
					}
				}
			} 
		}
	} 
	return derivedFacts;
}

template <typename RELATION_TYPE>
void merge(RelationSet<RELATION_TYPE>& s1, RelationSet<RELATION_TYPE>&s2)
{
	s2.set.merge(s1.set);
}

template<size_t I, typename STATE_RELATIONS_TYPE>
void merge(STATE_RELATIONS_TYPE& newState, STATE_RELATIONS_TYPE& state) {
	auto& newSet = get<I>(newState.stateRelations);
	auto& set = get<I>(state.stateRelations);
	merge(newSet, set);
}

template <size_t ... Is, typename STATE_RELATIONS_TYPE>
void merge(STATE_RELATIONS_TYPE& newState, STATE_RELATIONS_TYPE& state, index_sequence<Is...>) {
	((merge<Is>(newState, state)), ...);
}

template<typename ... RELATIONs>
void merge(State<RELATIONs...> &newState, State<RELATIONs...> &state) {
	typedef typename State<RELATIONs...>::StateRelationsType StateRelationsType;
	return merge(newState, state, make_index_sequence<tuple_size<StateRelationsType>::value>{});
}

template <typename RELATION_TYPE, typename ... RELATIONs>
void assign(RelationSet<RELATION_TYPE>&& facts, State<RELATIONs...> &state) {
	typedef RelationSet<RELATION_TYPE> SetType;
	merge(facts, get<SetType>(state.stateRelations));
}

template <typename ... RULE_TYPEs>
struct RuleSet {
	tuple<RULE_TYPEs...> rules;
};

template <typename ... RULE_TYPEs>
RuleSet<RULE_TYPEs...> ruleset(RULE_TYPEs&&... r) {
	return RuleSet<RULE_TYPEs...>{{r...}};
}

template <typename ... RULE_TYPEs, typename... RELATIONs>
void applyRuleSet(
	size_t iteration, 
	typename State<RELATIONs...>::StateSizesType& stateSizeDelta,
	const RuleSet<RULE_TYPEs...> &ruleSet, 
	State<RELATIONs...> &state
) {
	// compute new state
	State<RELATIONs...> newState;
	apply([&iteration, &stateSizeDelta, &state, &newState](auto &&... args) { 
		((assign(applyRule(iteration, stateSizeDelta, args, state), newState)), ...); 
	}, ruleSet.rules);
	// merge new state
	typename State<RELATIONs...>::StateSizesType before;
	state.sizes(before);
	merge(newState, state);
	state.sizes(stateSizeDelta);
	state.diff(stateSizeDelta, before);
}

template <typename ... RULE_TYPEs, typename... RELATIONs>
State<RELATIONs...> fixPoint(const RuleSet<RULE_TYPEs...> &ruleSet, const State<RELATIONs...> &state) {
	typedef State<RELATIONs...> StateType;
	StateType newState{state};
	typename State<RELATIONs...>::StateSizesType stateSizeDelta;
	size_t iteration = 0; // TODO: make this the max iterator in state, to allow warm restart
	do {
		applyRuleSet(iteration, stateSizeDelta, ruleSet, newState);
		iteration++;
	} while (StateType::size(stateSizeDelta) > 0);
	cout << "fix point in " << iteration << " iterations" << endl;
	return newState;
}

} // namespace datalog

#endif /* SRC_DATALOG_H_ */
