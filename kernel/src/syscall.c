#include "klib.h"
#include "cte.h"
#include "sysnum.h"
#include "vme.h"
#include "serial.h"
#include "loader.h"
#include "proc.h"
#include "timer.h"
#include "file.h"

typedef int (*syshandle_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

extern void *syscall_handle[NR_SYS];

void do_syscall(Context *ctx) {
  // TODO: Lab1-5 call specific syscall handle and set ctx register
  int sysnum = 0;
  uint32_t arg1 = 0;
  uint32_t arg2 = 0;
  uint32_t arg3 = 0;
  uint32_t arg4 = 0;
  uint32_t arg5 = 0;
  int res;
  sysnum=ctx->eax;
  arg1=ctx->ebx;
  arg2=ctx->ecx;
  arg3=ctx->edx;
  arg4=ctx->esi;
  arg5=ctx->edi;
  if (sysnum < 0 || sysnum >= NR_SYS) {
    res = -1;
  } else {
    res = ((syshandle_t)(syscall_handle[sysnum]))(arg1, arg2, arg3, arg4, arg5);
  }
  ctx->eax = res;
}

int sys_write(int fd, const void *buf, size_t count) {
  // TODO: rewrite me at Lab3-1
  return serial_write(buf, count);
}

int sys_read(int fd, void *buf, size_t count) {
  // TODO: rewrite me at Lab3-1
  return serial_read(buf, count);
}

int sys_brk(void *addr) {
  // TODO: Lab1-5
  size_t brk = proc_curr()->brk; // use brk of proc instead of this in Lab2-1
  size_t new_brk = PAGE_UP(addr);
  if (brk == 0) {
    brk = new_brk;
  } else if (new_brk > brk) {
    vm_map(vm_curr(),brk,new_brk-brk,7);
    brk=new_brk;
  } else if (new_brk < brk) {
    // can just do nothing
  }
  return 0;
}

void sys_sleep(int ticks) {
  int pre_tick=get_tick(); // Lab1-7
  while(1){
  int now_tick=get_tick();
  //sti(); hlt(); cli();
  proc_yield();
  if(now_tick-pre_tick>=ticks)
    break;
  } 
}

int sys_exec(const char *path, char *const argv[]) {

  PD *pd=vm_alloc();
  Context  ctx;
  if(load_user(pd,&ctx,path,argv)==-1){
    kfree(pd);
    return -1;
  }
  else{
    PD *pre=vm_curr();
    set_cr3(pd);
    proc_curr()->pgdir=pd;
    kfree(pre);
    irq_iret(&ctx);
}


}

int sys_getpid() {
  return  proc_curr()->pid; // Lab2-1
}

void sys_yield() {
  proc_yield();
}

int sys_fork() {
  proc_t * pcb=proc_alloc();
  if(pcb==NULL){
    return -1;
  }
  proc_copycurr(pcb);
  proc_addready(pcb);
  return pcb->pid;
}

void sys_exit(int status) {
  proc_makezombie(proc_curr(),status);
  INT(0x81);
  assert(0);
}

int sys_wait(int *status) {
  if(!proc_curr()->child_num){
    return -1;
  }
  sem_p(&proc_curr()->zombie_sem);
  proc_t* ans=proc_findzombie(proc_curr());
  
  
  if(status!=NULL){
    *status=ans->exit_code;
  }
    int pi=ans->pid;
    proc_free(ans);
    proc_curr()->child_num--;
    return pi;
  
}

int sys_sem_open(int value) {
  int index=proc_allocusem(proc_curr());
  if(index==-1){
    return -1;
  }
  usem_t* temp=usem_alloc(value);
  if(temp==NULL){
    return -1;
  }
  proc_curr()->usems[index]=temp;
  return index;
}

int sys_sem_p(int sem_id) {
  usem_t* temp=proc_getusem(proc_curr(),sem_id);
  if(temp==NULL)
  {
    return -1;
  }
  sem_p(&temp->sem);
  return 0;
}

int sys_sem_v(int sem_id) {
  usem_t* temp=proc_getusem(proc_curr(),sem_id);
  if(temp==NULL)
  {
    return -1;
  }
  sem_v(&temp->sem);
  return 0;
}

int sys_sem_close(int sem_id) {
  usem_t* temp=proc_getusem(proc_curr(),sem_id);
  if(temp==NULL)
  {
    return -1;
  }
  usem_close(temp);
  proc_curr()->usems[sem_id]=NULL;
  return 0;
}

int sys_open(const char *path, int mode) {
  TODO(); // Lab3-1
}

int sys_close(int fd) {
  TODO(); // Lab3-1
}

int sys_dup(int fd) {
  TODO(); // Lab3-1
}

uint32_t sys_lseek(int fd, uint32_t off, int whence) {
  TODO(); // Lab3-1
}

int sys_fstat(int fd, struct stat *st) {
  TODO(); // Lab3-1
}

int sys_chdir(const char *path) {
  TODO(); // Lab3-2
}

int sys_unlink(const char *path) {
  return iremove(path);
}

// optional syscall

void *sys_mmap() {
  TODO();
}

void sys_munmap(void *addr) {
  TODO();
}

int sys_clone(void (*entry)(void*), void *stack, void *arg) {
  TODO();
}

int sys_kill(int pid) {
  TODO();
}

int sys_cv_open() {
  TODO();
}

int sys_cv_wait(int cv_id, int sem_id) {
  TODO();
}

int sys_cv_sig(int cv_id) {
  TODO();
}

int sys_cv_sigall(int cv_id) {
  TODO();
}

int sys_cv_close(int cv_id) {
  TODO();
}

int sys_pipe(int fd[2]) {
  TODO();
}

int sys_link(const char *oldpath, const char *newpath) {
  TODO();
}

int sys_symlink(const char *oldpath, const char *newpath) {
  TODO();
}

void *syscall_handle[NR_SYS] = {
  [SYS_write] = sys_write,
  [SYS_read] = sys_read,
  [SYS_brk] = sys_brk,
  [SYS_sleep] = sys_sleep,
  [SYS_exec] = sys_exec,
  [SYS_getpid] = sys_getpid,
  [SYS_yield] = sys_yield,
  [SYS_fork] = sys_fork,
  [SYS_exit] = sys_exit,
  [SYS_wait] = sys_wait,
  [SYS_sem_open] = sys_sem_open,
  [SYS_sem_p] = sys_sem_p,
  [SYS_sem_v] = sys_sem_v,
  [SYS_sem_close] = sys_sem_close,
  [SYS_open] = sys_open,
  [SYS_close] = sys_close,
  [SYS_dup] = sys_dup,
  [SYS_lseek] = sys_lseek,
  [SYS_fstat] = sys_fstat,
  [SYS_chdir] = sys_chdir,
  [SYS_unlink] = sys_unlink,
  [SYS_mmap] = sys_mmap,
  [SYS_munmap] = sys_munmap,
  [SYS_clone] = sys_clone,
  [SYS_kill] = sys_kill,
  [SYS_cv_open] = sys_cv_open,
  [SYS_cv_wait] = sys_cv_wait,
  [SYS_cv_sig] = sys_cv_sig,
  [SYS_cv_sigall] = sys_cv_sigall,
  [SYS_cv_close] = sys_cv_close,
  [SYS_pipe] = sys_pipe,
  [SYS_link] = sys_link,
  [SYS_symlink] = sys_symlink};