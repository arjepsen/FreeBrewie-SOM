#ifndef FREEBREWIE_PROCESS_RUNNER_H
#define FREEBREWIE_PROCESS_RUNNER_H

/****************************************************************************************
 * @file Process_runner.h
 * @brief Passive runtime walker for a prepared process plan.
 *
 * Responsibility: hold the current process-plan position and carried machine targets.
 * Owns: active plan pointer, current step index, prompt/wait state, and current target image.
 * Must not own: recipe editing, LVGL widgets, serial transport, MCU ACK handling, or hard
 * safety/fault decisions.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Machine_targets.h"
#include "Process_plan.h"

typedef enum
{
    PROCESS_RUNNER_STATE_IDLE = 0,
    PROCESS_RUNNER_STATE_RUNNING,
    PROCESS_RUNNER_STATE_WAITING_FOR_USER,
    PROCESS_RUNNER_STATE_COMPLETE,
    PROCESS_RUNNER_STATE_ERROR
} process_runner_state_t;

typedef struct
{
    const process_plan_t *plan;
    machine_targets_t targets;
    process_runner_state_t state;
    uint8_t current_step_index;
    const char *status_text;  // Short diagnostic text for status/debug UI.
} process_runner_t;

void process_runner_init(process_runner_t *runner);
bool process_runner_start(process_runner_t *runner, const process_plan_t *plan);
bool process_runner_confirm_prompt(process_runner_t *runner);
const process_plan_step_t *process_runner_current_step(const process_runner_t *runner);
const machine_targets_t *process_runner_current_targets(const process_runner_t *runner);
bool process_runner_is_active(const process_runner_t *runner);

#endif
