#include "mmu.h"

void HardTLBBase::flush() {
  for(unsigned int i = 0; i < nset; i++) {
    entries[i].clear();
    order[i].clear();
  }
}

HardTLBEntry HardTLBBase::translate(uint64_t vpn, mem_access_info_t access_info) {
  uint32_t idx = vpn % nset;
  if(!entries[idx].count(vpn)) {
    // walk
    if(mmu->tlb_walk_record.levels == 0 || mmu->tlb_walk_record.vpn != vpn)
      mmu->walk(access_info);
    else
      walk_hit_n++;

    // emulate the L1 cache access
    for(int i = 0; i < mmu->tlb_walk_record.levels; i++)
      flexicas::read(mmu->tlb_walk_record.ptes[i], core);

    // refill
    if(mmu->tlb_walk_record.levels != 0) { // not a physical address
      miss_n++;
      refill(vpn, mmu->tlb_walk_record.ppn, mmu->tlb_walk_record.pte);
    } else { // physical address
      // TLB is bypassed
      return HardTLBEntry(false, vpn, 0);
    }
  } else { // hit
    hit_n++;
    order[idx].remove(vpn);
    order[idx].push_back(vpn);
  }
  access_n++;
  return entries[idx][vpn];
}

void HardTLBBase::refill(uint64_t vpn, uint64_t ppn, uint64_t pte) {
  uint32_t idx = vpn % nset;
  if(!entries[idx].count(vpn) && entries[idx].size() == nway) {
    uint64_t vpn_rp = order[idx].front();
    order[idx].pop_front();
    entries[idx].erase(vpn_rp);
    order[idx].push_back(vpn);
    entries[idx][vpn].va  = true;
    entries[idx][vpn].ppn = ppn;
    entries[idx][vpn].pte = pte;
  } else {
    if(entries[idx].count(vpn)) order[idx].remove(vpn);
    order[idx].push_back(vpn);
    entries[idx][vpn].va  = true;
    entries[idx][vpn].ppn = ppn;
    entries[idx][vpn].pte = pte;
  }
}

void HardTLBBase::shootdown(uint64_t vpn) {
  uint32_t idx = vpn % nset;
  if(entries[idx].count(vpn)) {
    entries[idx].erase(vpn);
    order[idx].remove(vpn);
  }
}
