use crate::testing::{test_scheduler, test_syscalls};
use std::env;

mod syscall_bindings;
mod testing;

fn main() {
        let args: Vec<String> = env::args().collect();

        if args.len() > 1 {
                if args[1].eq("syscalls") {
                        test_syscalls();
                }
                else {
                        test_scheduler();
                }
        }
        else {
                test_scheduler();
        }
}
