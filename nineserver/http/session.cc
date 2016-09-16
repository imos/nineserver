#include "nineserver/http/session.h"

#include <memory>
#include <functional>

#include "base/base.h"

DEFINE_int32(num_sessions, 10000, "# of sessions to accept.");

vector<Session::Deleter>* Session::deleters_ = nullptr;
std::mutex Session::deleters_mutex_;

namespace {

class SessionPool {
 public:
  SessionPool() {}

  static SessionPool* GetSessionPool() {
    static SessionPool* pool = new SessionPool();
    return pool;
  }

  void Compact() {
    if (sessions_.size() < FLAGS_num_sessions * 2) { return; }
    vector<pair<pair<bool, int64>, std::shared_ptr<Session>>> sessions;
    sessions.reserve(sessions_.size());
    for (const auto& key_and_session : sessions_) {
      std::shared_ptr<Session> session = key_and_session.second;
      auto session_guard = session->Lock();
      sessions.emplace_back(
          make_pair<bool, int64>(!session->Empty(), session->session_access()),
          std::move(session));
    }
    sort(sessions.rbegin(), sessions.rend());
    if (sessions.size() > FLAGS_num_sessions) {
      sessions.resize(FLAGS_num_sessions);
    }
    sessions_.clear();
    for (const auto& access_and_session : sessions) {
      std::shared_ptr<Session> session = access_and_session.second;
      sessions_.emplace(session->session_key(), std::move(session));
    }
  }

  std::shared_ptr<Session> Get(StringPiece session_key, bool add_session) {
    std::lock_guard<std::mutex> sessions_guard_(sessions_lock_);
    std::shared_ptr<Session>* session = FindOrNull(sessions_, session_key);
    if (session != nullptr) { return *session; }
    if (!add_session) { return nullptr; }
    Compact();
    std::shared_ptr<Session> new_session(new Session(session_key));
    sessions_.emplace(new_session->session_key(), new_session);
    return new_session;
  }

  bool Set(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> sessions_guard_(sessions_lock_);
    std::shared_ptr<Session>* current_session =
      FindOrNull(sessions_, session->session_key());
    if (current_session != nullptr && *current_session == session) {
      return false;
    }
    sessions_[session->session_key()] = session;
    return true;
  }

  int Remove(StringPiece session_key) {
    std::lock_guard<std::mutex> sessions_guard_(sessions_lock_);
    return sessions_.erase(session_key);
  }

 private:
  map<StringPiece, std::shared_ptr<Session>> sessions_;
  std::mutex sessions_lock_;
};

}  // namespace

std::shared_ptr<Session> GetSession(StringPiece session_key) {
  std::shared_ptr<Session> session =
      SessionPool::GetSessionPool()->Get(session_key, true);
  // Prevent acquiring a locked session.
  auto lock = session->Lock();
  return session;
}

bool SetSession(std::shared_ptr<Session> session) {
  return SessionPool::GetSessionPool()->Set(session);
}

bool RemoveSession(StringPiece session_key) {
  std::shared_ptr<Session> session =
      SessionPool::GetSessionPool()->Get(session_key, false);
  if (session == nullptr) { return false; }
  auto lock = session->Lock();
  return SessionPool::GetSessionPool()->Remove(session_key) > 0;
}
