#add -lssl and -lcypto if you want to enable HTTPS
LIBS=-pthread
CXX11_FLAGS=-std=c++11 -lstdc++
CXXFLAGS=-std=c++11 -lstdc++
WARN_FLAGS=-Wno-write-strings -Wno-literal-suffix

VERSION=`date +'%y%m%d%H%M%S'`

VPATH=./lib/unqlite/:./lib/mongoose/:./src/:./src/views/

BUILDDIR=./build

SOURCE:=$(wildcard ./src/*.cpp)
SOURCE+=$(wildcard ./src/views/*.cpp)
SOURCE+=$(wildcard ./src/views/list/*.cpp)

OBJECT:=$(patsubst ./src/%, ./build/%, $(patsubst %.cpp, %.o, $(SOURCE)))
OBJECT+=./build/libs/unqlite.o ./build/libs/mongoose.o

cchan: $(OBJECT)
	$(CXX) $(WARN_FLAGS) $(LIBS) $(CXX11_FLAGS) $(OBJECT) -o cchan -O3

	@echo Finish Building $(VERSION)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CXX11_FLAGS) -c $< -o $@ -D__BUILD_DATE=$(VERSION)

$(BUILDDIR)/views/%.o $(BUILDDIR)/views/list/%.o: %.cpp
	@mkdir -p ./build/views
	@mkdir -p ./build/views/list
	$(CXX) $(CXX11_FLAGS) -c $< -o $@

$(BUILDDIR)/libs/%.o: %.c
	@mkdir -p ./build/libs
	$(CC) -c $< -o $@


.PHONY: clean
clean:
	find ./build/ -name '*.o' -delete

test:
	./cchan --load ./cchan_test.conf

valgrind: main.o unqlite.o mongoose.o helper.o cookie.o general.o
	$(CXX) $(WARN_FLAGS) $(LIBS) $(CXX11_FLAGS) *.o -o cchan_v -O0 -g

	@echo Finish Building $(VERSION)


