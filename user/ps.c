#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  struct pstat uproc[NPROC];
  int nprocs;
  int i;
  char *state;
  static char *states[] = {
    [SLEEPING]  "sleeping",
    [RUNNABLE]  "runnable",
    [RUNNING]   "running ",
    [ZOMBIE]    "zombie  "
  };

  int age;
  int result;

  nprocs = getprocs(uproc);
  if (nprocs < 0)
    exit(-1);
  
  // The "\t" tab function is not working correctly, just tabbing uneven spaces
  printf("pid\tstate\tage\tsize\tpriority\tcputime\tppid\tname\n");
  for (i=0; i<nprocs; i++) {
    state = states[uproc[i].state];
    result = strcmp(state, "runnable");
    if(result == 0){
      age = uptime() - uproc[i].readytime;

      printf("%d\t%s\t%d\t%d\t%d\t%d\t%d\t%s\n", uproc[i].pid, state, age,
                   uproc[i].size, uproc[i].priority, uproc[i].readytime , uproc[i].ppid, uproc[i].name);

    }
    
    printf("%d\t%s\t%d\t%d\t%d\t%d\t%s\n", uproc[i].pid, state,
                   uproc[i].size, uproc[i].priority, uproc[i].readytime , uproc[i].ppid, uproc[i].name);
  }

  exit(0);
}