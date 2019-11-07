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
    State<Adviser, AcademicAncestor, QueryResult> state{advisers, {}, {}};

    // Apply multiple rules
    {
        RuleSet<DirectAcademicAncestor, IndirectAcademicAncestor> rules{{rule1, rule2}};

        cout << "before = " << state << endl;
        state = fixPoint(rules, state);
        cout << "after = " << state << endl;
    }

    // Query
    struct Query : Rule<QueryResult, AcademicAncestor, AcademicAncestor> {} query{
        atom<QueryResult>(x),
        {
            atom<AcademicAncestor>(robin, x),
            atom<AcademicAncestor>(x, mistral)
        }
    };

    // Apply a query
    {
        RuleSet<Query> rules{query};
        state = fixPoint(rules, state);
        cout << "after = " << state << endl;
    }

    return true;
}

int main()
{
    test1();
    test2();
}
