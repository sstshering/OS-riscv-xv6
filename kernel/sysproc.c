#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(n > 0){
    myproc()->sz = myproc()->sz + n;
  }else{
    uint sz = myproc()->sz;
    myproc()->sz = uvmdealloc(myproc()->pagetable, sz, sz+n);
  }
  /*
  if(growproc(n) < 0)
    return -1;
  */

  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// return the number of active processes in the system
// fill in user-provided data structure with pid,state,sz,ppid,name
uint64
sys_getprocs(void)
{
  uint64 addr;  // user pointer to struct pstat

  if (argaddr(0, &addr) < 0)
    return -1;
  return(procinfo(addr));
}

uint64
sys_wait2(void)     // Task 3 requirement
{ 
  uint64 p1;
  uint64 p2;
  if(argaddr(0, &p1) < 0 || argaddr(1, &p2) < 0)
    return -1;

  return wait2(p1, p2); // status
}

// add get and set priorty, set: getting the priorty and getting given priority in a0 like argint() int num = p->trapframe->a0
uint64
sys_getpriority(void)
{
  return getpriority(myproc()->priority);
}

uint64
sys_setpriority(void)
{
  int pr = sys_getpriority();
  struct proc *p = myproc();
  p->trapframe->a0 = pr;
  
  return setpriority(p->trapframe->a0);
}

uint64
sys_freepmem(void){
 return (nfreepages() * PGSIZE);
}

uint64
sys_sem_init(void){

  struct proc *p = myproc();

  uint64 addr;
  int start;
  int pshared;
  int index;

  if(argaddr(0, &addr) < 0){
    return -1;
  }

  if(argint(1, &pshared) < 0){
    return -1;
  }

  if(pshared == 0){
    return -1;
  }

  if(argint(2, &start) < 0){
    return -1;
  }

  index = semalloc();
  semtable.sem[index].count = start;

  if(copyout(p->pagetable,addr,(char*) &index, sizeof(index)) < 0){
    return -1;
  }

  return 0;
  
}

uint64
sys_sem_destroy(void){

  struct proc *p = myproc();

  uint64 addr;
  int index;

  if(argaddr(0,&addr) < 0){
    return -1;
  }

  acquire(&semtable.lock);

  if(copyin(p->pagetable,(char*) &index, addr, sizeof(int)) < 0){
    release(&semtable.lock);
    return 0;
  }

  semadealloc(index);
  release(&semtable.lock);
  return 0;
  
}

uint64
sys_sem_wait(void){

  struct proc *p = myproc();
  uint64 addr;
  int index;

  if(argaddr(0,&addr) < 0){
    return -1;
  }

  copyin(p->pagetable,(char*) &index, addr, sizeof(int));
  acquire(&semtable.sem[index].lock);

  if(semtable.sem[index].count > 0){

    semtable.sem[index].count--;
    release(&semtable.sem[index].lock);
    return 0;
  }else{

    while(semtable.sem[index].count == 0){
      
      sleep((void*)&semtable.sem[index], &semtable.sem[index].lock);
    }

    semtable.sem[index].count -= 1;
    release(&semtable.sem[index].lock);
  }

  return 0;
  
}

uint64
sys_sem_post(void){

  struct proc *p = myproc();
  uint64 addr;
  int index;

  if(argaddr(0, &addr) < 0){
    return -1;
  }

  copyin(p->pagetable,(char *)&index, addr, sizeof(int));
  acquire(&semtable.sem[index].lock);

  semtable.sem[index].count += 1;
  wakeup((void*)&semtable.sem[index]);

  release(&semtable.sem[index].lock);

  return 0;
  
}