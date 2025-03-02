#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif
// General note: pthread_t is unsigned long int
// cpu_set_t is a struct with an array, default size is fixed for all architecture
// pthread_t is like unsigned long

// __errno_location
// fork
// __fork
// __h_errno_location
// _IO_flockfile
// _IO_ftrylockfile
// _IO_funlockfile
// __libc_allocate_rtsig
// __libc_current_sigrtmax
// __libc_current_sigrtmin
GOM(pthread_atfork, iFEppp)
GOM(__pthread_atfork, iFEppp)
GOM(pthread_attr_destroy, iFEp)
// pthread_attr_getaffinity_np
#ifdef NOALIGN
GO(pthread_attr_getdetachstate, iFpp)
GO(pthread_attr_getguardsize, iFpp)
GO(pthread_attr_getinheritsched, iFpp)
GO(pthread_attr_getschedparam, iFpp)
GO(pthread_attr_getschedpolicy, iFpp)
GO(pthread_attr_getscope, iFpp)
#else
GOM(pthread_attr_getdetachstate, iFEpp)
GOM(pthread_attr_getguardsize, iFEpp)
GOM(pthread_attr_getinheritsched, iFEpp)
GOM(pthread_attr_getschedparam, iFEpp)
GOM(pthread_attr_getschedpolicy, iFEpp)
GOM(pthread_attr_getscope, iFEpp)
#endif
GOM(pthread_attr_getstack, iFEppp)
#ifdef NOALIGN
GO(pthread_attr_getstackaddr, iFpp)
GO(pthread_attr_getstacksize, iFpp)
GO(pthread_attr_init, iFp)
GO(pthread_attr_setaffinity_np, iFpLp)
GO(pthread_attr_setdetachstate, iFpi)
GO(pthread_attr_setguardsize, iFpL)
GO(pthread_attr_setinheritsched, iFpi)
GO(pthread_attr_setschedparam, iFpp)
GO(pthread_attr_setschedpolicy, iFpi)
GO(pthread_attr_setscope, iFpi)
GO(pthread_attr_setstackaddr, iFpp)
#else
GOM(pthread_attr_getstackaddr, iFEpp)
GOM(pthread_attr_getstacksize, iFEpp)
GOM(pthread_attr_init, iFEp)
GOM(pthread_attr_setaffinity_np, iFEpLp)
GOM(pthread_attr_setdetachstate, iFEpi)
GOM(pthread_attr_setguardsize, iFEpL)
GOM(pthread_attr_setinheritsched, iFEpi)
GOM(pthread_attr_setschedparam, iFEpp)
GOM(pthread_attr_setschedpolicy, iFEpi)
GOM(pthread_attr_setscope, iFEpi)
GOM(pthread_attr_setstackaddr, iFEpp)
#endif
GOM(pthread_attr_setstack, iFEppL)
GOM(pthread_attr_setstacksize, iFEpL)
#ifdef NOALIGN
GO(pthread_barrierattr_destroy, iFp)
GO(pthread_barrierattr_getpshared, iFpp)
GO(pthread_barrierattr_init, iFp)
GO(pthread_barrierattr_setpshared, iFpi)
#else
GOM(pthread_barrierattr_destroy, iFEp)
GOM(pthread_barrierattr_getpshared, iFEpp)
GOM(pthread_barrierattr_init, iFEp)
GOM(pthread_barrierattr_setpshared, iFEpi)
#endif
GO(pthread_barrier_destroy, iFp)
#ifdef NOALIGN
GO(pthread_barrier_init, iFppu)
#else
GOM(pthread_barrier_init, iFEppu)
#endif
GO(pthread_barrier_wait, iFp)
GO(pthread_cancel, iFL)
GOM(_pthread_cleanup_pop, vFEpi)
GOM(_pthread_cleanup_pop_restore, vFEpi)
GOM(_pthread_cleanup_push, vFEppp)
GOM(_pthread_cleanup_push_defer, vFEppp)
// __pthread_cleanup_routine
#ifdef NOALIGN
GO(pthread_condattr_destroy, iFp)
GO(pthread_condattr_getclock, iFpp)
GO(pthread_condattr_getpshared, iFpp)
GO(pthread_condattr_init, iFp)
GO(pthread_condattr_setclock, iFpi)
GO(pthread_condattr_setpshared, iFpi)
#else
GOM(pthread_condattr_destroy, iFEp)
GOM(pthread_condattr_getclock, iFEpp)
GOM(pthread_condattr_getpshared, iFEpp)
GOM(pthread_condattr_init, iFEp)
GOM(pthread_condattr_setclock, iFEpi)
GOM(pthread_condattr_setpshared, iFEpi)
#endif
GO(pthread_cond_broadcast, iFp)
GO(pthread_cond_destroy, iFp)
#ifdef NOALIGN
GO(pthread_cond_init, iFpp)
#else
GOM(pthread_cond_init, iFEpp)
#endif
GO(pthread_cond_signal, iFp)
GOM(pthread_cond_timedwait, iFEppp)
GOM(pthread_cond_wait, iFEpp)
GOM(pthread_create, iFEpppp)
GO(pthread_detach, iFL)
GO(pthread_equal, iFLL)
//GOM(pthread_exit, vFEp)
GO(pthread_exit, vFp)
GOM(pthread_getaffinity_np, iFEpLp)
GO(pthread_getattr_np, iFLp)
//GO(pthread_getconcurrency, iFv)
//GO(pthread_getcpuclockid, iFup)
GO(pthread_getschedparam, iFLpp)
GO(__pthread_getspecific, pFL)
GO(pthread_getspecific, pFL)
GO(pthread_getname_np, iFppL)
// __pthread_initialize_minimal
GO(pthread_join, iFLp)
GOM(__pthread_key_create, iFEpp)
GOM(pthread_key_create, iFEpp)
GO(pthread_key_delete, iFL)
GOM(pthread_kill, iFEpi)
// pthread_kill_other_threads_np
#ifdef NOALIGN
GO(__pthread_mutexattr_destroy, iFp)
GO(pthread_mutexattr_destroy, iFp)
GO(pthread_mutexattr_getkind_np, iFpp)
// pthread_mutexattr_getprioceiling
GO(pthread_mutexattr_getprotocol, iFpp)
// pthread_mutexattr_getpshared
// pthread_mutexattr_getrobust_np
GO(pthread_mutexattr_gettype, iFpp)
GO(__pthread_mutexattr_init, iFp)
GO(pthread_mutexattr_init, iFp)
GO(pthread_mutexattr_setkind_np, iFpi)
// pthread_mutexattr_setprioceiling
GO(pthread_mutexattr_setprotocol, iFpi)
GO(pthread_mutexattr_setpshared, iFpi)
// pthread_mutexattr_setrobust_np
GO(__pthread_mutexattr_settype, iFpi)
GO(pthread_mutexattr_settype, iFpi)
#else
GOM(__pthread_mutexattr_destroy, iFEp)
GOM(pthread_mutexattr_destroy, iFEp)
GOM(pthread_mutexattr_getkind_np, iFEpp)
// pthread_mutexattr_getprioceiling
GOM(pthread_mutexattr_getprotocol, iFEpp)
// pthread_mutexattr_getpshared
// pthread_mutexattr_getrobust_np
GOM(pthread_mutexattr_gettype, iFEpp)
GOM(__pthread_mutexattr_init, iFEp)
GOM(pthread_mutexattr_init, iFEp)
GOM(pthread_mutexattr_setkind_np, iFEpi)
// pthread_mutexattr_setprioceiling
GOM(pthread_mutexattr_setprotocol, iFEpi)
GOM(pthread_mutexattr_setpshared, iFEpi)
// pthread_mutexattr_setrobust_np
GOM(__pthread_mutexattr_settype, iFEpi)
GOM(pthread_mutexattr_settype, iFEpi)
#endif
// pthread_mutex_consistent_np
#ifdef NOALIGN
GO(__pthread_mutex_destroy, iFp)
GO(pthread_mutex_destroy, iFp)
// pthread_mutex_getprioceiling
GO(__pthread_mutex_init, iFpp)
GO(pthread_mutex_init, iFpp)
GO(__pthread_mutex_lock, iFp)
GO(pthread_mutex_lock, iFp)
// pthread_mutex_setprioceiling
GO(pthread_mutex_timedlock, iFpp)
GO(__pthread_mutex_trylock, iFp)
GO(pthread_mutex_trylock, iFp)
GO(__pthread_mutex_unlock, iFp)
GO(pthread_mutex_unlock, iFp)
#else
// phtread_mutex_t is 40 bytes on x86_64, but 48bytes on ARM64
GOM(__pthread_mutex_destroy, iFp)
GOM(pthread_mutex_destroy, iFp)
// pthread_mutex_getprioceiling
GOM(__pthread_mutex_init, iFpp)
GOM(pthread_mutex_init, iFpp)
GOM(__pthread_mutex_lock, iFp)
GOM(pthread_mutex_lock, iFp)
// pthread_mutex_setprioceiling
GOM(pthread_mutex_timedlock, iFpp)
GOM(__pthread_mutex_trylock, iFp)
GOM(pthread_mutex_trylock, iFp)
GOM(__pthread_mutex_unlock, iFp)
GOM(pthread_mutex_unlock, iFp)
#endif
GOM(pthread_once, iFEpp)
GOM(__pthread_once, iFEpp)
GOM(__pthread_register_cancel, vFEp)
// __pthread_register_cancel_defer
GO(pthread_rwlockattr_destroy, vFp)
GO(pthread_rwlockattr_getkind_np, iFpp)
// pthread_rwlockattr_getpshared
GO(pthread_rwlockattr_init, iFp)
GO(pthread_rwlockattr_setkind_np, iFpi)
// pthread_rwlockattr_setpshared
// __pthread_rwlock_destroy
GO(pthread_rwlock_destroy, iFp)
GO(__pthread_rwlock_init, iFpp)
GO(pthread_rwlock_init, iFpp)
GO(__pthread_rwlock_rdlock, iFp)
GO(pthread_rwlock_rdlock, iFp)
// pthread_rwlock_timedrdlock
// pthread_rwlock_timedwrlock
// __pthread_rwlock_tryrdlock
GO(pthread_rwlock_tryrdlock, iFp)
// __pthread_rwlock_trywrlock
GO(pthread_rwlock_trywrlock, iFp)
GO(__pthread_rwlock_unlock, iFp)
GO(pthread_rwlock_unlock, iFp)
GO(__pthread_rwlock_wrlock, iFp)
GO(pthread_rwlock_wrlock, iFp)
GO(pthread_self, LFv)
GOM(pthread_setaffinity_np, iFEpLp)
GO(pthread_setcancelstate, iFip)
GO(pthread_setcanceltype, iFip)
GO(pthread_setconcurrency, iFi)
GO(pthread_setname_np, iFpp)
GO(pthread_setschedparam, iFLip)
GO(pthread_setschedprio, iFpi)
GO(__pthread_setspecific, iFLp)
GO(pthread_setspecific, iFLp)
GO(pthread_sigmask, iFipp)
GO(pthread_spin_destroy, iFp)
GO(pthread_spin_init, iFpi)
GO(pthread_spin_lock, iFp)
GO(pthread_spin_trylock, iFp)
GO(pthread_spin_unlock, iFp)
GO(pthread_testcancel, vFv)
GO(pthread_timedjoin_np, iFppp)
GO(pthread_tryjoin_np, iFpp)
GOM(__pthread_unregister_cancel, vFEp)
// __pthread_unregister_cancel_restore
// __pthread_unwind
GOM(__pthread_unwind_next, vFEp)
GO(pthread_yield, iFv)
// raise
// __res_state
GO(sem_close, iFp)
GO(sem_destroy, iFp)
GO(sem_getvalue, iFpp)
GO(sem_init, iFpiu)
GO(sem_open, pFpOM)
GO(sem_post, iFp)
GO(sem_timedwait, iFpp)
GO(sem_trywait, iFp)
GO(sem_unlink, iFp)
GO(sem_wait, iFp)
// __sigaction
// system
// __vfork
