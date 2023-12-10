#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "psched.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  struct proc* p = myproc();
  p->sleep_tick = n;
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

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// TODO: set nice value and return
int sys_nice(void){
  int n, curr_n;

  if(argint(0, &n) < 0)
    return -1;
  
  acquire(&tickslock);
  if(n < 0 || n > 20){
    release(&tickslock);
    return -1;
  }
  curr_n = nice(n);
  release(&tickslock);
  return curr_n;
}

int sys_getschedstate(void){
  struct pschedinfo *user_info;

  // Fetch the user's pointer
  if(argptr(0, (void*)&user_info, sizeof(*user_info)) < 0)
    return -1;

  acquire(&tickslock);
  // Call the function to fill in the info
  if(getschedstate(user_info) < 0){
    release(&tickslock);
    return -1;
  }

  release(&tickslock);
  return 0;
}