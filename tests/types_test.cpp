#include <Datalog.h>

using namespace datalog;

int main() {

    {
        Relation<int> r1 { { { 1 } } };
        Relation<int, int> r2 { { { 1, 2 } } };
        Relation<string, int> r3 { { { "hello", 1 } } };
        Relation<string, int> r4 { { { "hello", 1 }, { "world", 2 } } };
        Relation<int, int> r5 { { { 1, 2 }, { 3, 4 } } };
        Relation<string, int, string> r6 { { { "hello", 1, "world" }, { "world", 2, "hello" }, { "world", 3, "world" } } };

        Relation<string, int> r7 = merge(r3, r4);
    }

    {
        Relation<Int> r2 { { { 1, Id { "2" } } } };
        Relation<String, Int> r4 { { { { "hello" }, 1 }, { { "world" }, Id { "2" } } } };
        Relation<String, Int, String> r6 { { { { "hello" }, 1, { Id { "3" } } }, { { "world" }, 2, { "hello" } }, { { Id { "4" } }, 3, { "world" } } } };
    }

    {
        struct Adviser: Relation<String, String> {
            using Relation<String, String>::Relation;
        };
        Adviser advisers { { { { "Andrew Rice" }, { "Mistral Contrastin" } }, { { "Andy Hopper" }, { "Andrew Rice" } }, { { "Alan Mycroft" }, {
                "Dominic Orchard" } }, { { "David Wheeler" }, { "Andy Hopper" } }, { { "Rod Burstall" }, { "Alan Mycroft" } }, { { "Robin Milner" }, {
                "Alan Mycroft" } } } };

        struct AcademicAncestor: Relation<String, String> {
            using Relation<String, String>::Relation;
        };

        // Rule1
        Atom<AcademicAncestor> head1 { { Id { "x" }, Id { "y" } } };
        Atom<Adviser> clause1 { { Id { "x" }, Id { "y" } } };
        Rule<Atom<AcademicAncestor>, Atom<Adviser>> rule1 { head1, { { clause1 } } };

        // Rule2
        Atom<AcademicAncestor> head2 { { Id { "x" }, Id { "z" } } };
        Atom<Adviser> clause21 { { Id { "x" }, Id { "y" } } };
        Atom<AcademicAncestor> clause22 { { Id { "y" }, Id { "z" } } };
        Rule<Atom<AcademicAncestor>, Atom<Adviser>, Atom<AcademicAncestor>> rule2 { head2, { { clause21, clause22 } } };

        // Query1
        struct Query1: Relation<String> {
            using Relation<String>::Relation;
        };
        Atom<Query1> head3 { { Id { "x" } } };
        Atom<AcademicAncestor> clause31 { { { "Robin Milner" }, Id { "x" } } };
        Atom<AcademicAncestor> clause32 { { Id { "x" }, { "Mistral Contrastin" } } };
        Rule<Atom<Query1>, Atom<AcademicAncestor>> query1 { head3, { { clause31, clause32 } } };
    }
}
