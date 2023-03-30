#include "boot.h"

// DO NOT DEFINE ANY NON-LOCAL VARIBLE!

void load_kernel() {
  
  Elf32_Ehdr *elf = (void *)0x8000;
  copy_from_disk(elf, 255 * SECTSIZE, SECTSIZE);
  Elf32_Phdr *ph, *eph;
  ph = (void*)((uint32_t)elf + elf->e_phoff);
  eph = ph + elf->e_phnum;
  for (; ph < eph; ph++) {
    if (ph->p_type == PT_LOAD) {
      memcpy((void*)ph->p_vaddr,(void*)(ph->p_offset+(uint32_t)elf),ph->p_filesz);
		  memset((void*)(ph->p_vaddr+ph->p_filesz),0,ph->p_memsz-ph->p_filesz);
      // TODO: Lab1-2, Load kernel and jump
    }
  }
  uint32_t entry = elf->e_entry; // change me
  ((void(*)())entry)();
}
