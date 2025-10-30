#!/bin/sh

OUTPUT_FILE1="times_hvf_log.csv"
OUTPUT_FILE2="switches_hvf_log.csv"

echo "pid,init_sched_value,turnaround_time,wait_time,computation_time,response_time" > "$OUTPUT_FILE1"
echo "from_to,pid,curr_sched_value,time_event" > "$OUTPUT_FILE2"

cat /sys/kernel/debug/tracing/trace | grep "hvf_task_terminated" | \
    awk -F, '{ print $2","$3","$4","$5","$6","$7 }' >> "$OUTPUT_FILE1"

dmesg | grep "hvf_task_switched_" | \
    awk -F, '{ print $2","$3","$4","$5 }' >> "$OUTPUT_FILE2"
