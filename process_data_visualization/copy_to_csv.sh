#!/bin/sh

OUTPUT_FILE="hvf_log.csv"

echo "pid,init_sched_value,turnaround_time,wait_time,computation_time,response_time" > "$OUTPUT_FILE"

cat /sys/kernel/debug/tracing/trace | grep "hvf_task_terminated" | \
    awk -F, '{ print $2","$3","$4","$5","$6 }' >> "$OUTPUT_FILE"
