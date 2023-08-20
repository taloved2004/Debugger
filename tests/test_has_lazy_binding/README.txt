how to run:

to compile:
g++ --std=c++11 -o testLazy  *.cpp 

to run: 
./testLazy test{i}.out

// results
test1 and test2 are without lazy binding
test3 is with lazy binding

to check yourself do:
readelf -d test{i}.out

and check if there is a BIND_NOW tag

to create a test:
to make him with lazy binding, compile it with the '-wl,zlazy' flag
to make him with OUT lazy binding, compile it with the '-wl,z, now' flag