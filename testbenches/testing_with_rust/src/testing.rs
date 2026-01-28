use std::{process::exit, ptr::null, time::{SystemTime, UNIX_EPOCH}};

use libc::{sleep, waitpid, pid_t};

use crate::syscall_bindings::{d_params, get_sched_params, get_sched_score, print_params, safe_fork, safe_getpid, set_hvf_scheduler_current, set_sched_params, test_for_wrong_set};

//scheduler testing

const DELAY: u32 = 750;
const K: u32 = 1000;
const H13: u32 = 1300;

pub fn core_delay() {
        let mut a: f64 = 1.1;
        for j in 0..100000 {
                a += f64::sqrt(1.1)*f64::sqrt(1.2)*f64::sqrt(1.3)*f64::sqrt(1.4)*f64::sqrt(1.5);
                a += f64::sqrt(1.6)*f64::sqrt(1.7)*f64::sqrt(1.8)*f64::sqrt(1.9)*f64::sqrt(2.0);
                a += f64::sqrt(1.1)*f64::sqrt(1.2)*f64::sqrt(1.3)*f64::sqrt(1.4)*f64::sqrt(1.5);
                a += f64::sqrt(1.6)*f64::sqrt(1.7)*f64::sqrt(1.8)*f64::sqrt(1.9);
        }
}

pub fn delay(workload: i32) {
        let total_workload: i32 = workload*(DELAY as i32);
        for i in 0..total_workload {
                core_delay();
        }
}

pub fn do_work(workload: i32) {
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


pub fn test_scheduler() {
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

//syscall testing

const GOOD_TIME: u32 = 1200;
const BAD_TIME: u32 = 120000;

pub fn make_first_right() {
        let now = SystemTime::now()
                .duration_since(UNIX_EPOCH).expect("Time went backwards");
        let now_secs: i64 = now.as_secs() as i64;
        match set_sched_params(now_secs + 7, now_secs + 12, GOOD_TIME as i64) {
                Ok(()) => (),
                Err(_err) => {
                        println!("[-] incorrect rejection of parameters with values: D1: {} D2: {} CT: {}", now_secs+7, now_secs+12, GOOD_TIME);
                        return;
                }
        }
        let mut params: d_params = d_params { deadline1: 0, deadline2: 0, computation_time: 0 };
        match get_sched_params(&mut params) {
                Ok(()) => (),
                Err(_err) => {
                        println!("Something went wrong ingetting the parameters");
                }
        }
        print_params(&params);
}

pub fn make_first_wrong() {
        let now = SystemTime::now()
                .duration_since(UNIX_EPOCH).expect("Time went backwards");
        let now_secs: i64 = now.as_secs() as i64;
        test_for_wrong_set(now_secs + 11, now_secs + 8, GOOD_TIME as i64,
                &"[+] correctly rejecting d2<d1 parameters".to_string());
        test_for_wrong_set(now_secs + 7, now_secs + 13, BAD_TIME as i64,
                &"[+] correctly rejecting parameters with computation_time more than given in deadlines".to_string());
}


pub fn test_syscalls() {
        make_first_right();
        let score_before_sleeping: i64 = match get_sched_score() {
                Ok(score) => score,
                Err(_err) => {
                        println!("Something went wrong calculating the score");
                        return;
                }
        };
        println!("score before sleeping = {}", score_before_sleeping);

        unsafe{sleep(4)};
        let score_after4: i64 = match get_sched_score() {
                Ok(score) => score,
                Err(_err) => {
                        println!("Something went wrong calculating the score");
                        return;
                }
        };
        println!("score after sleeping 4 = {}", score_after4);

        unsafe{sleep(3)};
        let score_after7: i64 = match get_sched_score() {
                Ok(score) => score,
                Err(_err) => {
                        println!("Something went wrong calculating the score");
                        return;
                }
        };
        println!("score after sleeping 7 = {}", score_after7);

        unsafe{sleep(2)};
        let score_after9: i64 = match get_sched_score() {
                Ok(score) => score,
                Err(_err) => {
                        println!("Something went wrong calculating the score");
                        return;
                }
        };
        println!("score after sleeping 9 = {}", score_after9);

        unsafe{sleep(4)};
        let score_after13: i64 = match get_sched_score() {
                Ok(score) => score,
                Err(_err) => {
                        println!("Something went wrong calculating the score");
                        return;
                }
        };
        println!("score after sleeping 13 = {}", score_after13);

        make_first_wrong();
}
