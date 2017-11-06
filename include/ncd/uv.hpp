#ifndef NCD_UV_HPP
# define NCD_UV_HPP

# include <uv.h>

namespace ncd {

//=============================================================================
// NAN UV RAII Wrappers
//=============================================================================

class Mutex {
public:
  Mutex() { uv_mutex_init(&mMutex); }
  ~Mutex() { uv_mutex_destroy(&mMutex); }

  void lock() { uv_mutex_lock(&mMutex); }
  void unlock() { uv_mutex_unlock(&mMutex); }

private:
  friend class Condition;
  uv_mutex_t mMutex;
};

class ScopedLock {
public:
  ScopedLock(Mutex & mutex) : mMutex(mutex) { mMutex.lock(); }
  ~ScopedLock() { mMutex.unlock(); }

  void lock() { mMutex.lock(); }
  void unlock() { mMutex.unlock(); }

private:
  Mutex & mMutex;
};


class Condition {
public:
  Condition()  { uv_cond_init(&mCondition); }
  ~Condition() { uv_cond_destroy(&mCondition); }

  void wait(Mutex & mutex) { uv_cond_wait(&mCondition, &mutex.mMutex); }
  void signal() { uv_cond_signal(&mCondition); }
  void broadcast() { uv_cond_broadcast(&mCondition); }

private:
  uv_cond_t mCondition;
};

template <typename T, typename UVHandle>
T*
nauv_unwrap(UVHandle * handle) {
  return static_cast<T*>(handle->data);
}

template <typename Key, typename T>
class ThreadLocal {
public:
  using ValueType = T;

  static
  void
  set(ValueType * value) {
    init();
    uv_key_set(&sKey, value);
  }

  static
  ValueType*
  get() {
    init();
    return static_cast<ValueType*>(uv_key_get(&sKey));
  }

  static
  void
  init() {
    if (!sInitialized) {
      uv_key_create(&sKey);
      sInitialized = true;
    }
  }
private:
  static bool sInitialized;
  static uv_key_t sKey;
};

template <typename Key, typename T>
bool
ThreadLocal<Key,T>::sInitialized = false;

template <typename Key, typename T>
uv_key_t
ThreadLocal<Key,T>::sKey = uv_key_t();

}  // end of namespace ncd
#endif  // NCD_UV_HPP
