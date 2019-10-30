#include <Datalog.h>

using namespace datalog;

int main() {

    struct Int: Relation<int> {
    };
    struct Int_Int: Relation<int, int> {
    };
    struct String_Int: Relation<string, int> {
    };
    struct String_Int_String: Relation<string, int, string> {
    };

    {
        Int r1{{{ 1, 2 }}};
        Int_Int r2 {{{ {1, 2} }}};
        Int_Int r3 {{{ {1, 2 }, { 3, 4 } }}};
        String_Int r4 {{{ { "hello", 1 } }}};
        String_Int r5 {{{ {"hello", 1 }, { "world", 2 } }}};
        String_Int_String r6 {{{ { "hello", 1, "world" }, { "world", 2, "hello" }, { "world", 3, "world" } }}};

#if 0
        String_Int r7 = merge(r4, r5);
#endif

    }

    {
        auto a = make_shared<Symbol<int>>();
        auto b = make_shared<Symbol<int>>();
        Int_Int::AtomicType r1 { { 1 }, { a } };
        String_Int::AtomicType r2 { { "world" }, { b } };
        auto c = make_shared<Symbol<string>>();
        String_Int_String::AtomicType r3 { { "hello" }, { 1 }, { c } };
    }

    {
        struct Adviser: Relation<string, string> {
        };
        Adviser advisers {{{ { { "Andrew Rice" }, { "Mistral Contrastin" } }, { { "Andy Hopper" }, { "Andrew Rice" } }, { { "Alan Mycroft" }, {
                "Dominic Orchard" } }, { { "David Wheeler" }, { "Andy Hopper" } }, { { "Rod Burstall" }, { "Alan Mycroft" } }, { { "Robin Milner" }, {
                "Alan Mycroft" } } }}};

        struct AcademicAncestor: Relation<string, string> {
        };

        auto x = make_shared<Symbol<string>>();
        auto y = make_shared<Symbol<string>>();
        auto z = make_shared<Symbol<string>>();
        // Rule1
        AcademicAncestor::AtomicType head1 { { x }, { y } };
        Adviser::AtomicType clause1 { { x }, { y } };
        Rule<AcademicAncestor, Adviser> rule1 { head1, { clause1 } };

        // Rule2
        AcademicAncestor::AtomicType head2 { { x }, { z } };
        Adviser::AtomicType clause2 { { x }, { y } };
        AcademicAncestor::AtomicType clause3 { { y }, { z } };
        typedef Rule<AcademicAncestor, Adviser, AcademicAncestor> Rule2Type;
        Rule2Type rule2 { head2, { clause2, clause3 } };

        // Query1
        struct Query1: Relation<string> {
        };

        Query1::AtomicType head3 { { x } };
        AcademicAncestor::AtomicType clause4 { { "Robin Milner" }, { x } };
        AcademicAncestor::AtomicType clause5 { { x }, { "Mistral Contrastin" } };
        Rule<Query1, AcademicAncestor, AcademicAncestor> query1 { head3, { clause4, clause5 } };

        // Bind 1 atom with 1 fact
        Adviser::GroundType fact1 { { { "Andrew Rice" } }, { { "Mistral Contrastin" } } };
        auto boundAtom1 = bind<Adviser>(clause1, fact1);
        if (boundAtom1.has_value()) {
            cout << "successful bind" << endl;
        } else {
            cout << "failed bind" << endl;
        }

        // Bind 1 atom with 1 fact
        Adviser::AtomicType dummyClause { { x }, { x } };
        auto boundAtom2 = bind<Adviser>(dummyClause, fact1);
        if (boundAtom2.has_value()) {
            cout << "successful bind" << endl;
        } else {
            cout << "failed bind" << endl;
        }

        // Bind 1 atom with 1 fact
        Adviser::GroundType fact2 { { { "Mistral Contrastin" } }, { { "Mistral Contrastin" } } };
        auto boundAtom3 = bind<Adviser>(dummyClause, fact2);
        if (boundAtom3.has_value()) {
            cout << "successful bind" << endl;
        } else {
            cout << "failed bind" << endl;
        }

        // Bind 1 atom with a relation (n facts)
        cout << "Should bind with all relations:" << endl;
        auto newRelation1 = bind(clause1, advisers);
        cout << "Should bind with 0 relations:" << endl;
        auto newRelation2 = bind(dummyClause, advisers);

        typedef State<Adviser, AcademicAncestor> StateType;
        StateType state1{{ advisers, {} }};
        auto it1 = state1.iterator();
#if 0
        while (it1.hasNext(state1)) {
           it1.next(state1);
        }
#endif

        AcademicAncestor dummyAncestors {{{ { { "Fred Bloggs" }, { "Nobby Perkins" } }, { { "Charlie Chopper" }, { "Jim Taylor" } },
            { { "Cooper Simpson" }, { "Lilly Fairweather" } }}}};
        StateType state2{{ advisers, dummyAncestors }};
        auto it2 = state2.iterator();
        while (it2.hasNext(state2)) {
           it2.next(state2);
        }


        apply<Rule2Type>(rule2, state1);
    }
}
