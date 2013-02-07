/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef	CXX_SYS_LOCKER_H
#define	CXX_SYS_LOCKER_H
#include <cassert>

namespace cxx {
    namespace sys {
        namespace detail {

            template<typename Lock >
            class syncobj {
            public:
                static void acquire(Lock& l) { l.acquire(); }
                static void acquire(Lock& l, bool read) { l.acquire(read); }
                static bool trylock(Lock& l) { return l.trylock(); }
                static bool waitfor(Lock& l, unsigned long msec) { return l.waitfor(msec); }
                static void release(Lock& l) { l.release(); }
                static void release(Lock& l, bool read) { l.release(read); }
            private:
                syncobj();
                syncobj(const syncobj& );
                syncobj& operator= (const syncobj& );
            };

            template<typename Lock >
            class scopelock {
            public:
                explicit scopelock(Lock& mtx, bool lock = true)
                    : locker(mtx), locked(false)
                {
                    if(lock) acquire();
                }
                ~scopelock()
                {
                    if(locked) release();
                }
                void acquire()
                {
                    if(!locked) {
                        syncobj<Lock>::acquire(locker);
                        locked = true;
                    }
                }
                void release()
                {
                    if(locked) {
                        syncobj<Lock>::release(locker);
                        locked = false;
                    }
                }

                bool islockd() const
                {
                    return locked;
                }
                operator const void*() const
                {
                    return locked ? this : 0;
                }
            private:
                scopelock(const scopelock& );
                scopelock& operator= (const scopelock& );

                Lock&	locker;
                bool	locked;
            };

            template<typename Lock >
            class triedlock {
            public:
                explicit triedlock(Lock& mtx)
                    : locker(mtx), locked(false)
                {
                    trylock();
                }
                triedlock(Lock& mtx, bool lock)
                    : locker(mtx), locked(false)
                {
                    if(lock) acquire();
                }
                ~triedlock()
                {
                    if(locked) release();
                }

                void acquire()
                {
                    if(!locked) {
                        syncobj<Lock>::acquire(locker);
                        locked = true;
                    }
                }
                void release()
                {
                    if(locked) {
                        syncobj<Lock>::release(locker);
                        locked = false;
                    }
                }
                bool trylock()
                {
                    // already locked, trylock will deadlock
                    assert(!locked);
                    return (locked = syncobj<Lock>::trylock(locker));
                }

                bool islockd() const
                {
                    return locked;
                }

                operator const void*() const
                {
                    return locked ? this : 0;
                }
            private:
                triedlock(const triedlock& );
                triedlock& operator= (const triedlock& );

                Lock&	locker;
                bool	locked;
            };

            template<typename Lock>
            class timedlock {
            public:
                timedlock(Lock& mtx, unsigned long msec)
                    : locker(mtx), locked(false)
                {
                    waitfor(msec);
                }
                timedlock(Lock& mtx, bool lock)
                    : locker(mtx), locked(false)
                {
                    if(lock) acquire();
                }
                ~timedlock()
                {
                    if(locked) release();
                }
                void acquire()
                {
                    if(!locked) {
                        syncobj<Lock>::acquire(locker);
                        locked = true;
                    }
                }
                void release()
                {
                    if(locked) {
                        syncobj<Lock>::release(locker);
                        locked = false;
                    }
                }
                bool waitfor(unsigned long msec)
                {
                    // already locked, and waitfor will deadlock
                    assert(!locked);
                    return (locked = syncobj<Lock>::waitfor(locker, msec));
                }

                bool islockd() const
                {
                    return locked;
                }
                operator const void*() const
                {
                    return locked ? this : 0;
                }
            private:
                timedlock(const timedlock& );
                timedlock& operator= (timedlock& );

                Lock&	locker;
                bool	locked;
            };

            template<typename Lock>
            class readerlock {
            public:
                readerlock(Lock& mtx) : locker(mtx), locked(false)
                {
                    acquire();
                }
                ~readerlock()
                {
                    if(locked) release();
                }

                void acquire()
                {
                    if(!locked) {
                        syncobj<Lock>::acquire(locker, true);
                        locked = true;
                    }
                }

                void release()
                {
                    if(locked) {
                        syncobj<Lock>::release(locker, true);
                        locked = false;
                    }
                }

                bool islockd() const
                {
                    return locked;
                }

                operator const void*() const
                {
                    return locked ? this : 0;
                }
            private:
                readerlock(const readerlock& );
                readerlock& operator= (const readerlock& );

                Lock&	locker;
                bool	locked;
            };

            template<typename Lock>
            class writerlock {
            public:
                writerlock(Lock& mtx) : locker(mtx), locked(false)
                {
                    acquire();
                }
                ~writerlock()
                {
                    release();
                }

                void acquire()
                {
                    if(!locked) {
                        syncobj<Lock>::acquire(locker, false);
                        locked = true;
                    }
                }
                void release()
                {
                    if(locked) {
                        syncobj<Lock>::release(locker, false);
                        locked = false;
                    }
                }

                bool islockd() const
                {
                    return locked;
                }

                operator const void*() const
                {
                    return locked ? this : 0;
                }
            private:
                writerlock(const writerlock& );
                writerlock& operator= (const writerlock& );

                Lock&	locker;
                bool	locked;
            };

        }
	}
}
#endif
