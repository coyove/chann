#include "general.h"

using namespace std;

void changeState(struct Thread* t, char statebit, bool op){
    char truebit = statebit;

    if(op){ // add a state
        if(t->state & truebit){} // already has the state
        else
            t->state += truebit;
    }else{
        if(t->state & truebit)
            t->state -= truebit;
        else{} // already lose the state
    }
}

string unq_resolve_state(char state){
    string ret = "";
    
    ret += state & NORMAL_DISPLAY ? "Normal," : "Deleted,";
    ret += state & MAIN_THREAD ? "Thread," : "";
    ret += state & THREAD_REPLY ? "Reply," : "";
    ret += state & SAGE_THREAD ? "Sage," : "";
    ret += state & LOCKED_THREAD ? "Locked," : "";

    return ret;
}

int unq_reset(unqlite *pDb){
    int rc = unq_write_int(pDb, "global_counter", 0, true);
    if (!rc) return 0;

    //construct the root thread
    cclong *key = new cclong(0);
    struct Thread *t = new Thread();
    t->threadID = 0;
    //t->state = 'r';
    t->nextThread = 0;
    t->childCount = 0;

    strcpy(t->content, "slogan");

    rc = unqlite_kv_store(pDb, key, 4, t, sizeof(struct Thread));
    if (rc != UNQLITE_OK)
        return 0;
    else{
        unqlite_commit(pDb);
        return 1;
    }
}

int unq_thread_parent(unqlite *pDb, struct Thread* b){
    // struct Thread* b = unq_read_thread(pDb, startID);
    int startID = b->threadID;
    if (!(b->state & NORMAL_DISPLAY)) return -1;
    if (b->state & MAIN_THREAD) return 0;
    if (b->parentThread) return b->parentThread;

    while (b->nextThread != startID){
        b = unq_read_thread(pDb, b->nextThread);
        if (b->parentThread) return b->parentThread;
    }

    return 0;
}

int unq_delete_thread(unqlite *pDb, int tid){
    // cchan will never truely delete a thread, it just hides it
    struct Thread* t = unq_read_thread(pDb, tid);
    struct Thread* tp = unq_read_thread(pDb, t->prevThread);
    struct Thread* tn = unq_read_thread(pDb, t->nextThread);

    if (!(t->state & NORMAL_DISPLAY)) return 0;

    if (t->parentThread == 0 && t->state & MAIN_THREAD){
        tn->prevThread = t->prevThread;
        tp->nextThread = t->nextThread;

        if (t->prevThread == t->nextThread){
            //this happens only if the thread has one child reply
            tn->nextThread = tn->threadID;
            unq_write_thread(pDb, tn->threadID, tn, false);
        }
        else{
            unq_write_thread(pDb, tn->threadID, tn, false);
            unq_write_thread(pDb, tp->threadID, tp, false);
        }

        //t->state = 'd';
        //t->state -= NORMAL_DISPLAY;
        changeState(t, NORMAL_DISPLAY, false);
        unq_write_thread(pDb, t->threadID, t, false);
    }

    if (!(t->state & MAIN_THREAD)){
        changeState(t, NORMAL_DISPLAY, false);
        unq_write_thread(pDb, t->threadID, t, false);
    }

    unqlite_commit(pDb);

    return 1;
}

int unq_new_thread(unqlite *pDb, 
    const char* content, 
    const char* title, 
    const char* ip, 
    const char* username, 
    const char* image, bool sega)
{
    struct Thread *t = new Thread();

    char contentkey[16];
    unqlite_util_random_string(pDb, contentkey, 15);
    contentkey[15] = 0;
    unq_write_string(pDb, contentkey, content, false);

    strncpy(t->content, contentkey, 16);
    strncpy(t->author, title, 64);
    strncpy(t->email, ip, 64);
    strncpy(t->ssid, username, 10);
    strncpy(t->imgSrc, image, 16);

    unqlite_commit(pDb);
    t->state = MAIN_THREAD + NORMAL_DISPLAY;
    if (sega) t->state += SAGE_THREAD;

    time_t rawtime;
    time(&rawtime);
    t->date = rawtime;

    struct Thread *r = unq_read_thread(pDb, 0); // get the root thread
    int oriNextThread = r->nextThread;

    //insert
    t->nextThread = r->nextThread;
    t->prevThread = 0; //point to the root
    t->childThread = 0; //no replies yet
    t->parentThread = 0;

    int newkey = ++r->childCount; //nextCounter(pDb);
    t->threadID = newkey;

    unq_write_thread(pDb, newkey, t, false);

    r->nextThread = newkey;
    unq_write_thread(pDb, 0, r, false);

    if (oriNextThread){
        struct Thread *rn = unq_read_thread(pDb, oriNextThread);
        rn->prevThread = newkey;
        unq_write_thread(pDb, oriNextThread, rn, false);
    }

    unqlite_commit(pDb);

    delete t;
    delete r;

    return 1;
}

