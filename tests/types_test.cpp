#include <Datalog.h>

using namespace datalog;

int main()
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
        Symbol<int> a;
        Symbol<int> b;
        Int_Int::Atom r1{{1}, sym(a)};
        String_Int::Atom r2{{"world"}, sym(b)};
        Symbol<string> c;
        String_Int_String::Atom r3{{"hello"}, {1}, sym(c)};
    }

    {
        struct Adviser : Relation<string, string>
        {
        };
        Adviser::Set advisers{{"Andrew Rice", "Mistral Contrastin"}, {"Andy Hopper", "Andrew Rice"}, {"Alan Mycroft", "Dominic Orchard"}, {"David Wheeler", "Andy Hopper"}, {"Rod Burstall", "Alan Mycroft"}, {"Robin Milner", "Alan Mycroft"}};

        struct AcademicAncestor : Relation<string, string>
        {
        };

        Symbol<string> x;
        Symbol<string> y;
        Symbol<string> z;

        // Rule1
        Adviser::Atom clause1{sym(x), sym(y)};
        auto rule1 = Rule<AcademicAncestor, Adviser>{
            {sym(x), sym(y)},
            {{sym(x), sym(y)}}};

        // Rule1 alternative
        struct AdviserIsAnAcademicAncestor : Rule<AcademicAncestor, Adviser>
        {
            Symbol<string> x, y, z;

            AdviserIsAnAcademicAncestor() : Define{
                                                AcademicAncestor::Atom{sym(x), sym(y)},
                                                {Adviser::Atom{sym(x), sym(y)}}} {};
        };

        // Rule2
        typedef Rule<AcademicAncestor, Adviser, AcademicAncestor> Rule2Type;
        auto rule2 = Rule2Type{
            AcademicAncestor::Atom{sym(x), sym(z)},
            {Adviser::Atom{sym(x), sym(y)},
             AcademicAncestor::Atom{sym(y), sym(z)}}};

        // Query1
        struct Query1 : Relation<string>
        {
        };

        auto query1 = Rule<Query1, AcademicAncestor, AcademicAncestor>{
            {sym(x)},
            {{{"Robin Milner"}, sym(x)},
             {sym(x), {"Mistral Contrastin"}}}};

        // Bind 1 atom with 1 fact
        Adviser fact1{{"Andrew Rice", "Mistral Contrastin"}};
        if (bind(clause1, fact1))
        {
            cout << "successful bind" << endl;
        }
        else
        {
            cout << "failed bind" << endl;
        }

        // Bind 1 atom with 1 fact
        Adviser::Atom dummyClause{sym(x), sym(x)};
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
#if 0
        // Bind 1 atom with a relation (n facts)
        cout << "Should bind with all relations:" << endl;
        auto newRelation1 = bind(clause1, advisers);
        cout << "Should bind with 0 relations:" << endl;
        auto newRelation2 = bind(dummyClause, advisers);
#endif

        typedef State<Adviser, AcademicAncestor> StateType;
        //StateType state1{{advisers, {}}};
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

        // TODO: try to remove need for specifying rule type
        auto derivedFacts1 = apply<Rule2Type>(rule2, state1);

        cout << "before = ";
        state1.print(cout);
        cout << endl;
        auto derivedFacts2 = apply<decltype(rule1)>(rule1, state1);
        //cout << "new derived facts = ";
        //derivedFacts2.print(cout);
        auto newState = add(state1, derivedFacts2);
        cout << "after = ";
        newState.print(cout);
        cout << endl;
    }
}