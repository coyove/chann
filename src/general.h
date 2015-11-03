#ifndef CHANN_GENERAL_HEADER_INCLUDED
#define CHANN_GENERAL_HEADER_INCLUDED

extern "C"{
#include "../lib/unqlite/unqlite.h"
}

#include <stdio.h>
#include <string>
#include <cstring>
#include <time.h>

#define NORMAL_DISPLAY  1
#define THREAD_REPLY    2
#define MAIN_THREAD     4
#define TOOMANY_REPLIES 8
#define SAGE_THREAD     16
#define LOCKED_THREAD   128

#if defined(_WIN32)
#define cclong long
#else
#define cclong int
#endif

// length: 205 bytes
struct Thread{
    char state;         // thread's state
    /*
        1       |1      |1      |1      |1      |1      |1      |1
        locked  |----   |----   |sage   |toomany|thread |reply  |normal

        old representation:
        01101101 = m
        01100011 = c
        01100100 = d
    */
    char content[16];   // (pointer) thread's content 
    char author[64];    // thread's title
    char email[64];     // thread's ip
    char ssid[10];      // thread's author
    time_t date;        // submitted date and time
    char imgSrc[16];    // if thread has an image attached
    int threadID;       // thread's ID
    int nextThread;     // (pointer) next thread's address
    int prevThread;     // (pointer) prev thread's address
    int childThread;    // (pointer) child thread's address (replies), root thread has no child thread
    int parentThread;
    int childCount;     // the number of children
};

void    
changeState(struct Thread* t, char statebit, bool op);

std::string 
unq_resolve_state(char state);

const char* 
unq_read_string(unqlite *pDb, const char* key);

int     
unq_write_string(unqlite *pDb, const char* key, const char* value, bool autoCommit);

int     
unq_write_int(unqlite *pDb, const char* key, int value, bool autoCommit);

struct Thread* 
unq_read_thread(unqlite *pDb, int key);

int     
unq_write_thread(unqlite *pDb, int key, struct Thread* t, bool autoCommit);

int     
unq_reset(unqlite *pDb);

int  
unq_thread_parent(unqlite *pDb, struct Thread* b);

int     
unq_delete_thread(unqlite *pDb, int tid);

int 
unq_new_thread(unqlite *pDb, 
                const char* content, 
                const char* title, 
                const char* ip, 
                const char* username, 
                const char* image, bool sage);

int 
unq_new_reply(unqlite *pDb, 
                int id, 
                const char* content, 
                const char* title, 
                const char* ip, 
                const char* username, 
                const char* image, bool sage);

#endif