int unq_new_reply(unqlite *pDb, 
    int id, 
    const char* content, 
    const char* title, 
    const char* ip, 
    const char* username, 
    const char* image, bool sega)
{
    struct Thread *r = unq_read_thread(pDb, id);
    struct Thread *root = unq_read_thread(pDb, 0);
    struct Thread *self = r;
    struct Thread *t = new Thread();
    bool reply2reply = false;

    if (r->state & THREAD_REPLY) reply2reply = true;

    char contentkey[16];
    unqlite_util_random_string(pDb, contentkey, 15);
    contentkey[15] = 0;
    unq_write_string(pDb, contentkey, content, false);

    strncpy(t->content, contentkey, 16);
    strncpy(t->author, title, 64);
    strncpy(t->email, ip, 64);
    strncpy(t->ssid, username, 10);
    strncpy(t->imgSrc, image, 16);

    unqlite_commit(pDb);
    t->state = THREAD_REPLY + NORMAL_DISPLAY;
    if (sega) t->state += SAGE_THREAD;

    time_t rawtime;
    time(&rawtime);
    t->date = rawtime;

    if (r->childThread == 0){ //no reply yet
        int newkey = ++root->childCount;// nextCounter(pDb);
        t->nextThread = newkey;
        t->prevThread = newkey;
        t->childThread = 0;
        t->parentThread = r->threadID;
        t->threadID = newkey;
        //it's strange and wired but we update the count of children here
        t->childCount = 1;
        unq_write_thread(pDb, newkey, t, false);
        r->childThread = newkey;
        unq_write_thread(pDb, r->threadID, r, false);
    }
    else{ // construct a circled linked list of threads
        r = unq_read_thread(pDb, r->childThread);
        r->childCount++;
        
        struct Thread *rp;
        if (r->prevThread == r->threadID && r->nextThread == r->threadID) // only one child
            rp = r;
        else
            rp = unq_read_thread(pDb, r->prevThread);

        int newkey = ++root->childCount; //nextCounter(pDb);

        t->prevThread = r->prevThread;
        t->nextThread = r->threadID;
        t->childThread = t->parentThread = 0;
        t->threadID = newkey;

        r->prevThread = newkey;
        rp->nextThread = newkey;

        unq_write_thread(pDb, newkey, t, false);
        unq_write_thread(pDb, r->threadID, r, false);
        unq_write_thread(pDb, rp->threadID, rp, false);

    }

    
    if (self->prevThread == 0 || self->state & SAGE_THREAD || t->state & SAGE_THREAD || reply2reply){
        //under certain circumstance the thread won't be pushed to the top
        //1. already at top
        //2. is a sega thread
        //3. has a sega reply
        //4. what it replies to is a comment itself
    }
    else{//push the replied thread to the top
        if (self->prevThread) { 
            struct Thread* sp = unq_read_thread(pDb, self->prevThread);
            sp->nextThread = self->nextThread;
            unq_write_thread(pDb, sp->threadID, sp, false);

            if (self->nextThread){
                struct Thread* sn = unq_read_thread(pDb, self->nextThread);
                sn->prevThread = sp->threadID;
                unq_write_thread(pDb, sn->threadID, sn, false);
            }
        }

        self->nextThread = root->nextThread;
        self->prevThread = 0;
        root->nextThread = self->threadID;

        if (self->nextThread){
            struct Thread* sn = unq_read_thread(pDb, self->nextThread);
            sn->prevThread = self->threadID;
            unq_write_thread(pDb, sn->threadID, sn, false);
        }

        unq_write_thread(pDb, self->threadID, self, false);

    }

    unq_write_thread(pDb, 0, root, false);
    unqlite_commit(pDb);

    
    delete root;
    if (self != r) delete self;
    delete r;
    delete t;

    return 1;
}

