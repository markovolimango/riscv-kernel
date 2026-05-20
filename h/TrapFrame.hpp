struct TrapFrame {
    uint64 x[32];
    uint64 sepc;
    uint64 sstatus;
    uint64 scause;
};