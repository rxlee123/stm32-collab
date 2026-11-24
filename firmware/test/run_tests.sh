#!/bin/sh
# Run all PC tests (no robot needed). From the firmware folder:  sh test/run_tests.sh
set -e
T="-Wall -Wextra -std=c99 -Itest -ICore/Inc"
gcc $T -DAIROST_MODE=0 -DMOTOR_MIN=0 -DSTART_RAMP_MS=0 -DWIDE_TO_INVERT_MS=500 test/test.c Core/Src/robot.c -o test/run && ./test/run
gcc $T -DAIROST_MODE=0 -DMOTOR_MIN=0 -DSTART_RAMP_MS=0 "-DROUTE_LIST={ R_LEFT, R_FINISH }" test/test_route.c Core/Src/robot.c -o test/run_route && ./test/run_route
gcc $T -DAIROST_MODE=0 -DMOTOR_MIN=0 -DSTART_RAMP_MS=0 "-DROUTE_LIST={ R_BLIND(100), R_OPEN, R_PICKUP(50), R_LEFT, R_DROP(50), R_INVERT, R_FINISH }" test/test_map.c Core/Src/robot.c -o test/run_map && ./test/run_map
gcc $T -DAIROST_MODE=0 test/test_motor_min.c Core/Src/robot.c -o test/run_motor && ./test/run_motor
for m in 1 2 3 0; do gcc $T -DAIROST_MODE=0 '-DBT_NAME="Doomsday01"' -DMODULE_TYPE=$m test/test_rename.c Core/Src/robot.c -o test/run_rename && ./test/run_rename; done
gcc $T -DAIROST_MODE=1 -DMOTOR_MIN=0 -DSTART_RAMP_MS=0 test/test_airost.c Core/Src/robot.c -o test/run_airost && ./test/run_airost
gcc $T -DAIROST_MODE=1 -DMOTOR_MIN=0 -DSTART_RAMP_MS=0 -DARRIVE_MM=60 test/test_airost.c Core/Src/robot.c -o test/run_airost_creep && ./test/run_airost_creep
