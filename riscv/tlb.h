// simulate the hardware TLB in actual hardware processors

#ifndef _RISCV_TLB_H
#define _RISCV_TLB_H

#include <map>
#include <vector>
#include <list>

struct mem_access_info_t; 

struct HardTLBEntry {
  bool     va;
  uint64_t ppn;
  uint64_t pte;
  HardTLBEntry(): va(false), ppn(0), pte(0) {}
  HardTLBEntry(bool va, uint64_t ppn, uint64_t pte): va(va), ppn(ppn), pte(pte) {}
};

struct WalkRecord {
  int levels;
  uint64_t vpn;
  uint64_t ppn;
  uint64_t pte;
  uint64_t ptes[6]; // maximally walk 6 page table entries
};

class HardTLBBase
{
protected:
  const int core;
  std::vector<std::map<uint64_t, HardTLBEntry> > entries;
  std::vector<std::list<uint64_t> > order;
  uint32_t nset, nway;  // number of sets and ways
  mmu_t *mmu;           // the pe core MMU

public:
  uint64_t access_n, miss_n, hit_n, walk_hit_n; // pfcs

  HardTLBBase(int core, mmu_t *mmu, uint32_t nway, uint32_t nset = 1)
    : core(core), nset(nset), nway(nway), mmu(mmu),
      access_n(0), miss_n(0), hit_n(0), walk_hit_n(0)
  {
    entries.resize(nset);
    order.resize(nset);
  }
  HardTLBEntry translate(uint64_t vpn, mem_access_info_t access_info);
  void refill(uint64_t vpn, uint64_t ppn, uint64_t pte);
  void shootdown(uint64_t vpn);
  void flush();
  uint64_t translate_simple(uint64_t vaddr); // a simple translator used by pfc interface
};

#endif
