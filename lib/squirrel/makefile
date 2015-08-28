CC= gcc -std=c++11
VPATH=../unqlite/
squirrel: main.o unqlite.o general.o
	$(CC) -I/usr/include/mysql/ -lstdc++ *.o -o squirrel -pthread -lmysqlclient -lz 

clean:
	rm -f *.o
	
