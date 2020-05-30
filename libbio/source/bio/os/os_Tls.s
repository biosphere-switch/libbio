.global _ZN3bio2os21GetThreadLocalStorageEv
_ZN3bio2os21GetThreadLocalStorageEv:
    mrs x0, tpidrro_el0
    ret