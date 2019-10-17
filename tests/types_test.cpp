#include <Datalog.h>

using namespace datalog;

int main() {

    struct Int: RelationType<int> {
    };
    struct Int_Int: RelationType<int, int> {
    };
    struct String_Int: RelationType<string, int> {
    };
    struct String_Int_String: RelationType<string, int, string> {
    };
    {
        Relation<Int> r1 { { { 1 }, { 2 } } };

        Relation<Int_Int> r2 { { { 1, 2 } } };
        Relation<Int_Int> r3 { { { 1, 2 }, { 3, 4 } } };

        Relation<String_Int> r4 { { { "hello", 1 } } };
        Relation<String_Int> r5 { { { "hello", 1 }, { "world", 2 } } };

        Relation<String_Int_String> r6 { { { "hello", 1, "world" }, { "world", 2, "hello" }, { "world", 3, "world" } } };

        Relation<String_Int> r7 = merge(r4, r5);
    }

    {
        Atom<Int_Int> r1 { { 1, Symbol { "2" } } };
        Atom<String_Int> r2 { { { "world" }, Symbol { "2" } } };
        Atom<String_Int_String> r3 { { { "hello" }, 1, { Symbol { "3" } } } };
    }

    {
        struct Adviser: RelationType<string, string> {
        };
        Relation<Adviser> advisers { { { { "Andrew Rice" }, { "Mistral Contrastin" } }, { { "Andy Hopper" }, { "Andrew Rice" } }, { { "Alan Mycroft" }, {
                "Dominic Orchard" } }, { { "David Wheeler" }, { "Andy Hopper" } }, { { "Rod Burstall" }, { "Alan Mycroft" } }, { { "Robin Milner" }, {
                "Alan Mycroft" } } } };

        struct AcademicAncestor: RelationType<string, string> {
        };

        // Rule1
        Atom<AcademicAncestor> head1 { { Symbol { "x" }, Symbol { "y" } } };
        Atom<Adviser> clause1 { { Symbol { "x" }, Symbol { "y" } } };
        Rule<AcademicAncestor, Adviser> rule1 { head1, { { clause1 } } };

        // Rule2
        Atom<AcademicAncestor> head2 { { Symbol { "x" }, Symbol { "z" } } };
        Atom<Adviser> clause2 { { Symbol { "x" }, Symbol { "y" } } };
        Atom<AcademicAncestor> clause3 { { Symbol { "y" }, Symbol { "z" } } };
        Rule<AcademicAncestor, Adviser, AcademicAncestor> rule2 { head2, { { clause2, clause3 } } };

        // Query1
        struct Query1: RelationType<string> {
        };
        Atom<Query1> head3 { { Symbol { "x" } } };
        Atom<AcademicAncestor> clause4 { { { "Robin Milner" }, Symbol { "x" } } };
        Atom<AcademicAncestor> clause5 { { Symbol { "x" }, { "Mistral Contrastin" } } };
        Rule<Query1, AcademicAncestor> query1 { head3, { { clause4, clause5 } } };

        Adviser::GroundType fact1{ { "Andrew Rice" }, { "Mistral Contrastin" } };
        auto indexSequence = index_sequence_for<typename Adviser::GroundType>{};
        //auto boundAtom = bind(clause1, fact1, indexSequence);

    }
}
