#include <Datalog.h>

using namespace datalog;

bool test1()
{
    // Relations
    typedef const char* NameType;
    enum Kind {person, god};
    struct Thing : Relation<NameType, Kind>{};
    struct Mortal : Relation<NameType>{};

    // Rule
    Variable<NameType> x;
    struct PeopleAreMortal : Rule<Mortal, Thing> {} rule{
        atom<Mortal>(x),
        {
            atom<Thing>(x, person)
        }
    };

    // Extensional data
    NameType socrates{"Socrates"};
    NameType rhiannon{"Rhiannon"};
    NameType albert{"Albert"};
    NameType anna{"Anna"};
    NameType henry{"Henry"};
    NameType ian{"Ian"};
    NameType zeus{"Zeus"};
    NameType persephone{"Persephone"};
    NameType thor{"Thor"};

    Thing::Set things{
        {socrates, person},
        {rhiannon, person},
        {albert, person},
        {anna, person},
        {henry, person},
        {ian, person},
        {zeus, god},
        {persephone, god},
        {thor, god}};

    State<Thing, Mortal> state{things, {}};

    // Apply rule
    RuleSet<PeopleAreMortal> rules{rule};

    cout << "before = " << state << endl;
    state = fixPoint(rules, state);
    cout << "after = " << state << endl;

    return true;
}

bool test2()
{
    // Relations
    typedef const char* NameType;
    struct Adviser : Relation<NameType, NameType>{};
    struct AcademicAncestor : Relation<NameType, NameType>{};
    struct QueryResult : Relation<NameType>{};

    // Extensional data
    NameType andrew{"Andrew Rice"};
    NameType mistral{"Mistral Contrastin"};
    NameType dominic{"Dominic Orchard"};
    NameType andy{"Andy Hopper"};
    NameType alan{"Alan Mycroft"};
    NameType rod{"Rod Burstall"};
    NameType robin{"Robin Milner"};
    NameType david{"David Wheeler"};

    Adviser::Set advisers{
        {andrew, mistral},
        {dominic, mistral},
        {andy, andrew},
        {alan, dominic},
        {david, andy},
        {rod, alan},
        {robin, alan}};

    // Rule1
    Variable<NameType> x, y, z;

    struct DirectAcademicAncestor : Rule<AcademicAncestor, Adviser> {} rule1{
        atom<AcademicAncestor>(x, y),
        {
            atom<Adviser>(x, y)
        }
    };

    // Rule2
    struct IndirectAcademicAncestor : Rule<AcademicAncestor, Adviser, AcademicAncestor> {} rule2{
        atom<AcademicAncestor>(x, z),
        {
            atom<Adviser>(x, y),
            atom<Adviser>(y, z)
        }
    };

    // Query
    struct Query : Rule<QueryResult, AcademicAncestor, AcademicAncestor> {} query{
        atom<QueryResult>(x),
        {
            atom<AcademicAncestor>(robin, x),
            atom<AcademicAncestor>(x, mistral)
        }
    };

    // Apply multiple rules
    {
        State<Adviser, AcademicAncestor, QueryResult> state{advisers, {}, {}};
        RuleSet<DirectAcademicAncestor, IndirectAcademicAncestor, Query> rules{{rule1, rule2, query}};

        cout << "before = " << state << endl;
        state = fixPoint(rules, state);
        cout << "after = " << state << endl;
    }

    return true;
}

