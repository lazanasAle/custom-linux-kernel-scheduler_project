use std::{os::unix::raw::pid_t, process::exit, ptr::null, time::{SystemTime, UNIX_EPOCH}};

use libc::waitpid;

use crate::syscall_bindings::{safe_fork, safe_getpid, set_hvf_scheduler_current, set_sched_params};

mod syscall_bindings;

const DELAY: u32 = 750;
const K: u32 = 1000;
const H13: u32 = 1300;

fn core_delay() {
        let mut a: f64 = 1.1;
        for j in 0..100000 {
                a += f64::sqrt(1.1)*f64::sqrt(1.2)*f64::sqrt(1.3)*f64::sqrt(1.4)*f64::sqrt(1.5);
                a += f64::sqrt(1.6)*f64::sqrt(1.7)*f64::sqrt(1.8)*f64::sqrt(1.9)*f64::sqrt(2.0);
                a += f64::sqrt(1.1)*f64::sqrt(1.2)*f64::sqrt(1.3)*f64::sqrt(1.4)*f64::sqrt(1.5);
                a += f64::sqrt(1.6)*f64::sqrt(1.7)*f64::sqrt(1.8)*f64::sqrt(1.9);
        }
}

fn delay(workload: i32) {
        let total_workload: i32 = workload*(DELAY as i32);
        for i in 0..total_workload {
                core_delay();
        }
}

fn do_work(workload: i32) {
        let my_pid: pid_t = match safe_getpid() {
                Ok(pid) => pid,
                Err(err) => {
                        eprintln!("error-occured-on getting pid");
                        return;
                }
        };
        println!("Process {} begins", my_pid);
        delay(workload);
        println!("Process {} ends", my_pid);
}


fn main() {
        let mut procs: [pid_t; 20] = [0; 20];

        for j in 0..20 {
                procs[j] = match safe_fork() {
                        Ok(pid) => pid,
                        Err(err) => {
                                eprintln!("error on fork");
                                return;
                        }
                };
                if procs[j] == 0 {
                        let now = SystemTime::now()
                                .duration_since(UNIX_EPOCH).expect("Time went backwards");
                        let now_secs: i64 = now.as_secs() as i64;
                        let sched_par_stat = match set_sched_params(now_secs+(j as i64)+1,
                                now_secs+(j as i64)+7, (K+H13*(j as u32)) as i64) {
                                        Ok(()) => (),
                                        Err(err) => {
                                                eprintln!("Error occured invalid parameter setting");
                                                exit(-1);
                                        }
                        };
                        set_hvf_scheduler_current();
                        do_work((j as i32)+1);
                        exit(0);
                }
        }

        for j in 0..20 {
                unsafe {
                        waitpid(procs[j], null::<i32>() as *mut i32, 0);
                };
        }
}
