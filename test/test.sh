#!/bin/bash

# build
cd ../build
cmake ..
make

# run
cd ..

# # token -0
# ./bin/compiler test/testcase/basic/45_comment1.sy -s0 -o ./test/output/basic/45_comment1.tk


# ayntax -0
./bin/compiler test/testcase/basic/04_arr_defn3.sy -s1 -o ./test/output/basic/04_arr_defn3.json