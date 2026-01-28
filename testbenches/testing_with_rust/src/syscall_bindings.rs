use libc::{syscall, pid_t, sched_param, sched_setscheduler, fork, getpid};

const SCHED_HVF: i32 = 8;
const __NR_SET_SCHED_PARS: i64 = 467;
const __NR_GET_SCHED_PARS: i64 = 468;
const __NR_GET_SCHED_SCORE: i64 = 469;

#[repr(C)]
struct d_params {
        deadline1: i64,
        deadline2: i64,
        computation_time: i64
}


pub fn set_sched_params(x: i64, y: i64, z: i64) -> Result<(), std::io::Error> {
        let ret: i64 = unsafe {
                syscall(__NR_SET_SCHED_PARS, x, y, z)
        };
        if ret < 0 {
                return Err(std::io::Error::last_os_error());
        }
        else {
                return Ok(());
        }
}

pub fn get_sched_params(d: &mut d_params) -> Result<(), std::io::Error> {
        let ret: i64 = unsafe {
                syscall(__NR_GET_SCHED_PARS, d as *const d_params)
        };

        if ret < 0 {
                return Err(std::io::Error::last_os_error());
        }
        else {
                return Ok(());
        }
}
pub fn get_sched_score() -> Result<i64, std::io::Error>{
        let ret: i64 = unsafe {
                syscall(__NR_GET_SCHED_SCORE)
        };

        if ret < 0 {
                return Err(std::io::Error::last_os_error());
        }
        else {
                return Ok(ret);
        }
}

pub fn set_hvf_scheduler(pid: pid_t) {
        let param = sched_param{sched_priority: 0};
        // safety note calling syscall to change scheduler for this process.
        unsafe {
                sched_setscheduler(pid, SCHED_HVF, &param);
        }
}

pub fn set_hvf_scheduler_current() {
        set_hvf_scheduler(0);
}

pub fn safe_fork() -> Result<pid_t, std::io::Error> {
        let pid: pid_t = unsafe { fork() };

        if pid < 0 {
                return Err(std::io::Error::last_os_error());
        }
        else {
                return Ok(pid);
        }
}

pub fn safe_getpid() -> Result<pid_t, std::io::Error> {
        let pid: pid_t = unsafe { getpid() };

        if pid < 0 {
                return Err(std::io::Error::last_os_error());
        }
        else {
                 return Ok(pid);
        }
}
