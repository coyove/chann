#add -lssl and -lcypto if you want to enable HTTPS
LIBS=-pthread
CXX11_FLAGS=-std=c++11 -lstdc++
WARN_FLAGS=-Wno-write-strings -Wno-literal-suffix

VERSION=`date +'%Y%m%d%H%M%S'`

VPATH=./lib/unqlite/:./lib/mongoose/:./src/

cchan: main.o unqlite.o mongoose.o helper.o cookie.o general.o
	$(CXX) $(WARN_FLAGS) $(LIBS) $(CXX11_FLAGS) *.o -o cchan -O2

	@echo Finish Building $(VERSION)

main.o: main.cpp general.h cookie.h helper.h unqlite.h mongoose.h
	$(CXX) $(WARN_FLAGS) $(CXX11_FLAGS) -c ./src/main.cpp -D__BUILD_DATE=$(VERSION)

general.o: 
	$(CXX) $(WARN_FLAGS) $(CXX11_FLAGS) -c ./src/general.cpp


.PHONY: clean
clean:
	rm -R *.o

stop:
	ps aux | grep "deamon" | awk '{system("kill -9 " $$2)}'
	ps aux | grep "cchan" | awk '{system("kill -9 " $$2)}'

test:
	./cchan --title "TEST SITE" --admin-spell 111 --tpp 10 --database db/test2.db --port 13739 --salt zhang --stop-ipcheck


