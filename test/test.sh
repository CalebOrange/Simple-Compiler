#!/bin/bash

# build
cd ../build
cmake ..
make

# run
cd ..

# token -0
# ./bin/compiler test/testcase/function/95_float.sy -s0 -o ./test/output/function/95_float.tk


# syntax -0
# ./bin/compiler test/testcase/basic/00_main.sy -s1 -o ./test/output/basic/00_main.json

# semantic 
./bin/compiler test/testcase/basic/00_main.sy -e -o ./test/output/basic/00_main.out