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
  extern void init(int ncore);
  extern void read(uint64_t addr, int core, bool ic = false);
  extern void write(uint64_t addr, int core);
  extern void flush(uint64_t addr, int core);
  extern void writeback(uint64_t addr, int core);
  extern void exit();
  extern void csr_write(uint64_t cmd);
  extern uint64_t csr_read();
}

class flexicas_csr_t: public csr_t {
public:
  flexicas_csr_t(processor_t* const proc)
    : csr_t(proc, flexicas::CSR_FLAXICAS_PFC) {}

  virtual reg_t read() const noexcept override {
    return flexicas::csr_read();
  }

protected:
  virtual bool unlogged_write(const reg_t val) noexcept override {
    flexicas::csr_write(val);
    return true;
  }
};

#endif
