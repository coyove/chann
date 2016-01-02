#ifndef CHANN_LEVELDB_HEADER_INCLUDED
#define CHANN_LEVELDB_HEADER_INCLUDED

#include <string>
#include <cstring>
#include <stdint.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include "config.h"

class LevelDB {
public:
  leveldb::DB *db;

public:
  // Return the global object
  LevelDB& global();

  // This does nothing
  LevelDB();

  // Init the leveldb
  void            init(std::string);

  // Set value
  leveldb::Status put(const std::string key,
                      const std::string value);

  // Get value
  std::string     get(const std::string key);
};

class _Thread {
public:

  static std::mutex counter_mutex;
#define _INT_SERIALIZATION(X) std::memcpy(buf, &X, sizeof(X)); buf += sizeof(X);

#define _INT_DESERIALIZATION(X) std::memcpy(&X, buf, sizeof(X)); buf += sizeof(X);

#define _STRING_SERIALIZATION(X) tmp = X.size(); \
  std::memcpy(buf,      &tmp,      4);           \
  std::memcpy(buf += 4, X.c_str(), tmp);         \
  buf += tmp;

#define _STRING_DESERIALIZATION(X)               \
  std::memcpy(&tmp, buf, 4);                     \
  X.reserve(tmp + 1);                            \
  std::memcpy((char *)X.c_str(), buf += 4, tmp); \
  *((char *)X.c_str() + tmp) = 0;                \
  X                          = X.c_str();        \
  buf                       += tmp;

public:
  // State bit
  enum
  {
    NORMAL = 1,   // 00000001
    THREAD = 4,   // 00000100
    REPLY  = 2,   // 00000010
    SAGE   = 16,  // 00010000
    LOCK   = 128, // 10000000
    FULL   = 8    // 00001000
  };

public:

  std::string content;
  std::string title;
  std::string ip;
  std::string author;
  std::string img;
#define _THREAD_STRING_CHAIN(X) X(title) X(author) X(ip) X(img) X(content);

  time_t   date;
  uint16_t state;
  uint32_t no;
  uint32_t next;
  uint32_t prev;
  uint32_t child;
  uint32_t parent;
  uint32_t children_count;
#define _THREAD_INT_CHAIN(X) X(state) X(no) X(next) X(prev) X(child) X(parent) X( \
    children_count) X(date);
  bool error;

  // Init the thread and set error = false
  _Thread();

  // Set thread's date to current time
  void            set_current_time();

  // Serialize (this) to binary data
  std::string     serialize() const;

  // Deserialize binary data to (this)
  void            deserialize(std::string s);

  // State bit to readable string
  std::string     resolve_state();

  // Write a thread to leveldb
  static bool     write(const _Thread th);

  // Read a thread from leveldb, if failed, _Thread.error is true
  static _Thread  read(const uint32_t id);

  // Goto next thread
  _Thread       & goto_next();

  // Goto prev thread
  _Thread       & goto_prev();

  // Increase the counter by 1 and return the new value
  static uint32_t inc_counter();

  // Insert (aft) after (p), if (bat) provided, the operation will be stored inside
  static bool     insert_after(_Thread              p,
                               _Thread              aft,
                               leveldb::WriteBatch *bat = nullptr);

  // If the root is invalid, init the chain by resetting the root,
  // otherwise just return true
  static bool    init(bool f = false);

  // Make a new empty thread and return it
  static _Thread make();

  // Unlink a thread from the chain, if it is a reply, then just hide it
  static _Thread unlink(uint32_t tno);

  // Append a child to (p), (p) can be a thread or a reply
  static bool    append_child(_Thread p,
                              _Thread ch);

  // Find the parent of a given thread
  static int32_t find_parent(_Thread th);
};

extern std::mutex counter_mutex;

extern std::ostream& operator<<(std::ostream& os,
                                _Thread     & rhs);

extern void          printt(_Thread     r,
                            std::string lv);

#endif // ifndef CHANN_LEVELDB_HEADER_INCLUDED