#if 1
bool po1()
{
    typedef unsigned int Number;
    struct Check : Relation<Number, Number, Number, Number, Number, Number>{};
    struct In : Relation<Number, Number, Number, Number, Number, Number, Number>{};
    struct A : Relation<Number, Number>{};

    #include "in.txt"
    #include "check.txt"

    State<Check, In, A> state{check, in, {}};

    Variable<Number> a, b, c, d, e, f, i;
    Variable<Number> anon1, anon2, anon3, anon4, anon5, anon6, anon7, anon8;

    typedef Rule<A, Check, In> P0RuleType;

    // A(1,i) :- Check(_, b, c, d, e, f), In(_, b, c, d, e, f, i).
    struct Rule1 : P0RuleType {} rule1{
        atom<A>(1u, i), {atom<Check>(anon1, b, c, d, e, f), atom<In>(anon2, b, c, d, e, f, i)}
    };
    // A(2,i) :- Check(a, _, c, d, e, f), In(a, _, c, d, e, f, i).
    struct Rule2 : P0RuleType {} rule2{
        atom<A>(2u, i), {atom<Check>(a, anon1, c, d, e, f), atom<In>(a, anon2, c, d, e, f, i)}
    };
    // A(3,i) :- Check(a, b, _, d, e, f), In(a, b, _, d, e, f, i).
    struct Rule3 : P0RuleType {} rule3{
        atom<A>(3u, i), {atom<Check>(a, b, anon1, d, e, f), atom<In>(a, b, anon2, d, e, f, i)}
    };
    // A(4,i) :- Check(a, b, c, _, e, f), In(a, b, c, _, e, f, i).
    struct Rule4 : P0RuleType {} rule4{
        atom<A>(4u, i), {atom<Check>(a, b, c, anon1, e, f), atom<In>(a, b, c, anon2, e, f, i)}
    };
    // A(5,i) :- Check(a, b, c, d, _, f), In(a, b, c, d, _, f, i).
    struct Rule5 : P0RuleType {} rule5{
        atom<A>(5u, i), {atom<Check>(a, b, c, d, anon1, f), atom<In>(a, b, c, d, anon2, f, i)}
    };
    // A(6,i) :- Check(a, b, c, d, e, _), In(a, b, c, d, e, _, i).
    struct Rule6 : P0RuleType {} rule6{
        atom<A>(6u, i), {atom<Check>(a, b, c, d, e, anon1), atom<In>(a, b, c, d, e, anon2, i)}
    };
    // A(7, i) :- Check(_, _, c, d, e, f), In(_, _, c, d, e, f, i).
    struct Rule7 : P0RuleType {} rule7{
        atom<A>(7u, i), {atom<Check>(anon1, anon2, c, d, e, f), atom<In>(anon3, anon4, c, d, e, f, i)}
    };
    // A(8, i) :- Check(a, _, _, d, e, f), In(a, _, _, d, e, f, i).
    struct Rule8 : P0RuleType {} rule8{
        atom<A>(8u, i), {atom<Check>(a, anon1, anon2, d, e, f), atom<In>(a, anon3, anon4, d, e, f, i)}
    };
    // A(9, i) :- Check(a, b, _, _, e, f), In(a, b, _, _, e, f, i).
    struct Rule9 : P0RuleType {} rule9{
        atom<A>(9u, i), {atom<Check>(a, b, anon1, anon2, e, f), atom<In>(a, b, anon3, anon4, e, f, i)}
    };
    // A(10, i) :- Check(a, b, c, _, _, f), In(a, b, c, _, _, f, i).
    struct Rule10 : P0RuleType {} rule10{
        atom<A>(10u, i), {atom<Check>(a, b, c, anon1, anon2, f), atom<In>(a, b, c, anon3, anon4, f, i)}
    };
    // A(11, i) :- Check(a, b, c, d, _, _), In(a, b, c, d, _, _, i).
    struct Rule11 : P0RuleType {} rule11{
        atom<A>(11u, i), {atom<Check>(a, b, c, d, anon1, anon2), atom<In>(a, b, c, d, anon3, anon4, i)}
    };
    // A(12, i) :- Check(_, _, _, d, e, f), In(_, _, _, d, e, f, i).
    struct Rule12 : P0RuleType {} rule12{
        atom<A>(12u, i), {atom<Check>(anon1, anon2, anon3, d, e, f), atom<In>(anon4, anon5, anon6, d, e, f, i)}
    };
    // A(13, i) :- Check(a, _, _, _, e, f), In(a, _, _, _, e, f, i).
    struct Rule13 : P0RuleType {} rule13{
        atom<A>(13u, i), {atom<Check>(a, anon1, anon2, anon3, e, f), atom<In>(a, anon4, anon5, anon6, e, f, i)}
    };
    // A(14, i) :- Check(a, b, _, _, _, f), In(a, b, _, _, _, f, i).
    struct Rule14 : P0RuleType {} rule14{
        atom<A>(14u, i), {atom<Check>(a, b, anon1, anon2, anon3, f), atom<In>(a, b, anon4, anon5, anon6, f, i)}
    };
    // A(15, i) :- Check(a, b, c, _, _, _), In(a, b, c, _, _, _, i).
    struct Rule15 : P0RuleType {} rule15{
        atom<A>(15u, i), {atom<Check>(a, b, c, anon1, anon2, anon3), atom<In>(a, b, c, anon4, anon5, anon6, i)}
    };
    // A(16, i) :- Check(_, _, _, _, e, f), In(_, _, _, _, e, f, i).
    struct Rule16 : P0RuleType {} rule16{
        atom<A>(16u, i), {atom<Check>(anon1, anon2, anon3, anon4, e, f), atom<In>(anon5, anon6, anon7, anon8, e, f, i)}
    };
    // A(17, i) :- Check(a, _, _, _, _, f), In(a, _, _, _, _, f, i).
    struct Rule17 : P0RuleType {} rule17{
        atom<A>(17u, i), {atom<Check>(a, anon1, anon2, anon3, anon4, f), atom<In>(a, anon5, anon6, anon7, anon8, f, i)}
    };
    // A(18, i) :- Check(a, b, _, _, _, _), In(a, b, _, _, _, _, i).
    struct Rule18 : P0RuleType {} rule18{
        atom<A>(18u, i), {atom<Check>(a, b, anon1, anon2, anon3, anon4), atom<In>(a, b, anon5, anon6, anon7, anon8, i)}
    };
    //  A(19, i) :- Check(a, b, c, d, e, f), In(a, b, c, d, e, f, i).
    struct Rule19 : P0RuleType {} rule19{
        atom<A>(19u, i), {atom<Check>(a, b, c, d, e, f), atom<In>(a, b, c, d, e, f, i)}
    };

    RuleSet<Rule1, Rule2, Rule3, Rule4, Rule5, Rule6, Rule7, Rule8, Rule9, Rule10, Rule11, Rule12, Rule13, Rule14, Rule15, Rule16, Rule17, Rule18, Rule19>
    rules{
        {rule1, rule2, rule3, rule4, rule5, rule6, rule7, rule8, rule9, rule10, rule11, rule12, rule13, rule14, rule15, rule16, rule17, rule18, rule19}
    };

    //cout << "before = " << state << endl;
    state = fixPoint(rules, state);

    #include "a.txt"

    const auto& computedA = state.getSet<A>();

    cout << "result = ";
    operator<< <A>(cout, computedA);
    cout << endl;

    return computedA == aOut;
}
#endif

int main()
{
    bool ok1 = test1();
    bool ok2= test2();
#if 1
    bool ok3 = po1();
    if (!(ok1 and ok2 and ok3)) {
        cout << "FAIL" << endl;
        return 1;
    } else {
        cout << "PASSED" << endl;
        return 0;
    }
#endif
    return 1;
}
