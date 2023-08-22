#create shared library
gcc -shared -fPIC -o libdyn.so tests/test_debugger/test_dynamic_linking/libdyn.c
sudo mv libdyn.so /usr/lib/

#create debugger
g++ --std=c++11 -o prf  *.cpp


#create minor tests
gcc -o ./minor_tests/test.out ./minor_tests/test.c -no-pie
gcc -o ./minor_tests/dyn_test.out ./minor_tests/dyn_test.c -no-pie -ldyn -Wl,-z,now
gcc -o ./minor_tests/lazy_dyn.out ./minor_tests/dyn_test.c -no-pie -ldyn -Wl,-zlazy

gcc -o ./minor_tests/pic_test.out ./minor_tests/test.c 
