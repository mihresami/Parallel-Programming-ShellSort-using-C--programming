#ifndef HRTIMER_H
#define HRTIMERS_H

#if defined(sparc)
#include <sys/time.h>
#elif defined(_i386) || defined(_tune_i686_) || defined (_x86_64_) || defined(_x86_64) || defined(_amd64)
#include <string.h>

typedef long long int hrtime_t;

extern "C" {
  __inline__ hrtime_t rdtsc() {
    unsigned long int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return (hrtime_t)hi <<32 | lo;
}

}
#endif
class HRTimer {
 public:
  HRTimer() {
#if defined(__linux) || defined(__linux__) || defined(linux)
    FILE* cpuinfo;
    char str[100];
    cpuinfo = fopen("/proc/cpuinfo","r");
    while(fgets(str,100,cpuinfo) != NULL){
      char cmp_str[8];
      strncpy(cmp_str, str, 7);
      cmp_str[7] = '\0';
      if(strcmp(cmp_str, "cpu MHz") == 0){
        double cpu_mhz;
        sscanf(str, "cpu MHz : %lf", &cpu_mhz);
        m_cpu_mhz = cpu_mhz;
        break;
      }
    }

    fclose( cpuinfo );

#endif
  }

  hrtime_t get_time_ns() {
#if defined(sparc)
    return gethrtime();
#elif defined(__i386) || defined(__tune_i686__) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64)
    return static_cast<hrtime_t> (((double)(rdtsc())*1.0e3)/(m_cpu_mhz));
#endif
  }

 private:
#if defined(__linux) || defined(__linux__) || defined(linux)
  double m_cpu_mhz;
#endif
};

#endif
