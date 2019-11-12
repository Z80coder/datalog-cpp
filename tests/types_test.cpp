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
    auto x = var<Name>();
    auto rule1 = rule(atom<Mortal>(x), atom<Thing>(x, person));

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

    auto x = var<Name>();
    auto y = var<Name>();
    auto z = var<Name>();

    // TODO
    //auto inDirectAcademicAncestor = atom<AcademicAncestor>(x, z) <= atom<Adviser>(x, y) && atom<AcademicAncestor>(y, z);

    auto directAcademicAncestor = rule(atom<AcademicAncestor>(x, y), atom<Adviser>(x, y));
    auto indirectAcademicAncestor = rule(atom<AcademicAncestor>(x, z), atom<Adviser>(x, y), atom<AcademicAncestor>(y, z));
    auto query = rule(
        atom<QueryResult>(x),
        body( 
            atom<AcademicAncestor>(robin, x), 
            atom<AcademicAncestor>(x, mistral)
        )
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

    auto a = var<Number>();
    auto b = var<Number>();
    auto c = var<Number>();
    auto d = var<Number>();
    auto e = var<Number>();
    auto f = var<Number>();
    auto i = var<Number>();
    auto anon1 = var<Number>();
    auto anon2 = var<Number>();
    auto anon3 = var<Number>();
    auto anon4 = var<Number>();
    auto anon5 = var<Number>();
    auto anon6 = var<Number>();
    auto anon7 = var<Number>();
    auto anon8 = var<Number>();
    
    // A(1,i) :- Check(_, b, c, d, e, f), In(_, b, c, d, e, f, i).
    auto rule1 = rule(atom<A>(1u, i), atom<Check>(anon1, b, c, d, e, f), atom<In>(anon2, b, c, d, e, f, i));
    // A(2,i) :- Check(a, _, c, d, e, f), In(a, _, c, d, e, f, i).
    auto rule2 = rule(atom<A>(2u, i), atom<Check>(a, anon1, c, d, e, f), atom<In>(a, anon2, c, d, e, f, i));
    // A(3,i) :- Check(a, b, _, d, e, f), In(a, b, _, d, e, f, i).
    auto rule3 = rule(atom<A>(3u, i), atom<Check>(a, b, anon1, d, e, f), atom<In>(a, b, anon2, d, e, f, i));
    // A(4,i) :- Check(a, b, c, _, e, f), In(a, b, c, _, e, f, i).
    auto rule4 = rule(atom<A>(4u, i), atom<Check>(a, b, c, anon1, e, f), atom<In>(a, b, c, anon2, e, f, i));
    // A(5,i) :- Check(a, b, c, d, _, f), In(a, b, c, d, _, f, i).
    auto rule5 = rule(atom<A>(5u, i), atom<Check>(a, b, c, d, anon1, f), atom<In>(a, b, c, d, anon2, f, i));
    // A(6,i) :- Check(a, b, c, d, e, _), In(a, b, c, d, e, _, i).
    auto rule6 = rule(atom<A>(6u, i), atom<Check>(a, b, c, d, e, anon1), atom<In>(a, b, c, d, e, anon2, i));
    // A(7, i) :- Check(_, _, c, d, e, f), In(_, _, c, d, e, f, i).
    auto rule7 = rule(atom<A>(7u, i), atom<Check>(anon1, anon2, c, d, e, f), atom<In>(anon3, anon4, c, d, e, f, i));
    // A(8, i) :- Check(a, _, _, d, e, f), In(a, _, _, d, e, f, i).
    auto rule8 = rule(atom<A>(8u, i), atom<Check>(a, anon1, anon2, d, e, f), atom<In>(a, anon3, anon4, d, e, f, i));
    // A(9, i) :- Check(a, b, _, _, e, f), In(a, b, _, _, e, f, i).
    auto rule9 = rule(atom<A>(9u, i), atom<Check>(a, b, anon1, anon2, e, f), atom<In>(a, b, anon3, anon4, e, f, i));
    // A(10, i) :- Check(a, b, c, _, _, f), In(a, b, c, _, _, f, i).
    auto rule10 = rule(atom<A>(10u, i), atom<Check>(a, b, c, anon1, anon2, f), atom<In>(a, b, c, anon3, anon4, f, i));
    // A(11, i) :- Check(a, b, c, d, _, _), In(a, b, c, d, _, _, i).
    auto rule11 = rule(atom<A>(11u, i), atom<Check>(a, b, c, d, anon1, anon2), atom<In>(a, b, c, d, anon3, anon4, i));
    // A(12, i) :- Check(_, _, _, d, e, f), In(_, _, _, d, e, f, i).
    auto rule12 = rule(atom<A>(12u, i), atom<Check>(anon1, anon2, anon3, d, e, f), atom<In>(anon4, anon5, anon6, d, e, f, i));
    // A(13, i) :- Check(a, _, _, _, e, f), In(a, _, _, _, e, f, i).
    auto rule13 = rule(atom<A>(13u, i), atom<Check>(a, anon1, anon2, anon3, e, f), atom<In>(a, anon4, anon5, anon6, e, f, i));
    // A(14, i) :- Check(a, b, _, _, _, f), In(a, b, _, _, _, f, i).
    auto rule14 = rule(atom<A>(14u, i), atom<Check>(a, b, anon1, anon2, anon3, f), atom<In>(a, b, anon4, anon5, anon6, f, i));
    // A(15, i) :- Check(a, b, c, _, _, _), In(a, b, c, _, _, _, i).
    auto rule15 = rule(atom<A>(15u, i), atom<Check>(a, b, c, anon1, anon2, anon3), atom<In>(a, b, c, anon4, anon5, anon6, i));
    // A(16, i) :- Check(_, _, _, _, e, f), In(_, _, _, _, e, f, i).
    auto rule16 = rule(atom<A>(16u, i), atom<Check>(anon1, anon2, anon3, anon4, e, f), atom<In>(anon5, anon6, anon7, anon8, e, f, i));
    // A(17, i) :- Check(a, _, _, _, _, f), In(a, _, _, _, _, f, i).
    auto rule17 = rule(atom<A>(17u, i), atom<Check>(a, anon1, anon2, anon3, anon4, f), atom<In>(a, anon5, anon6, anon7, anon8, f, i));
    // A(18, i) :- Check(a, b, _, _, _, _), In(a, b, _, _, _, _, i).
    auto rule18 = rule(atom<A>(18u, i), atom<Check>(a, b, anon1, anon2, anon3, anon4), atom<In>(a, b, anon5, anon6, anon7, anon8, i));
    //  A(19, i) :- Check(a, b, c, d, e, f), In(a, b, c, d, e, f, i).
    auto rule19 = rule(atom<A>(19u, i), atom<Check>(a, b, c, d, e, f), atom<In>(a, b, c, d, e, f, i));

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

bool test4()
{
    // Relations
    typedef const char* Name;
    typedef unsigned int Age;
    enum Gender {male, female, NA};
    enum Country {england, scotland, wales, france, germany, netherlands, spain};
    struct Person : Relation<Name, Age, Gender, Country>{};

    // Extensional data
    Name sam{"Sam"};
    Name tim{"Tim"};
    Name rod{"Rod"};
    Name bob{"Bob"};
    Name jill{"Jill"};
    Name jane{"Jane"};
    Name sally{"Sally"};

    Person::Set people{
        {sam, 48u, male, scotland},
        {tim, 25u, male, england},
        {rod, 38u, male, germany},
        {bob, 18u, male, england},
        {jill, 56u, female, wales},
        {jane, 32u, female, france},
        {sally, 40u, female, netherlands}
    };

    auto name = var<Name>();
    auto age = var<Age>();
    auto gender = var<Gender>();
    auto country = var<Country>();

    struct Female : Relation<Name>{}; 
    auto females = rule(
        atom<Female>(name),
        atom<Person>(name, age, female, country)
    );

#if 1
    typedef float Metres;
    struct Height : Relation<Name, Metres>{}; 

    auto height = var<Metres>();

    auto heights = rule(
        atom<Height>(name, height),
        body(atom<Person>(name, age, female, country))
    );

    auto anyPerson = atom<Person>(name, age, gender, country);
    auto timPerson = atom<Person>(tim, age, gender, country);

    auto heightsExternal = rule(
        atom<Height>(name, height),
        body(
            anyPerson,
            timPerson//,
            // TODO: allow mixing
            //atom<Person>(name, 25u, gender, country)
        ),
        external(height, [&anyPerson]() { return 0.0f; })
    );

#endif

    // Apply rules
    State<Person, Female, Height> state{people, {}, {}};
    RuleSet<decltype(females), decltype(heights)> rules{
        {females, heights}
    };

    cout << "before = " << state << endl;
    state = fixPoint(rules, state);
    cout << "after = " << state << endl;

    return true;
}

int main()
{
    bool ok1 = test1();
    bool ok2 = test2();
    bool ok3 = po1();
    bool ok4 = test4();

    if (!(ok1 and ok2 and ok3 and ok4)) {
        cout << "FAIL" << endl;
        return 1;
    } else {
        cout << "PASSED" << endl;
        return 0;
    }
    return 1;
}
