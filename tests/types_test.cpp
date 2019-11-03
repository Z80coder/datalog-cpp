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
        auto a = make_shared<Symbol<int>>();
        auto b = make_shared<Symbol<int>>();
        Int_Int::Atom r1{{1}, {a}};
        String_Int::Atom r2{{"world"}, {b}};
        auto c = make_shared<Symbol<string>>();
        String_Int_String::Atom r3{{"hello"}, {1}, {c}};
    }

    {
        struct Adviser : Relation<string, string>
        {
        };
        Adviser::Set advisers{{"Andrew Rice", "Mistral Contrastin"}, {"Andy Hopper", "Andrew Rice"}, {"Alan Mycroft", "Dominic Orchard"}, {"David Wheeler", "Andy Hopper"}, {"Rod Burstall", "Alan Mycroft"}, {"Robin Milner", "Alan Mycroft"}};

        struct AcademicAncestor : Relation<string, string>
        {
        };

        auto x = make_shared<Symbol<string>>();
        auto y = make_shared<Symbol<string>>();
        auto z = make_shared<Symbol<string>>();
        // Rule1
        Adviser::Atom clause1{{x}, {y}};
        Rule<AcademicAncestor, Adviser> rule1{
            {{x}, {y}}, 
            {
                {{x}, {y}}
            }
        };

        // Rule2
        typedef Rule<AcademicAncestor, Adviser, AcademicAncestor> Rule2Type;
        Rule2Type rule2{
            {{x}, {z}}, 
            {
                {{x}, {y}}, 
                {{y}, {z}}
            }
        };

        // Query1
        struct Query1 : Relation<string>
        {
        };

        Rule<Query1, AcademicAncestor, AcademicAncestor> query1{
            {{x}}, 
            {
                {{"Robin Milner"}, {x}}, 
                {{x}, {"Mistral Contrastin"}}
            }
        };

        // Bind 1 atom with 1 fact
        Adviser fact1{{"Andrew Rice", "Mistral Contrastin"}};
        if (bind<Adviser>(clause1, fact1))
        {
            cout << "successful bind" << endl;
        }
        else
        {
            cout << "failed bind" << endl;
        }

        // Bind 1 atom with 1 fact
        Adviser::Atom dummyClause{{x}, {x}};
        if (bind<Adviser>(dummyClause, fact1))
        {
            cout << "successful bind" << endl;
        }
        else
        {
            cout << "failed bind" << endl;
        }

        // Bind 1 atom with 1 fact
        Adviser fact2{{"Mistral Contrastin", "Mistral Contrastin"}};
        if (bind<Adviser>(dummyClause, fact2))
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
        StateType state1{{advisers, {}}};
        auto it1 = state1.iterator();
        while (it1.hasNext())
        {
            it1.next();
        }

        AcademicAncestor::Set dummyAncestors{{"Fred Bloggs", "Nobby Perkins"}, {"Charlie Chopper", "Jim Taylor"}, {"Cooper Simpson", "Lilly Fairweather"}};
        StateType state2{{advisers, dummyAncestors}};
        auto it2 = state2.iterator();
        while (it2.hasNext())
        {
            it2.next();
        }

        apply<Rule2Type>(rule2, state1);
    }
}