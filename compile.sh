gcc -shared -fPIC -o libdyn.so tests/test_debugger/test_dynamic_linking/libdyn.c
sudo mv libdyn.so /usr/lib/


g++ --std=c++11 -o prf  *.cpp

gcc -o test.out test.c -no-pie
gcc -o dyn_test.out dyn_test.c -no-pie -ldyn -Wl,-z,now
gcc -o lazy_dyn.out dyn_test.c -no-pie -ldyn -Wl,-zlazy
