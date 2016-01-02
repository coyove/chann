LIBS=-pthread -lleveldb
CXX11_FLAGS=-std=c++11 -lstdc++ -O2
CXXFLAGS=-std=c++11 -lstdc++
WARN_FLAGS=-Wno-write-strings -Wno-literal-suffix

ifndef CAPTCHA
USE_CAPTCHA=
else
USE_CAPTCHA=-DGOOGLE_RECAPTCHA
endif

VERSION=`date +'%y%m%d%H%M%S'`

VPATH=./lib/mongoose/:./src/:./src/views/

BUILDDIR=./build

SOURCE:=$(wildcard ./src/*.cpp)
SOURCE+=$(wildcard ./src/views/*.cpp)
SOURCE+=$(wildcard ./src/views/list/*.cpp)
SOURCE+=$(wildcard ./src/actions/*.cpp)

OBJECT:=$(patsubst ./src/%, ./build/%, $(patsubst %.cpp, %.o, $(SOURCE)))
OBJECT+=./build/libs/mongoose.o

cchan: $(OBJECT)
	$(CXX) $(WARN_FLAGS) $(LIBS) $(CXX11_FLAGS) $(OBJECT) -o chann

	@echo Finish Building $(VERSION)

$(BUILDDIR)/%.o: %.cpp
	$(CXX) $(CXX11_FLAGS) -c $< -o $@ -D__BUILD_DATE=$(VERSION) $(USE_CAPTCHA)

$(BUILDDIR)/views/%.o $(BUILDDIR)/views/list/%.o $(BUILDDIR)/actions/%.o: %.cpp
	@mkdir -p ./build/views
	@mkdir -p ./build/views/list
	@mkdir -p ./build/actions
	$(CXX) $(CXX11_FLAGS) -c $< -o $@

$(BUILDDIR)/libs/%.o: %.c
	@mkdir -p ./build/libs
	$(CC) -c $< -o $@


.PHONY: clean
clean:
	find ./build/ -name '*.o' -delete

test:
	./chann --load ./chann_test.conf


