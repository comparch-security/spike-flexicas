#include "mmu.h"

void HardTLBBase::flush() {
  for(unsigned int i = 0; i < nset; i++) {
    entries[i].clear();
    order[i].clear();
  }
  if(outer) outer->flush();
}

HardTLBEntry * HardTLBBase::hit(uint64_t vpn) {
  uint32_t idx = vpn % nset;
  access_n++;
  if(entries[idx].count(vpn)) {
    hit_n++;
    order[idx].remove(vpn);
    order[idx].push_back(vpn);
    return &(entries[idx][vpn]);
  }

  miss_n++;

  if(outer) {
    auto entry = outer->hit(vpn);
    if(entry)
      return refill(entry->vpn, entry->ppn, entry->pte);
    else
      return nullptr;
  }

  return nullptr;
}

HardTLBEntry HardTLBBase::translate(uint64_t vpn, mem_access_info_t access_info) {
  auto entry = hit(vpn);
  if(entry) return *entry;

  // miss, need walk the page table
  if(mmu->tlb_walk_record.levels == 0 || mmu->tlb_walk_record.vpn != vpn)
    mmu->walk(access_info);
  else
    walk_hit_n++;

  // emulate the L1 cache access
  for(int i = 0; i < mmu->tlb_walk_record.levels; i++)
    flexicas::read(mmu->tlb_walk_record.ptes[i], core);

  // refill
  if(mmu->tlb_walk_record.levels != 0) // not a physical address
    return *(refill(vpn, mmu->tlb_walk_record.ppn, mmu->tlb_walk_record.pte));
  else // physical address and TLB is bypassed
    return HardTLBEntry(false, vpn, vpn, 0);
}

HardTLBEntry * HardTLBBase::refill(uint64_t vpn, uint64_t ppn, uint64_t pte) {
  uint32_t idx = vpn % nset;
  if(!entries[idx].count(vpn) && entries[idx].size() == nway) {
    // replace
    auto vpn_replace = order[idx].front();
    order[idx].pop_front();

    // writeback
    if(outer) {
      auto entry_replace = &(entries[idx][vpn_replace]);
      outer->refill(entry_replace->vpn, entry_replace->ppn, entry_replace->pte);
    }
    entries[idx].erase(vpn_replace);
  }

  // update replace order
  if(entries[idx].count(vpn)) order[idx].remove(vpn);
  order[idx].push_back(vpn);

  auto entry = &(entries[idx][vpn]);
  entry->va  = true;
  entry->vpn = vpn;
  entry->ppn = ppn;
  entry->pte = pte;
  return entry;
}

void HardTLBBase::shootdown(uint64_t vpn) {
  uint32_t idx = vpn % nset;
  if(entries[idx].count(vpn)) {
    entries[idx].erase(vpn);
    order[idx].remove(vpn);
  }
  if(outer) outer->shootdown(vpn);
}

uint64_t HardTLBBase::translate_simple(uint64_t vaddr) {
  auto tr = translate(vaddr >> PGSHIFT, mmu->default_access_info(vaddr));
  return (tr.ppn << PGSHIFT) | (vaddr & ((1ull << PGSHIFT) - 1));
}
