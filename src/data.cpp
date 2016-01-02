#include "data.h"

LevelDB& LevelDB::global() {
  static LevelDB ldb;

  return ldb;
}

LevelDB::LevelDB()
{}

void LevelDB::init(std::string db_path) {
  std::string path = db_path + ".ldb";

  leveldb::Options options;

  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db);

  if (!status.ok()) {
    std::cerr << status.ToString() << std::endl;
  }
}

leveldb::Status LevelDB::put(const std::string key, const std::string value) {
  return db->Put(leveldb::WriteOptions(), key, value);
}

std::string LevelDB::get(const std::string key) {
  std::string ret;
  auto s = db->Get(leveldb::ReadOptions(), key, &ret);

  if (!s.ok())
  {
    char dummy[4] = { 0xC };
    ret = std::string(dummy, 4);
  }

  return ret;
}

std::mutex _Thread::counter_mutex;

_Thread::_Thread() {
  error = false;
}

void _Thread::set_current_time() {
  time_t rawtime;

  time(&rawtime);
  date = rawtime;
}

std::string _Thread::serialize() const {
  uint32_t buf_len = 4 + 2 + 4 * 6 + sizeof(time_t) +
                     4 + title.size() +
                     4 + author.size() +
                     4 + ip.size() +
                     4 + img.size() +
                     4 + content.size();

  char *buf = new char[buf_len];
  char *ret = buf;
  uint32_t tmp;

  std::memcpy(buf, &buf_len, 4);

  buf += 4;

  _THREAD_INT_CHAIN(_INT_SERIALIZATION);
  _THREAD_STRING_CHAIN(_STRING_SERIALIZATION);

  std::string rs(ret, buf_len);
  delete[] ret;

  return rs;
}

void _Thread::deserialize(std::string s) {
  char *buf = (char *)s.data();
  uint32_t tmp;
  uint32_t chk;

  std::memcpy(&chk, buf, 4);

  buf += 4;

  if (chk != s.size())
  {
    error = true;
    return;
  }

  _THREAD_INT_CHAIN(_INT_DESERIALIZATION);
  _THREAD_STRING_CHAIN(_STRING_DESERIALIZATION);
}

bool _Thread::write(const _Thread th) {
  if (th.error) return false;
  else
  {
    LevelDB ldb;
    auto    s = ldb.global().put(std::to_string(th.no), th.serialize());
    return s.ok();
  }
}

_Thread _Thread::read(const uint32_t id) {
  _Thread ret;
  LevelDB ldb;

  ret.deserialize(ldb.global().get(std::to_string(id)));

  return ret;
}

_Thread& _Thread::goto_next() {
  *this = _Thread::read(next);
  return *this;
}

_Thread& _Thread::goto_prev() {
  *this = _Thread::read(prev);
  return *this;
}

uint32_t _Thread::inc_counter() {
  std::lock_guard<std::mutex> cnt(_Thread::counter_mutex);

  _Thread root = _Thread::read(0);

  root.children_count++;
  _Thread::write(root);

  return root.children_count;
}

bool _Thread::insert_after(_Thread p, _Thread aft, leveldb::WriteBatch *bat) {
  _Thread  _pn = _Thread::read(p.next);
  _Thread& pn  = (_pn.no == p.no) ? p : _pn;

  p.next = aft.no;

  aft.prev = p.no;
  aft.next = pn.no;

  pn.prev = aft.no;

  LevelDB ldb;
  leveldb::WriteBatch *batch;

  if (bat != nullptr) batch = bat;
  else batch = new leveldb::WriteBatch();

  batch->Put(std::to_string(p.no),   p.serialize());
  batch->Put(std::to_string(aft.no), aft.serialize());
  batch->Put(std::to_string(pn.no),  pn.serialize());

  if (bat == nullptr)
  {
    auto s = ldb.global().db->Write(leveldb::WriteOptions(), batch);
    delete batch;
    return s.ok();
  }
  else return true;
}

