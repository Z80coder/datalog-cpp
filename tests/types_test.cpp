#include <Datalog.h>

using namespace datalog;

bool test1()
{
    enum Kind
    {
        person = 0,
        god
    };

    struct Thing : Relation<string, Kind>
    {
    };

    Thing::Set things{
        {"Socrates", person},
        {"Rhiannon", person},
        {"Albert", person},
        {"Anna", person},
        {"Henry", person},
        {"Ian", person},
        {"Zeus", god},
        {"Persephone", god},
        {"Thor", god}};

    struct Mortal : Relation<string>
    {
    };

    Mortal::Set mortals{};
    
    struct AllPeopleAreMortal : Rule<Mortal, Thing>
    {
        Variable<string> x;

        AllPeopleAreMortal() : Define{
                                   Mortal::Atom{var(x)},
                                   {Thing::Atom{var(x), {person}}}} {};
    };

    typedef State<Thing, Mortal> StateType;
    StateType state{{{things},
                     {{}}}};

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
        {"Robin Milner", "Alan Mycroft"}};

    struct AcademicAncestor : Relation<string, string>
    {
    };

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

    typedef State<Adviser, AcademicAncestor, QueryResult> StateType;
    // TODO: remove additional brackets
    StateType state{{{advisers},
                     {{}},
                     {{}}}};

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
    {
        RuleSet<Query> rules{};
        state = fixPoint(rules, state);
        cout << "after = ";
        state.print(cout);
        cout << endl;
    }

    return true;
}

int main()
{
    test1();
    test2();
}
