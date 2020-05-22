.global _ZN3bio2os26GetThreadLocalStorageValueEv
_ZN3bio2os26GetThreadLocalStorageValueEv:
    mrs x0, tpidrro_el0
    ret