#define _CRT_SECURE_NO_WARNINGS

#ifndef CCHAN_GENERAL_HEADER_INCLUDED
#define CCHAN_GENERAL_HEADER_INCLUDED

extern "C"{
#include "../unqlite/unqlite.h"
}

#include <stdio.h>
#include <string>
#include <cstring>
#include <time.h>

#define MAIN_THREAD		4
#define THREAD_REPLY	2
#define SAGE_THREAD		16
#define LOCKED_THREAD	128
#define TOOMANY_REPLIES 8
#define NORMAL_DISPLAY	1

#if defined(_WIN32)
#define cclong long
#else
#define cclong int
#endif

struct Thread{
	char state;			// thread's state
	/*
		1		|1		|1		|1		|1		|1		|1		|1
		locked	|----	|----	|sage	|toomany|thread	|reply	|normal

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
	cclong threadID;		// thread's ID
	cclong nextThread;	// (pointer) next thread's address
	cclong prevThread;	// (pointer) prev thread's address
	cclong childThread;	// (pointer) child thread's address (replies), root thread has no child thread
	cclong parentThread;
	cclong childCount;	// the number of children
};

struct chatData {
	int roomID;
	char chatterSSID[64];
};

struct History{
	char chatterSSID[64];	
	char message[1024] = {0};
	time_t postTime;
};

// when use readXXX, remember to destory them

void    changeState(struct Thread* t, char statebit, bool op);
char*   resolveState(char state);
char*   readString(unqlite *pDb, char* key);
char*   readString_(unqlite *pDb, cclong key);
int     writeString(unqlite *pDb, char* key, const char* value, bool autoCommit);
cclong  writeString_(unqlite *pDb, char* value, bool autoCommit);
cclong  readcclong(unqlite *pDb, char* key);
cclong  readcclong_(unqlite *pDb, cclong key);
int     writecclong(unqlite *pDb, char* key, cclong value, bool autoCommit);
cclong  writecclong_(unqlite *pDb, cclong value, bool autoCommit);
struct Thread* readThread_(unqlite *pDb, cclong key);
cclong  writeNewThread(unqlite *pDb, struct Thread* t, bool autoCommit);
int     writeThread(unqlite *pDb, cclong key, struct Thread* t, bool autoCommit);
cclong  nextCounter(unqlite *pDb);
int     resetDatabase(unqlite *pDb);
cclong  findParent(unqlite *pDb, struct Thread* b);
int     deleteThread(unqlite *pDb, cclong tid);
int     newThread(unqlite *pDb, const char* content, char* author, const char* email, char* ssid, char* imgSrc, bool sega);
int     newReply(unqlite *pDb, cclong id, const char* content, char* author, const char* email, char* ssid, char* imgSrc, bool sega);
int     displayReply(unqlite *pDb, struct Thread *t);
int     listThread(unqlite *pDb);

#endif