bool _Thread::init(bool force_init) {
  _Thread root = _Thread::read(0);

  if (!root.error && !force_init) return true;

  root.error          = false;
  root.no             = 0;
  root.next           = 0;
  root.prev           = 0;
  root.children_count = 0;
  root.child          = 0;
  root.parent         = 0;
  root.content        = "root";
  root.set_current_time();
  root.state = 0; // root has a 0 state (invisible)

  return _Thread::write(root);
}

_Thread _Thread::make() {
  _Thread ret;

  ret.no             = inc_counter();
  ret.next           = 0;
  ret.child          = 0;
  ret.children_count = 0;
  ret.prev           = 0;
  ret.parent         = 0;
  ret.set_current_time();
  ret.state = _Thread::THREAD + _Thread::NORMAL;

  return ret;
}

_Thread _Thread::unlink(uint32_t tno) {
  _Thread  ret = _Thread::read(tno);
  _Thread  _p  = _Thread::read(ret.prev);
  _Thread  n   = _Thread::read(ret.next);
  _Thread& p   = (_p.no == n.no) ? n : _p;

  p.next = n.no;
  n.prev = p.no;

  LevelDB ldb;
  leveldb::WriteBatch batch;
  batch.Put(std::to_string(p.no), p.serialize());
  batch.Put(std::to_string(n.no), n.serialize());
  auto s = ldb.global().db->Write(leveldb::WriteOptions(), &batch);

  ret.prev  = 0;
  ret.next  = 0;
  ret.error = !s.ok();

  return ret;
}

bool _Thread::append_child(_Thread p, _Thread ch) {
  ch.state |= _Thread::REPLY;

  if (ch.state & _Thread::THREAD) ch.state -= _Thread::THREAD;

  LevelDB ldb;
  leveldb::WriteBatch batch;

  if (p.child == 0)
  {
    p.child          = ch.no;
    p.children_count = 1;
    ch.parent        = p.no;
    ch.next          = ch.prev = ch.no; // self circle
    batch.Put(std::to_string(p.no),  p.serialize());
    batch.Put(std::to_string(ch.no), ch.serialize());
  }
  else
  {
    _Thread begin_child = _Thread::read(p.child).goto_prev();
    p.children_count++;
    batch.Put(std::to_string(p.no), p.serialize());
    _Thread::insert_after(begin_child, ch, &batch);
  }

  auto s = ldb.global().db->Write(leveldb::WriteOptions(), &batch);

  if (!s.ok()) return false;

  if (p.state & _Thread::THREAD &&
      !(p.state & _Thread::SAGE)) // only an unsaged thread will be pushed to
                                  // top when new reply came
  {
    p = _Thread::unlink(p.no);

    if (p.error) return false;

    return _Thread::insert_after(_Thread::read(0), p);
  }
  else return s.ok();
}

int32_t _Thread::find_parent(_Thread th) {
  uint32_t startID = th.no;

  if (!(th.state & _Thread::NORMAL)) return -1;

  if (th.state & _Thread::THREAD) return 0;

  if (th.parent) return th.parent;

  while (th.goto_next().no != startID) {
    // b = unq_read_thread(pDb, b->nextThread);
    if (th.parent) return th.parent;
  }

  return 0;
}

std::string _Thread::resolve_state() {
  std::string ret = "";

  ret += state & NORMAL ? "Normal," : "Deleted,";
  ret += state & THREAD ? "Thread," : "";
  ret += state & REPLY ? "Reply," : "";
  ret += state & SAGE ? "Sage," : "";
  ret += state & LOCK ? "Locked," : "";

  return ret;
}

std::ostream& operator<<(std::ostream& os, _Thread& rhs)
{
  os << (rhs.state & _Thread::THREAD ? "Thread " : "Reply ") << "No." << rhs.no <<
  ":" << rhs.content << "::::" << rhs.children_count;
  return os;
}

void printt(_Thread r, std::string lv)
{
  auto i = r.no;
  std::cout << lv << r << std::endl;

  while (r.goto_next().no != i)
  {
    std::cout << lv << r << std::endl;

    if (r.child) printt(_Thread::read(r.child), lv + " ");
  }
}

#
