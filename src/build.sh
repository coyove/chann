gcc -lstdc++ -std=c++11 mongoose.c unqlite.c general.cpp helper.cpp cookie.cpp main.cpp -pthread -lssl -lcrypt -o cchan -Wno-write-strings -Wno-literal-suffix
