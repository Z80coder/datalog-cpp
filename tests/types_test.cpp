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
    auto rule1 = rule(atom<Mortal>(&x), atom<Thing>(&x, person));

    State<Thing, Mortal> state{things, {}};

    // Apply rule
    auto rules = ruleset(rule1);

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

    auto x = new Variable<Name>();
    auto y = new Variable<Name>();
    auto z = new Variable<Name>();

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
    auto rules = ruleset(directAcademicAncestor, indirectAcademicAncestor, query);
    State<Adviser, AcademicAncestor, QueryResult> state{advisers, {}, {}};

    cout << "before = " << state << endl;
    state = fixPoint(rules, state);
    cout << "after = " << state << endl;

    delete x;
    delete y;
    delete z;

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

    auto a = new Variable<Number>();
    auto b = new Variable<Number>();
    auto c = new Variable<Number>();
    auto d = new Variable<Number>();
    auto e = new Variable<Number>();
    auto f = new Variable<Number>();
    auto i = new Variable<Number>();
    auto anon1 = new Variable<Number>();
    auto anon2 = new Variable<Number>();
    auto anon3 = new Variable<Number>();
    auto anon4 = new Variable<Number>();
    auto anon5 = new Variable<Number>();
    auto anon6 = new Variable<Number>();
    auto anon7 = new Variable<Number>();
    auto anon8 = new Variable<Number>();
    
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

    auto rules = ruleset(rule1, rule2, rule3, rule4, rule5, rule6, rule7, rule8, rule9, rule10, rule11, rule12, rule13,
        rule14, rule15, rule16, rule17, rule18, rule19);

    //cout << "before = " << state << endl;
    state = fixPoint(rules, state);

    #include "a.txt"

    const auto& computedA = state.getSet<A>();

    cout << "result = ";
    operator<< <A>(cout, computedA);
    cout << endl;

    delete a;
    delete b;
    delete c;
    delete d;
    delete e;
    delete f;
    delete i;
    delete anon1;
    delete anon2;
    delete anon3;
    delete anon4;
    delete anon5;
    delete anon6;
    delete anon7;
    delete anon8;
    
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

    auto name = new Variable<Name>();
    auto age = new Variable<Age>();
    auto gender = new Variable<Gender>();
    auto country = new Variable<Country>();

    struct Female : Relation<Name>{}; 
    auto females = rule(
        atom<Female>(name),
        atom<Person>(name, age, female, country)
    );

    typedef float Metres;
    struct Height : Relation<Name, Metres>{}; 

    auto height = new Variable<Metres>();

    auto heights = rule(
        atom<Height>(name, 1.0f),
        body(
            atom<Person>(name, age, gender, country)
        )
    );


    // Use this pattern to get at values too
    auto anyPerson = atom<Person>(name, age, gender, country);

    auto externalHeights1 = rule(
        atom<Height>(name, height),
        body(
            atom<Person>(name, age, gender, country)
        ),
        lambda(
            height, 
            [&anyPerson]() { 
                cout << "hello world!" << endl;
                auto person = ground<Person>(anyPerson);
                auto age = get<Age>(person);
                return age * 3.0f; 
            }
        )
    );

    // Use this pattern to get at variables (values can't be got directly)
    auto externalHeights = rule(
        atom<Height>(name, height),
        body(atom<Person>(name, age, female, country)),
        lambda(height, [&age]() { return age->value() * 3.0f; } )
    );

    // Apply rules
    auto rules = ruleset(females, heights, externalHeights);
    State<Person, Female, Height> state{people, {}, {}};

    cout << "before = " << state << endl;
    state = fixPoint(rules, state);
    cout << "after = " << state << endl;

    delete name;
    delete age;
    delete gender;
    delete country;
    delete height;

    return true;
}

int main()
{
    bool ok1 = test1();
    bool ok2 = test2();
#if 1
    bool ok3 = po1();
    bool ok4 = test4();

    if (!(ok1 and ok2 and ok3 and ok4)) {
        cout << "FAIL" << endl;
        return 1;
    } else {
        cout << "PASSED" << endl;
        return 0;
    }
#endif
    return 1;
}
