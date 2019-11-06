#include <Datalog.h>

using namespace datalog;

bool test1()
{
    enum Kind {
        person = 0,
        god
    };

    struct Thing : Relation<string, Kind> {};

    Thing::Set things{
        {"Socrates", person}, 
        {"Rhiannon", person}, 
        {"Albert", person},
        {"Anna", person},
        {"Henry", person}, 
        {"Ian", person}, 
        {"Zeus", god},
        {"Persephone", god},
        {"Thor", god}
    };

    struct Mortal : Relation<string> {};

    struct AllPeopleAreMortal : Rule<Mortal, Thing>
    {
        Variable<string> x;

        AllPeopleAreMortal() : Define{
            Mortal::Atom{var(x)},
            {
                Thing::Atom{var(x), {person}}
            }
        } {};
    };

    typedef State<Thing, Mortal> StateType;
    StateType state{{
        {things}, 
        {{}} 
    }};

    cout << "before = ";
    state.print(cout);
    cout << endl;

    RuleSet<AllPeopleAreMortal> rules{};
    state = fixPoint(rules, state);

    cout << "after = ";
    state.print(cout);
    cout << endl;

    return true;
}

bool test2()
{

    struct Int : Relation<int>
    {
    };
    struct Int_Int : Relation<int, int>
    {
    };
    struct String_Int : Relation<string, int>
    {
    };
    struct String_Int_String : Relation<string, int, string>
    {
    };

    {
        Int::Set r1{1, 2};
        Int_Int::Set r2{{1, 2}};
        Int_Int::Set r3{{1, 2}, {3, 4}};
        String_Int::Set r4{{"hello", 1}};
        String_Int::Set r5{{"hello", 1}, {"world", 2}};
        String_Int_String::Set r6{{"hello", 1, "world"}, {"world", 2, "hello"}, {"world", 3, "world"}};

#if 0
        String_Int r7 = merge(r4, r5);
#endif
    }
    {
        Variable<int> a;
        Variable<int> b;
        Int_Int::Atom r1{{1}, var(a)};
        String_Int::Atom r2{{"world"}, var(b)};
        Variable<string> c;
        String_Int_String::Atom r3{{"hello"}, {1}, var(c)};
    }

    {
        struct Adviser : Relation<string, string>
        {
        };
        Adviser::Set advisers{
            {"Andrew Rice", "Mistral Contrastin"}, 
            {"Dominic Orchard", "Mistral Contrastin"}, 
            {"Andy Hopper", "Andrew Rice"}, 
            {"Alan Mycroft", "Dominic Orchard"}, 
            {"David Wheeler", "Andy Hopper"}, 
            {"Rod Burstall", "Alan Mycroft"}, 
            {"Robin Milner", "Alan Mycroft"}
        };

        struct AcademicAncestor : Relation<string, string>
        {
        };

#if 0
        {
            Variable<string> x;
            Variable<string> y;
            Variable<string> z;

            // Bind 1 atom with 1 fact
            Adviser fact1{{"Andrew Rice", "Mistral Contrastin"}};
            Adviser::Atom clause1{var(x), var(y)};
            if (bind(clause1, fact1))
            {
                cout << "successful bind" << endl;
            }
            else
            {
                cout << "failed bind" << endl;
            }

            // Bind 1 atom with 1 fact
            Adviser::Atom dummyClause{var(x), var(x)};
            if (bind(dummyClause, fact1))
            {
                cout << "successful bind" << endl;
            }
            else
            {
                cout << "failed bind" << endl;
            }

            // Bind 1 atom with 1 fact
            Adviser fact2{{"Mistral Contrastin", "Mistral Contrastin"}};
            if (bind(dummyClause, fact2))
            {
                cout << "successful bind" << endl;
            }
            else
            {
                cout << "failed bind" << endl;
            }
        }
#endif

        // Rule1
        struct DirectAdviserIsAnAcademicAncestor : Rule<AcademicAncestor, Adviser>
        {
            Variable<string> x, y;

            DirectAdviserIsAnAcademicAncestor() : Define{
                                                      AcademicAncestor::Atom{var(x), var(y)},
                                                      {Adviser::Atom{var(x), var(y)}}} {};
        };

        // Rule2
        struct IndirectAdviserIsAnAcademicAncestor : Rule<AcademicAncestor, Adviser, AcademicAncestor>
        {
            Variable<string> x, y, z;

            IndirectAdviserIsAnAcademicAncestor() : Define{
                                                        AcademicAncestor::Atom{var(x), var(z)},
                                                        {Adviser::Atom{var(x), var(y)},
                                                         AcademicAncestor::Atom{var(y), var(z)}}} {};
        };

        // Query
        struct QueryResult : Relation<string>
        {
        };

        struct Query : Rule<QueryResult, AcademicAncestor, AcademicAncestor>
        {
            Variable<string> x, y;

            Query() : Define{
                          QueryResult::Atom{var(x)},
                          {AcademicAncestor::Atom{{"Robin Milner"}, var(x)},
                           AcademicAncestor::Atom{var(x), {"Mistral Contrastin"}}}} {};
        };

#if 0
        typedef State<Adviser, AcademicAncestor> StateType;
        //StateType state1{{advisers, {}}};
        // TODO: remove additional brackets
        StateType state1{{{advisers}, {{}}}};

        auto it1 = state1.iterator();
        while (it1.hasNext())
        {
            it1.next();
        }

        AcademicAncestor::Set dummyAncestors{{"Fred Bloggs", "Nobby Perkins"}, {"Charlie Chopper", "Jim Taylor"}, {"Cooper Simpson", "Lilly Fairweather"}};
        //StateType state2{{advisers, dummyAncestors}};
        StateType state2{{{advisers}, {dummyAncestors}}};
        auto it2 = state2.iterator();
        while (it2.hasNext())
        {
            it2.next();
        }
#endif

#if 0
        auto rule1 = IndirectAdviserIsAnAcademicAncestor{};
        auto derivedFacts1 = applyRule(rule1, state1);

        cout << "before = ";
        state1.print(cout);
        cout << endl;
        auto rule2 = DirectAdviserIsAnAcademicAncestor{};
        auto derivedFacts2 = applyRule(rule2, state1);
        //cout << "new derived facts = ";
        //derivedFacts2.print(cout);
        auto newState1 = add(derivedFacts2, state1);
        cout << "after = ";
        newState1.print(cout);
        cout << endl;

        // Apply multiple rules
        cout << "before = ";
        state1.print(cout);
        cout << endl;
        RuleSet<DirectAdviserIsAnAcademicAncestor, IndirectAdviserIsAnAcademicAncestor> rules{};
        auto newState2 = applyRuleSet(rules, state1);
        //newState2 = applyRuleSet(rules, newState2);
        cout << "after = ";
        newState2.print(cout);
        cout << endl;
#endif
        typedef State<Adviser, AcademicAncestor, QueryResult> StateType;
        // TODO: remove additional brackets
        StateType state{{
            {advisers}, 
            {{}}, 
            {{}} 
        }};

        // Apply multiple rules
        {
            RuleSet<DirectAdviserIsAnAcademicAncestor, IndirectAdviserIsAnAcademicAncestor> rules{};

            cout << "before = ";
            state.print(cout);
            cout << endl;
            state = fixPoint(rules, state);
            cout << "after = ";
            state.print(cout);
            cout << endl;
        }

        // Apply a query
#if 1
        {
            RuleSet<Query> rules{};
            state = fixPoint(rules, state);
            cout << "after = ";
            state.print(cout);
            cout << endl;
        }
#endif
    }

    return true;
}

int main() {
    test1();
    test2();
}
