#add -lssl and -lcypto if you want to enable HTTPS
LIBS=-pthread
CXX11_FLAGS=-std=c++11 -lstdc++
WARN_FLAGS=-Wno-write-strings -Wno-literal-suffix

VERSION=`date +'%y%m%d%H%M%S'`

VPATH=./lib/unqlite/:./lib/mongoose/:./src/:./src/views/

cchan: main.o unqlite.o mongoose.o helper.o general.o single.o
	$(CXX) $(WARN_FLAGS) $(LIBS) $(CXX11_FLAGS) *.o -o cchan -O3

	@echo Finish Building $(VERSION)

main.o: main.cpp single.h config.h general.h helper.h unqlite.h mongoose.h
	$(CXX) $(WARN_FLAGS) $(CXX11_FLAGS) -c ./src/main.cpp -D__BUILD_DATE=$(VERSION)

general.o: 
	$(CXX) $(WARN_FLAGS) $(CXX11_FLAGS) -c ./src/general.cpp

helper.o:
	$(CXX) $(WARN_FLAGS) $(CXX11_FLAGS) -c ./src/helper.cpp

single.o:
	$(CXX) $(WARN_FLAGS) $(CXX11_FLAGS) -c ./src/views/single.cpp


.PHONY: clean
clean:
	rm -R *.o

test:
	./cchan --load ./cchan_test.conf

valgrind: main.o unqlite.o mongoose.o helper.o cookie.o general.o
	$(CXX) $(WARN_FLAGS) $(LIBS) $(CXX11_FLAGS) *.o -o cchan_v -O0 -g

	@echo Finish Building $(VERSION)


