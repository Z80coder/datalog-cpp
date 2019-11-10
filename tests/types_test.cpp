#include <Datalog.h>

using namespace datalog;

bool test1()
{
    // Relations
    typedef const char* Name;
    enum Kind {person, god};
    struct Thing : Relation<Name, Kind>{};
    struct Mortal : Relation<Name>{};

    // Extensional data
    Name socrates{"Socrates"};
    Name rhiannon{"Rhiannon"};
    Name albert{"Albert"};
    Name anna{"Anna"};
    Name henry{"Henry"};
    Name ian{"Ian"};
    Name zeus{"Zeus"};
    Name persephone{"Persephone"};
    Name thor{"Thor"};

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

    // Rule
    Variable<Name> x;
    auto rule1 = Rule<Mortal, Thing>::rule(
        Mortal::head(&x),
        Thing::clause(&x, person)
    );

    State<Thing, Mortal> state{things, {}};

    // Apply rule
    RuleSet<decltype(rule1)> rules{rule1};

    cout << "before = " << state << endl;
    state = fixPoint(rules, state);
    cout << "after = " << state << endl;
    return true;
}

bool test2()
{
    // Relations
    typedef const char* Name;
    struct Adviser : Relation<Name, Name>{};
    struct AcademicAncestor : Relation<Name, Name>{};
    struct QueryResult : Relation<Name>{};

    // Extensional data
    Name andrew{"Andrew Rice"};
    Name mistral{"Mistral Contrastin"};
    Name dominic{"Dominic Orchard"};
    Name andy{"Andy Hopper"};
    Name alan{"Alan Mycroft"};
    Name rod{"Rod Burstall"};
    Name robin{"Robin Milner"};
    Name david{"David Wheeler"};

    Adviser::Set advisers{
        {andrew, mistral},
        {dominic, mistral},
        {andy, andrew},
        {alan, dominic},
        {david, andy},
        {rod, alan},
        {robin, alan}};

    Variable<Name> x, y, z;
    auto directAcademicAncestor = Rule<AcademicAncestor, Adviser>::rule(
        AcademicAncestor::head(&x, &y),
        Adviser::clause(&x, &y)
    );
    auto indirectAcademicAncestor = Rule<AcademicAncestor, Adviser, AcademicAncestor>::rule(
        AcademicAncestor::head(&x, &z),
        Adviser::clause(&x, &y),
        AcademicAncestor::clause(&y, &z)
    );
    auto query = Rule<QueryResult, AcademicAncestor, AcademicAncestor>::rule(
        QueryResult::head(&x),
        AcademicAncestor::clause(robin, &x),
        AcademicAncestor::clause(&x, mistral)
    );

    // Apply rules
    State<Adviser, AcademicAncestor, QueryResult> state{advisers, {}, {}};
    RuleSet<decltype(directAcademicAncestor), decltype(indirectAcademicAncestor), decltype(query)> rules{
        {directAcademicAncestor, indirectAcademicAncestor, query}
    };

    cout << "before = " << state << endl;
    state = fixPoint(rules, state);
    cout << "after = " << state << endl;

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

    // TODO
    //auto var = []() { return make_unique<Variable<Number>>(); };
    auto var = []() { return new Variable<Number>; };
    auto a = var();
    auto b = var();
    auto c = var();
    auto d = var();
    auto e = var();
    auto f = var();
    auto i = var();
    auto anon1 = var();
    auto anon2 = var();
    auto anon3 = var();
    auto anon4 = var();
    auto anon5 = var();
    auto anon6 = var();
    auto anon7 = var();
    auto anon8 = var();
    
    typedef Rule<A, Check, In> P0RuleType;

    // A(1,i) :- Check(_, b, c, d, e, f), In(_, b, c, d, e, f, i).
    auto rule1 = P0RuleType::rule(
        A::head(1u, i), Check::clause(anon1, b, c, d, e, f), In::clause(anon2, b, c, d, e, f, i)
    );
    // A(2,i) :- Check(a, _, c, d, e, f), In(a, _, c, d, e, f, i).
    auto rule2 = P0RuleType::rule(
        A::head(2u, i), Check::clause(a, anon1, c, d, e, f), In::clause(a, anon2, c, d, e, f, i)
    );
    // A(3,i) :- Check(a, b, _, d, e, f), In(a, b, _, d, e, f, i).
    auto rule3 = P0RuleType::rule(
        A::head(3u, i), Check::clause(a, b, anon1, d, e, f), In::clause(a, b, anon2, d, e, f, i)
    );
    // A(4,i) :- Check(a, b, c, _, e, f), In(a, b, c, _, e, f, i).
    auto rule4 = P0RuleType::rule(
        A::head(4u, i), Check::clause(a, b, c, anon1, e, f), In::clause(a, b, c, anon2, e, f, i)
    );
    // A(5,i) :- Check(a, b, c, d, _, f), In(a, b, c, d, _, f, i).
    auto rule5 = P0RuleType::rule(
        A::head(5u, i), Check::clause(a, b, c, d, anon1, f), In::clause(a, b, c, d, anon2, f, i)
    );
    // A(6,i) :- Check(a, b, c, d, e, _), In(a, b, c, d, e, _, i).
    auto rule6 = P0RuleType::rule(
        A::head(6u, i), Check::clause(a, b, c, d, e, anon1), In::clause(a, b, c, d, e, anon2, i)
    );
    // A(7, i) :- Check(_, _, c, d, e, f), In(_, _, c, d, e, f, i).
    auto rule7 = P0RuleType::rule(
        A::head(7u, i), Check::clause(anon1, anon2, c, d, e, f), In::clause(anon3, anon4, c, d, e, f, i)
    );
    // A(8, i) :- Check(a, _, _, d, e, f), In(a, _, _, d, e, f, i).
    auto rule8 = P0RuleType::rule(
        A::head(8u, i), Check::clause(a, anon1, anon2, d, e, f), In::clause(a, anon3, anon4, d, e, f, i)
    );
    // A(9, i) :- Check(a, b, _, _, e, f), In(a, b, _, _, e, f, i).
    auto rule9 = P0RuleType::rule(
        A::head(9u, i), Check::clause(a, b, anon1, anon2, e, f), In::clause(a, b, anon3, anon4, e, f, i)
    );
    // A(10, i) :- Check(a, b, c, _, _, f), In(a, b, c, _, _, f, i).
    auto rule10 = P0RuleType::rule(
        A::head(10u, i), Check::clause(a, b, c, anon1, anon2, f), In::clause(a, b, c, anon3, anon4, f, i)
    );
    // A(11, i) :- Check(a, b, c, d, _, _), In(a, b, c, d, _, _, i).
    auto rule11 = P0RuleType::rule(
        A::head(11u, i), Check::clause(a, b, c, d, anon1, anon2), In::clause(a, b, c, d, anon3, anon4, i)
    );
    // A(12, i) :- Check(_, _, _, d, e, f), In(_, _, _, d, e, f, i).
    auto rule12 = P0RuleType::rule(
        A::head(12u, i), Check::clause(anon1, anon2, anon3, d, e, f), In::clause(anon4, anon5, anon6, d, e, f, i)
    );
    // A(13, i) :- Check(a, _, _, _, e, f), In(a, _, _, _, e, f, i).
    auto rule13 = P0RuleType::rule(
        A::head(13u, i), Check::clause(a, anon1, anon2, anon3, e, f), In::clause(a, anon4, anon5, anon6, e, f, i)
    );
    // A(14, i) :- Check(a, b, _, _, _, f), In(a, b, _, _, _, f, i).
    auto rule14 = P0RuleType::rule(
        A::head(14u, i), Check::clause(a, b, anon1, anon2, anon3, f), In::clause(a, b, anon4, anon5, anon6, f, i)
    );
    // A(15, i) :- Check(a, b, c, _, _, _), In(a, b, c, _, _, _, i).
    auto rule15 = P0RuleType::rule(
        A::head(15u, i), Check::clause(a, b, c, anon1, anon2, anon3), In::clause(a, b, c, anon4, anon5, anon6, i)
    );
    // A(16, i) :- Check(_, _, _, _, e, f), In(_, _, _, _, e, f, i).
    auto rule16 = P0RuleType::rule(
        A::head(16u, i), Check::clause(anon1, anon2, anon3, anon4, e, f), In::clause(anon5, anon6, anon7, anon8, e, f, i)
    );
    // A(17, i) :- Check(a, _, _, _, _, f), In(a, _, _, _, _, f, i).
    auto rule17 = P0RuleType::rule(
        A::head(17u, i), Check::clause(a, anon1, anon2, anon3, anon4, f), In::clause(a, anon5, anon6, anon7, anon8, f, i)
    );
    // A(18, i) :- Check(a, b, _, _, _, _), In(a, b, _, _, _, _, i).
    auto rule18 = P0RuleType::rule(
        A::head(18u, i), Check::clause(a, b, anon1, anon2, anon3, anon4), In::clause(a, b, anon5, anon6, anon7, anon8, i)
    );
    //  A(19, i) :- Check(a, b, c, d, e, f), In(a, b, c, d, e, f, i).
    auto rule19 = P0RuleType::rule(
        A::head(19u, i), Check::clause(a, b, c, d, e, f), In::clause(a, b, c, d, e, f, i)
    );

    RuleSet<decltype(rule1), decltype(rule2), decltype(rule3), decltype(rule4), decltype(rule5), decltype(rule6), decltype(rule7), decltype(rule8), decltype(rule9), decltype(rule10), decltype(rule11), decltype(rule12), decltype(rule13), decltype(rule14), decltype(rule15), decltype(rule16), decltype(rule17), decltype(rule18), decltype(rule19)>
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
