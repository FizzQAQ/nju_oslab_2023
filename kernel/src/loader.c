#include "klib.h"
#include "vme.h"
#include "cte.h"
#include "loader.h"
#include "disk.h"
#include "fs.h"
#include <elf.h>

uint32_t load_elf(PD *pgdir, const char *name) {
  Elf32_Ehdr elf;
  Elf32_Phdr ph;
  inode_t *inode = iopen(name, TYPE_NONE);
  if (!inode) return -1;
  iread(inode, 0, &elf, sizeof(elf));
  if (*(uint32_t*)(&elf) != 0x464c457f) { // check ELF magic number
    iclose(inode);
    return -1;
  }
  for (int i = 0; i < elf.e_phnum; ++i) {
    iread(inode, elf.e_phoff + i * sizeof(ph), &ph, sizeof(ph));
    if (ph.p_type == PT_LOAD) {
      // Lab1-2: Load segment to physical memory
      // Lab1-4: Load segment to virtual memory
    PD* newpd=vm_curr();
    set_cr3(pgdir);
    vm_map(pgdir,ph.p_vaddr,ph.p_memsz,7);
    void* pa=vm_walk(pgdir,ph.p_vaddr,7); 
    iread(inode, ph.p_offset, (void*)pa,ph.p_filesz);
    memset((void*)(pa+ph.p_filesz),0,ph.p_memsz-ph.p_filesz);
    set_cr3(newpd);
    }
  }
  vm_map(pgdir,USR_MEM-PGSIZE,PGSIZE,7);
  // TODO: Lab1-4 alloc stack memory in pgdir
  iclose(inode);
  return elf.e_entry;
}

#define MAX_ARGS_NUM 31

uint32_t load_arg(PD *pgdir, char *const argv[]) {
  // Lab1-8: Load argv to user stack
  char *stack_top = (char*)vm_walk(pgdir, USR_MEM - PGSIZE, 7) + PGSIZE;
  size_t argv_va[MAX_ARGS_NUM + 1];
  int argc;
  for (argc = 0; argv[argc]; ++argc) {
    assert(argc < MAX_ARGS_NUM);
    // push the string of argv[argc] to stack, record its va to argv_va[argc]
    stack_top-=(strlen(argv[argc])+1);
    strcpy(stack_top,argv[argc]);
    argv_va[argc]=USR_MEM-PGSIZE+ADDR2OFF(stack_top);//TODO
  }
  argv_va[argc] = 0; // set last argv NULL
  stack_top -= ADDR2OFF(stack_top) % 4; // align to 4 bytes
  for (int i = argc; i >= 0; --i) {
    // push the address of argv_va[argc] to stack to make argv array
    stack_top -= sizeof(size_t);
    *(size_t*)stack_top = argv_va[i];
  }
 
  // push the address of the argv array as argument for _start
  size_t temp=(size_t)(USR_MEM-PGSIZE+ADDR2OFF(stack_top));
  stack_top -= sizeof(size_t);
  *(size_t*)stack_top = temp;//TODO
  // push argc as argument for _start
  stack_top -= sizeof(size_t);
  *(size_t*)stack_top = argc;
  stack_top -= sizeof(size_t); // a hole for return value (useless but necessary)
  //Log("%hx",(USR_MEM - PGSIZE + ADDR2OFF(stack_top)));
  return USR_MEM - PGSIZE + ADDR2OFF(stack_top);
}

int load_user(PD *pgdir, Context *ctx, const char *name, char *const argv[]) {
  size_t eip = load_elf(pgdir, name);
  if (eip == -1) return -1;
  ctx->cs = USEL(SEG_UCODE);
  ctx->ds = USEL(SEG_UDATA);
  ctx->eip = eip;
  ctx->ss= USEL(SEG_UDATA);
  // ctx->esp=USR_MEM-16;
  ctx->esp=load_arg(pgdir,argv);
  // TODO: Lab1-6 init ctx->ss and esp
  ctx->eflags = 0x202; // TODO: Lab1-7 change me to 0x202
  return 0;
}
