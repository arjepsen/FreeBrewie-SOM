#include "Process_runner.h"

static bool process_runner_step_is_valid(const process_runner_t *runner);
static void process_runner_enter_current_step(process_runner_t *runner);

/****************************************************************************************
 * @brief Return true when the runner points at an existing process-plan step.
 ****************************************************************************************/
static bool process_runner_step_is_valid(const process_runner_t *runner)
{
    return (runner != NULL && runner->plan != NULL &&
            runner->current_step_index < runner->plan->step_count);
}

/****************************************************************************************
 * @brief Enter the current step and apply its immediate runtime meaning.
 *
 * This is intentionally conservative. Target segments update the carried machine target
 * image, prompts stop for user confirmation, and complete stops the runner. Time, sensor,
 * and fault-based advancement will be added around this once the orchestrator owns real
 * active brewing permissions.
 ****************************************************************************************/
static void process_runner_enter_current_step(process_runner_t *runner)
{
    const process_plan_step_t *step;

    if (!process_runner_step_is_valid(runner))
    {
        runner->state = PROCESS_RUNNER_STATE_ERROR;
        runner->status_text = "Invalid process step";
        return;
    }

    step = &runner->plan->steps[runner->current_step_index];
    runner->status_text = step->label;

    switch (step->kind)
    {
        case PROCESS_PLAN_STEP_TARGET_SEGMENT:
            machine_targets_apply_process_step(&runner->targets, step);
            runner->state = PROCESS_RUNNER_STATE_RUNNING;
            break;

        case PROCESS_PLAN_STEP_PROMPT:
            runner->state = PROCESS_RUNNER_STATE_WAITING_FOR_USER;
            break;

        case PROCESS_PLAN_STEP_COMPLETE:
            runner->state = PROCESS_RUNNER_STATE_COMPLETE;
            runner->status_text = "Process complete";
            break;

        default:
            runner->state = PROCESS_RUNNER_STATE_ERROR;
            runner->status_text = "Unknown process step";
            break;
    }
}

/****************************************************************************************
 * @brief Reset the runner to idle with no active plan.
 ****************************************************************************************/
void process_runner_init(process_runner_t *runner)
{
    if (runner == NULL)
    {
        return;
    }

    runner->plan = NULL;
    machine_targets_init(&runner->targets);
    runner->state = PROCESS_RUNNER_STATE_IDLE;
    runner->current_step_index = 0U;
    runner->status_text = "No active process";
}

/****************************************************************************************
 * @brief Start walking a prepared process plan.
 *
 * The runner does not check lid locks, faults, startup state, or user permissions. Those
 * checks belong in the future app orchestrator before it calls this function.
 ****************************************************************************************/
bool process_runner_start(process_runner_t *runner, const process_plan_t *plan)
{
    if (runner == NULL || plan == NULL || !plan->ready_for_preflight ||
        plan->step_count == 0U)
    {
        return false;
    }

    runner->plan = plan;
    machine_targets_clear(&runner->targets);
    runner->state = PROCESS_RUNNER_STATE_RUNNING;
    runner->current_step_index = 0U;
    runner->status_text = "Process started";
    process_runner_enter_current_step(runner);

    return (runner->state != PROCESS_RUNNER_STATE_ERROR);
}

/****************************************************************************************
 * @brief Advance past the current prompt after the user confirms it.
 *
 * This is the only advancement operation in the first runner scaffold. Duration and sensor
 * exits require current machine facts and timing policy, so they should be added with the
 * real active-brewing workflow instead of guessed here.
 ****************************************************************************************/
bool process_runner_confirm_prompt(process_runner_t *runner)
{
    if (runner == NULL || runner->state != PROCESS_RUNNER_STATE_WAITING_FOR_USER)
    {
        return false;
    }

    runner->current_step_index++;
    process_runner_enter_current_step(runner);

    return (runner->state != PROCESS_RUNNER_STATE_ERROR);
}

/****************************************************************************************
 * @brief Return the current process-plan step, or NULL when none is active.
 ****************************************************************************************/
const process_plan_step_t *process_runner_current_step(const process_runner_t *runner)
{
    if (!process_runner_step_is_valid(runner))
    {
        return NULL;
    }

    return &runner->plan->steps[runner->current_step_index];
}

/****************************************************************************************
 * @brief Return the carried target image that would later become a control snapshot.
 ****************************************************************************************/
const machine_targets_t *process_runner_current_targets(const process_runner_t *runner)
{
    if (runner == NULL)
    {
        return NULL;
    }

    return &runner->targets;
}

/****************************************************************************************
 * @brief Return true while the runner owns a not-yet-finished process.
 ****************************************************************************************/
bool process_runner_is_active(const process_runner_t *runner)
{
    return (runner != NULL &&
            (runner->state == PROCESS_RUNNER_STATE_RUNNING ||
             runner->state == PROCESS_RUNNER_STATE_WAITING_FOR_USER));
}
