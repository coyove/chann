#define _CRT_SECURE_NO_WARNINGS

extern "C"{
#include "unqlite.h"
}

#include <stdio.h>
#include <string>
#include <cstring>
#include <time.h>

#define MAIN_THREAD		4
#define THREAD_REPLY	2
#define SAGE_THREAD		16
#define LOCKED_THREAD	128
#define NORMAL_DISPLAY	1

struct Thread{
	char state;			// thread's state
	/*
		1		|1		|1		|1		|1		|1		|1		|1
		locked	|----	|----	|sage	|----	|thread	|reply	|normal

		old representation:
		01101101 = m
		01100011 = c
		01100100 = d
	*/
	char content[16];	// (pointer) thread's content 
	char author[64];	// thread's author
	char email[64];		// author's e-mail
	char ssid[10];		// author's ssid
	time_t date;		// submitted date and time
	char imgSrc[16];	// if thread has an image attached
	long threadID;		// thread's ID
	long nextThread;	// (pointer) next thread's address
	long prevThread;	// (pointer) prev thread's address
	long childThread;	// (pointer) child thread's address (replies), root thread has no child thread
	long parentThread;
	long childCount;	// the number of children
};

// when use readXXX, remember to destory them

void changeState(struct Thread* t, char statebit, bool op);
char * resolveState(char state);
char* readString(unqlite *pDb, char* key);
char* readString_(unqlite *pDb, long key);
int writeString(unqlite *pDb, char* key, const char* value, bool autoCommit);
long writeString_(unqlite *pDb, char* value, bool autoCommit);
long readLong(unqlite *pDb, char* key);
long readLong_(unqlite *pDb, long key);
int writeLong(unqlite *pDb, char* key, long value, bool autoCommit);
long writeLong_(unqlite *pDb, long value, bool autoCommit);
struct Thread* readThread_(unqlite *pDb, long key);
long writeNewThread(unqlite *pDb, struct Thread* t, bool autoCommit);
int writeThread(unqlite *pDb, long key, struct Thread* t, bool autoCommit);
long nextCounter(unqlite *pDb);
int resetDatabase(unqlite *pDb);
long findParent(unqlite *pDb, long startID);
int deleteThread(unqlite *pDb, long tid);
int newThread(unqlite *pDb, const char* content, char* author, char* email, char* ssid, char* imgSrc, bool sega);
int newReply(unqlite *pDb, long id, const char* content, char* author, char* email, char* ssid, char* imgSrc, bool sega);
int displayReply(unqlite *pDb, struct Thread *t);
int listThread(unqlite *pDb);