#!/bin/bash
set -e
echo "Running tests"
../build/types_test
../build/variable_test
../build/tuple_binding_test
echo "Checking for memory leaks"
cd ../build; ctest --overwrite MemoryCheckCommandOptions="--leak-check=full --error-exitcode=1" -T memcheck