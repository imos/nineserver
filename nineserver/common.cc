#include "nineserver/common.h"

#include <sys/time.h>
#include <sys/types.h> 
#include <time.h>
#include <unistd.h>
#include "base/base.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

bool CaseInsensitiveEqual(StringPiece l, StringPiece r) {
  if (l.size() != r.size()) { return false; }
  return strncasecmp(l.data(), r.data(), l.size()) == 0;
}

double GetThreadClock() {
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  thread_port_t thread = mach_thread_self();
  mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
  thread_basic_info_data_t info;
  CHECK_EQ(thread_info(thread, THREAD_BASIC_INFO, (thread_info_t)&info, &count),
           KERN_SUCCESS);
  mach_port_deallocate(mach_task_self(), thread);
  return info.user_time.seconds + info.system_time.seconds +
         (info.user_time.microseconds + info.system_time.microseconds) * 1e-6;
  // clock_serv_t cclock;
  // mach_timespec_t mts;
  // host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  // clock_get_time(cclock, &mts);
  // mach_port_deallocate(mach_task_self(), cclock);
  // clock.tv_sec = mts.tv_sec;
  // clock.tv_nsec = mts.tv_nsec;
#else
  struct timespec clock;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &clock);
  return clock.tv_sec + clock.tv_nsec * 1e-9;
#endif
}

double GetCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}

int64 Rand() {
  thread_local uint32 x = GetCurrentTime() * 1e6;
  thread_local uint32 y = getpid();
  thread_local uint32 z = 521288629;
  thread_local uint32 w = 88675123;
  unsigned long t = (x ^ (x << 11));
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}
