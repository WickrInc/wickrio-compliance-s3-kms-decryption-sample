#export LIB=/usr/local/lib
#export INCLUDE=/usr/local/include
cmake -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS -I/usr/local/include -L/usr/local/lib" ..
