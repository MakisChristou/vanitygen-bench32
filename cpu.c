/* cpu.c - CPU scheduler routines */

#include "externs.h"

// below code taken and modified from https://www.hybridkernel.com/2015/01/18/binding_threads_to_cores_osx.html
// this code is needed because Apple does not support Linux's APIs.
#ifdef __APPLE__
  #include <sys/sysctl.h>
  #include <pthread.h>
  #include <mach/thread_act.h>
  #define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"

  typedef struct cpu_set {
    uint32_t    count;
  } cpu_set_t;

  static inline void
  CPU_ZERO_S(size_t setsize, cpu_set_t *cs) { cs->count = 0; }

  static inline void
  CPU_SET_S(int num, size_t setsize, cpu_set_t *cs) { cs->count |= (1 << num); }

  static inline int
  CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

  static inline int
  CPU_ISSET_S(int num, size_t setsize, cpu_set_t *cs) { return (cs->count & (1 << num)); }

  cpu_set_t *
  CPU_ALLOC(int num) { return (cpu_set_t *)malloc(sizeof(cpu_set_t)); }

  void 
  CPU_FREE(cpu_set_t *set) { free(set); }

  size_t
  CPU_ALLOC_SIZE(int num) { return sizeof(cpu_set_t); }

  static inline int
  CPU_COUNT_S(size_t setsize, cpu_set_t *set) { return set->count; }

  int sched_getaffinity(pid_t pid, size_t cpu_size, cpu_set_t *cpu_set)
  {
    int32_t core_count = 0;
    size_t  len = sizeof(core_count);
    int ret = sysctlbyname(SYSCTL_CORE_COUNT, &core_count, &len, 0, 0);
    if (ret) {
      printf("error while get core count %d\n", ret);
      return -1;
    }
    cpu_set->count = 0;
    for (int i = 0; i < core_count; i++) {
      cpu_set->count |= (1 << i);
    }

    return 0;
  }

  int sched_setaffinity(pthread_t thread, size_t cpu_size,
                           cpu_set_t *cpu_set)
  {
    thread_port_t mach_thread;
    int core = 0;
  
    for (core = 0; core < 8 * cpu_size; core++) {
      if (CPU_ISSET(core, cpu_set)) break;
    }
    printf("binding to core %d\n", core);
    thread_affinity_policy_data_t policy = { core };
    mach_thread = pthread_mach_thread_np(thread);
    thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                      (thread_policy_t)&policy, 1);
    return 0;
  }
#endif

static cpu_set_t *cpuset;
static int cpuset_ncpu;
static size_t cpuset_size;


// Return a count of the CPUs currently available to this process.
//
int get_num_cpus()
{
  cpuset_ncpu=1024;  /* Starting number of CPUs */

  while(1) {
    cpuset=CPU_ALLOC(cpuset_ncpu);
    cpuset_size=CPU_ALLOC_SIZE(cpuset_ncpu);

    if(!sched_getaffinity(0, cpuset_size, cpuset))
      return CPU_COUNT_S(cpuset_size, cpuset);

    if(errno == EINVAL) {
      /* Loop, doubling the cpuset, until sched_getaffinity() succeeds */
      CPU_FREE(cpuset);
      cpuset_ncpu *= 2;
      continue;
    }

    /* Unexpected error, but at least 1 CPU has to be available */
    CPU_FREE(cpuset);
    cpuset=NULL;
    cpuset_ncpu=0;
    cpuset_size=0;
    return 1;
  }
}

// Set this thread's CPU affinity to the Nth CPU in the list.
//
void set_working_cpu(int thread)
{
  int i;

  if(!cpuset_size)
    return;

  // The cpuset is already populated with the available CPUs on this system
  // from the call to get_num_cpus(). Look for the Nth one.

  for(i=0;;) {
    if(CPU_ISSET_S(i, cpuset_size, cpuset) && !thread--) {
      CPU_ZERO_S(cpuset_size, cpuset);
      CPU_SET_S(i, cpuset_size, cpuset);
      sched_setaffinity(0, cpuset_size, cpuset);  /* Ignore any errors */
      return;
    }

    if(++i >= cpuset_ncpu)
      i=0;  /* Wrap around */
  }
}
