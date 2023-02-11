#include "boot.h"

// DO NOT DEFINE ANY NON-LOCAL VARIBLE!

void load_kernel() {
  char hello[] = {'\n', 'h', 'e', 'l', 'l', 'o', '\n', 0};
  putstr(hello);
  while (1) ;
  // remove both lines above before write codes below
  Elf32_Ehdr *elf = (void *)0x8000;
  copy_from_disk(elf, 255 * SECTSIZE, SECTSIZE);
  Elf32_Phdr *ph, *eph;
  ph = (void*)((uint32_t)elf + elf->e_phoff);
  eph = ph + elf->e_phnum;
  for (; ph < eph; ph++) {
    if (ph->p_type == PT_LOAD) {
      // TODO: Lab1-2, Load kernel and jump
    }
  }
  uint32_t entry = 114514; // change me
  ((void(*)())entry)();
}
