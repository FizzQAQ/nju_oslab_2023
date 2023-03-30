#include "klib.h"
#include "cte.h"
#include "proc.h"

#define PROC_NUM 64

static __attribute__((used)) int next_pid = 1;

proc_t pcb[PROC_NUM];
static proc_t *curr = &pcb[0];

void init_proc() {
  pcb[0].status=RUNNING;
  pcb[0].pgdir=vm_curr();
  pcb[0].kstack=(void*)(KER_MEM-PGSIZE);
  
  sem_init(&pcb[0].zombie_sem,0);
  // Lab2-1, set status and pgdir
  // Lab2-4, init zombie_sem
  // Lab3-2, set cwd
}

proc_t *proc_alloc() {
  // Lab2-1: find a unused pcb from pcb[1..PROC_NUM-1], return NULL if no such one
  for(int i=1;i<PROC_NUM;i++){
    if(pcb[i].status==UNUSED){
      pcb[i].pid=next_pid;
      next_pid++;
      pcb[i].status=UNINIT;
      pcb[i].pgdir=vm_alloc();
      pcb[i].kstack=kalloc();
      pcb[i].brk=0;
      pcb[i].ctx= &pcb[i].kstack->ctx;
      pcb[i].parent=NULL;
      pcb[i].child_num=0;
      for(size_t j=0;j<MAX_USEM;j++){
        pcb[i].usems[j]=NULL;
      }
      sem_init(&pcb[i].zombie_sem,0);
      return &pcb[i];
    }
  }
  return NULL;
  // init ALL attributes of the pcb
}

void proc_free(proc_t *proc) {
  // Lab2-1: free proc's pgdir and kstack and mark it UNUSED
  proc->status=UNUSED;
  
}

proc_t *proc_curr() {
  return curr;
}

void proc_run(proc_t *proc) {
  proc->status = RUNNING;
  curr = proc;
  set_cr3(proc->pgdir);
  set_tss(KSEL(SEG_KDATA), (uint32_t)STACK_TOP(proc->kstack));
  irq_iret(proc->ctx);
}

void proc_addready(proc_t *proc) {
  // Lab2-1: mark proc READY
  proc->status = READY;
}

void proc_yield() {
  // Lab2-1: mark curr proc READY, then int $0x81
  curr->status = READY;
  INT(0x81);
}

void proc_copycurr(proc_t *proc) {
  vm_copycurr(proc->pgdir);
  proc->brk=curr->brk;
  *proc->ctx=curr->kstack->ctx;
  proc->ctx->eax=0;
  proc->parent=curr;
  for(size_t j=0;j<MAX_USEM;j++){
        proc->usems[j]=curr->usems[j];
      }
  curr->child_num++;
  // Lab2-2: copy curr proc
  // Lab2-5: dup opened usems
  // Lab3-1: dup opened files
  // Lab3-2: dup cwd
  //assert(0);
}

void proc_makezombie(proc_t *proc, int exitcode) {
  proc->status=ZOMBIE;
  proc->exit_code=exitcode;
  if(proc->parent!=NULL)
  sem_v(&proc->parent->zombie_sem);
  for(size_t j=0;j<MAX_USEM;j++)
  if(proc->usems[j]!=NULL)
  usem_close(proc->usems[j]);
  for(int i=0;i<PROC_NUM;i++)
  {
    if (pcb[i].parent==proc){
      pcb[i].parent=NULL;
    }
  }
  // Lab2-3: mark proc ZOMBIE and record exitcode, set children's parent to NULL
  // Lab2-5: close opened usem
  // Lab3-1: close opened files
  // Lab3-2: close cwd
  //assert(0);
}

proc_t *proc_findzombie(proc_t *proc) {
  // Lab2-3: find a ZOMBIE whose parent is proc, return NULL if none
  proc_t* ans=NULL;
  for(int i=0;i<PROC_NUM;i++)
  {
    if (pcb[i].parent==proc){
      if(pcb[i].status==ZOMBIE){
        ans=&pcb[i];
        break;
      }
    }
  }
  return ans;
}

void proc_block() {
  // Lab2-4: mark curr proc BLOCKED, then int $0x81
  curr->status = BLOCKED;
  INT(0x81);
}

int proc_allocusem(proc_t *proc) {
  for(size_t j=0;j<MAX_USEM;j++){
        if(proc->usems[j]==NULL){
          return j;
        }
      }
  return -1;
}

usem_t *proc_getusem(proc_t *proc, int sem_id) {
  if(sem_id>=MAX_USEM) return NULL;
  return proc->usems[sem_id];
}

int proc_allocfile(proc_t *proc) {
  // Lab3-1: find a free slot in proc->files, return its index, or -1 if none
  TODO();
}

file_t *proc_getfile(proc_t *proc, int fd) {
  // Lab3-1: return proc->files[fd], or NULL if fd out of bound
  TODO();
}

void schedule(Context *ctx) {
  // Lab2-1: save ctx to curr->ctx, then find a READY proc and run it
  curr->ctx=ctx;
  int curr_num=curr-pcb;
  for(int i=1;i<PROC_NUM+1;i++){
    if(pcb[(curr_num+i)%PROC_NUM].status==READY){
      proc_run(&pcb[(curr_num+i)%PROC_NUM]);
      }
  }
}
