#ifndef CM_SPIKE_FLEXICAS_HPP
#define CM_SPIKE_FLEXICAS_HPP

// the header file expose to spike

#include "csrs.h"

namespace flexicas {
  static const unsigned int CSR_FLAXICAS_PFC = 0x8F0;
  extern int  ncore();
  extern int  cache_level();
  extern int  cache_set(int level, bool ic);
  extern int  cache_way(int level, bool ic);
  extern void init(int ncore, const char *pfc_log_prefix);
  extern void read(uint64_t addr, int core, bool ic = false);
  extern void write(uint64_t addr, int core);
  extern void flush(uint64_t addr, int core);
  extern void writeback(uint64_t addr, int core);
  extern void exit();
  extern void csr_write(uint64_t cmd, int core);
  extern uint64_t csr_read(int core);
  extern void bump_cycle(int step, int core);
}

class flexicas_csr_t: public csr_t {
  int core_index;
public:
  flexicas_csr_t(processor_t* const proc, int core)
    : csr_t(proc, flexicas::CSR_FLAXICAS_PFC), core_index(core) {}

  virtual reg_t read() const noexcept override {
    return flexicas::csr_read(core_index);
  }

protected:
  virtual bool unlogged_write(const reg_t val) noexcept override {
    flexicas::csr_write(val, core_index);
    return true;
  }
};

#endif
