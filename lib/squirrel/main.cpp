// #define NS_ENABLE_DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>

extern "C" {
#include "../unqlite/unqlite.h"
#include <mysql/mysql.h>
}

#include "general.h"

using namespace std;

unqlite *pDb;
MYSQL   *conn;

#define TEST_ARG(b1, b2) (strcmp(argv[i], b1) == 0 || strcmp(argv[i], b2) == 0)

char* nowNow(){
    time_t rawtime;
    time(&rawtime);

    char *timetmp = new char[64];
    struct tm * timeinfo;
    timeinfo = localtime(&(rawtime));

    strftime(timetmp, 64, "%Y/%m/%d %X", timeinfo);
    return timetmp;//asctime(localtime(&rawtime));
}

void doThread(){
    struct Thread *r = readThread_(pDb, 0); 
    struct Thread *t;

    // int i = 1;
    for(cclong i = r->childCount; i > 0; i--){
        t = readThread_(pDb, i);
        
        char *stat = "INSERT INTO `cchan` (`id`, `title`, `date`, `poster`, `comment`, `image`, `ip`, `state`, `next_thread`, `prev_thread`, `child_thread`, `parent_thread`, `children_count`) "
        "VALUES ('%d','%s',%d,'%s','%s','%s','%s','%c','%d','%d','%d','%d','%d')";

        char z_title[256] = {0};
        mysql_real_escape_string(conn, z_title, t->author, strlen(t->author));

        char z_comment[8192] = {0};

        char* comment = readString(pDb, t->content);

        if(!comment) continue;

        mysql_real_escape_string(conn, z_comment, comment, strlen(comment));
        if(comment) delete comment;

    
        char query[16384] = {0};
        int len;
        
        len = snprintf(query, 16384, stat, 
            t->threadID,
            z_title,
            t->date,
            t->ssid,
            z_comment,
            t->imgSrc,
            t->email,
            t->state,
            t->nextThread,
            t->prevThread,
            t->childThread,
            t->parentThread,
            t->childCount);

        mysql_real_query(conn, query, len);
        printf("No.%d dumped to mysql\n", i);

        if(t) delete t;
    }
}

int main(int argc, char *argv[]){
    char zPath[64] = "default.db"; 
    char m_user[64] = "root";
    char m_pass[64] = "";
    char m_table[64] = "cchan";
    
    for (int i = 0; i < argc; ++i){
        if(TEST_ARG("--database",   "-d")){ strncpy(zPath, argv[++i], 64); continue; }
        if(TEST_ARG("--table",      "-t")){ strncpy(m_table, argv[++i], 64); continue; }
        if(TEST_ARG("--password",   "-p")){ strncpy(m_pass, argv[++i], 64); continue; }
        if(TEST_ARG("--user",       "-u")){ strncpy(m_user, argv[++i], 64); continue; }
    }

    if (unqlite_open(&pDb, zPath, UNQLITE_READ_ONLY) != UNQLITE_OK) printf("Unqlite error\n");

    printf("Dumping Database '%s'\n", zPath);

    conn = mysql_init(NULL);
    mysql_real_connect(conn, "localhost", m_user, m_pass, m_table, 0, NULL, 0);
    // mysql_real_query(conn, "use cchan", 100);

    doThread();
    printf("Dumping Job Finished @ %s\n", nowNow());

    mysql_close(conn);
    unqlite_close(pDb);

    return 0;
}