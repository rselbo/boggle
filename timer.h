#pragma once
#if defined(POSIX)
#include <time.h>
#else
#include <windows.h>
#endif

class Timer
{
public:
  Timer() {}
  ~Timer() {}
  
  void Start();
  uint64_t Stop();
  uint64_t GetResolution();
private:
#if defined(POSIX)
  struct timespec m_Start;
#else
  LARGE_INTEGER m_Start;
#endif
};

#if defined(POSIX)
void Timer::Start()
{
  clock_gettime(CLOCK_REALTIME, &m_Start);
}

uint64_t Timer::Stop()
{
  struct timespec end, diff;
  clock_gettime(CLOCK_REALTIME, &end);

  if ((end.tv_nsec-m_Start.tv_nsec)<0) 
  {
    diff.tv_sec = end.tv_sec-m_Start.tv_sec-1;
    diff.tv_nsec = 1000000000+end.tv_nsec-m_Start.tv_nsec;
  } 
  else 
  {
    diff.tv_sec = end.tv_sec-m_Start.tv_sec;
    diff.tv_nsec = end.tv_nsec-m_Start.tv_nsec;
  }
  uint64_t elapsed = diff.tv_nsec + 1000000000 * diff.tv_sec;
  return elapsed;
}

uint64_t Timer::GetResolution()
{
  struct timespec res;
  clock_getres(CLOCK_REALTIME, &res);
  uint64_t resolution = res.tv_nsec + 1000000000 * res.tv_sec;
  return resolution * 1000000000;
}
#else
void Timer::Start()
{
  QueryPerformanceCounter(&m_Start);
}

uint64_t Timer::Stop()
{
  LARGE_INTEGER end;
  QueryPerformanceCounter(&end);
  uint64_t elapsed = end.QuadPart - m_Start.QuadPart;
  return elapsed;
}

uint64_t Timer::GetResolution()
{
  LARGE_INTEGER res;
  QueryPerformanceFrequency(&res);
  return  res.QuadPart;
}
#endif