int displayReply(unqlite *pDb, struct Thread *t){
    cclong ct = t->childThread;
    struct Thread *r = unq_read_thread(pDb, ct); // beginning of the circle
    cclong rid = r->threadID; //the ID

    struct tm * timeinfo;
    timeinfo = localtime(&(r->date));
    fprintf(stdout, "  %d Reply(s)\n", r->childCount);
    fprintf(stdout, "  Reply-ID: %d, Author: %s, Date: %s  %s\n", rid, r->author, asctime(timeinfo), unq_read_string(pDb, r->content));

    while (r->prevThread != rid){
        r = unq_read_thread(pDb, r->prevThread);
        timeinfo = localtime(&(r->date));
        fprintf(stdout, "  Reply-ID: %d, Author: %s, Date: %s  %s\n", r->threadID, r->author, asctime(timeinfo), unq_read_string(pDb, r->content));
    }

    return 1;
}

int listThread(unqlite *pDb){
    struct Thread *r = unq_read_thread(pDb, 0); // get the root thread
    //iterThread(pDb, r);

    while (r->nextThread){
        r = unq_read_thread(pDb, r->nextThread);

        struct tm * timeinfo;
        timeinfo = localtime(&(r->date));
        fprintf(stdout, "ID: %d, Author: %s, Date: %s%s\n", r->threadID, r->author, asctime(timeinfo), unq_read_string(pDb, r->content));

        if (r->childThread) displayReply(pDb, r);
    }

    return 1;
}

int unq_write_string(unqlite *pDb, const char* key, const char* value, bool autoCommit){
    int rc;
    rc = unqlite_kv_store_fmt(pDb, key, -1, value); //simple
    if (rc != UNQLITE_OK)
        return 0;
    else{
        if (autoCommit) unqlite_commit(pDb);
        return 1;
    }
}

const char* unq_read_string(unqlite *pDb, const char* key){
    unqlite_int64 nBytes;
    int rc;
    rc = unqlite_kv_fetch(pDb, key, -1, NULL, &nBytes);
    if (rc != UNQLITE_OK)
        return 0;

    char *zBuf = new char[++nBytes];
    if (zBuf == NULL) 
        return 0;

    rc = unqlite_kv_fetch(pDb, key, -1, zBuf, &nBytes);
    if (rc != UNQLITE_OK)
        return 0;

    zBuf[nBytes] = 0;

    return zBuf;
}

int unq_write_int(unqlite *pDb, const char* key, int value, bool autoCommit){
    int* zzz = new cclong(value);
    int rc;
    rc = unqlite_kv_store(pDb, key, -1, zzz, 4);
    delete zzz;

    if (rc != UNQLITE_OK)
        return 0;
    else{
        if(autoCommit) unqlite_commit(pDb);
        return 1;
    }
}

int unq_read_int(unqlite *pDb, int key){
    unqlite_int64 nBytes = 4;
    int *k = new int(key);
    int rc;
    int* zBuf = new int(0);

    rc = unqlite_kv_fetch(pDb, k, 4, zBuf, &nBytes);
    delete k;

    // if (rc != UNQLITE_OK)
    //  return 0;

    return *zBuf;
}

struct Thread* unq_read_thread(unqlite *pDb, int key){
    unqlite_int64 nBytes = sizeof(struct Thread);
    // cclong *k = new cclong(key);
    int k = key;

    int rc;
    struct Thread* zBuf = new Thread();

    rc = unqlite_kv_fetch(pDb, &k, 4, zBuf, &nBytes);
    
    // delete k;
    //even the fetch function failed, we return an empty structure

    return zBuf;
}

std::shared_ptr<struct Thread> unq_read_thread_sp(unqlite * pDb, int key)
{
    unqlite_int64 nBytes = sizeof(struct Thread);
    std::shared_ptr<struct Thread> ret = std::shared_ptr<struct Thread>(new Thread());

    unqlite_kv_fetch(pDb, &key, 4, ret.get(), &nBytes);

    return ret;
}

int unq_write_thread(unqlite *pDb, cclong key, struct Thread* t, bool autoCommit){
    
    int k = key;
    int rc;
    rc = unqlite_kv_store(pDb, &k, 4, t, sizeof(struct Thread));

    if (rc != UNQLITE_OK)
        return 0;
    else{
        if (autoCommit) unqlite_commit(pDb);
        return 1;
    }
}