#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

failures=0

check_not_tracked() {
    local path="$1"
    if git ls-files --error-unmatch -- "$path" >/dev/null 2>&1; then
        printf 'tracked unexpectedly: %s\n' "$path" >&2
        failures=1
    fi
}

check_prefix_empty() {
    local prefix="$1"
    if grep -q "^${prefix}/" < <(git ls-files); then
        printf 'tracked unexpectedly under: %s\n' "$prefix" >&2
        failures=1
    fi
}

check_prefix_empty "lichee/rtos/build"

check_not_tracked "lichee/rtos/components/common/aw"
check_not_tracked "lichee/rtos/components/common/thirdparty"
check_not_tracked "lichee/rtos/drivers/rtos-hal"

check_not_tracked "lichee/rtos/.config"
check_not_tracked "lichee/rtos/.dbuild/pretty/__pycache__/prettyformat.cpython-312.pyc"
check_not_tracked "lichee/rtos/include/generated/xr875s1h10_solution_c906/autoconf.h"
check_not_tracked "lichee/rtos/include/generated/xr875s1h10_solution_m33/autoconf.h"

check_not_tracked "board/xr875s1h10/solution/bin/freertos.fex"
check_not_tracked "board/xr875s1h10/solution/bin/rtos_arm_sun20iw2p1-recovery.fex"
check_not_tracked "board/xr875s1h10/solution/bin/rtos_arm_sun20iw2p1.fex"
check_not_tracked "board/xr875s1h10/solution/bin/rtos_hpsram_arm_sun20iw2p1.fex"
check_not_tracked "board/xr875s1h10/solution/bin/rtos_riscv_sun20iw2p1.fex"

if [ "$failures" -ne 0 ]; then
    exit 1
fi

printf 'tracked local-state check passed\n'
