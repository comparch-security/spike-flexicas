require_extension('S');
require_impl(IMPL_MMU);
if (STATE.v) {
  if (STATE.prv == PRV_U || get_field(STATE.hstatus->read(), HSTATUS_VTVM))
    require_novirt();
} else {
  require_privilege(get_field(STATE.mstatus->read(), MSTATUS_TVM) ? PRV_M : PRV_S);
}
MMU.flush_tlb(true);
// this is a simplified implementation ignoring rs1 and rs2
// See privileged spec 4.2.1:
//   Supervisor Memory-Management Fence Instruction
