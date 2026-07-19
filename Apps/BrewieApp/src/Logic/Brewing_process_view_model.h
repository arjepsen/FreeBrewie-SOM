#ifndef FREEBREWIE_BREWING_PROCESS_VIEW_MODEL_H
#define FREEBREWIE_BREWING_PROCESS_VIEW_MODEL_H

/****************************************************************************************
 * @file Brewing_process_view_model.h
 * @brief Read-only presentation model for the Active Brewing screen.
 *
 * Responsibility: describe the current brewing-process presentation state for UI widgets.
 * Owns: stage/progress/control-display fields used by the Active Brewing screen.
 * Must not own: brewing permission, safety decisions, MCU commands, or recipe execution.
 ****************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "Status_view_model.h"

typedef enum
{
    BREWING_PROCESS_STAGE_WATER = 0,
    BREWING_PROCESS_STAGE_MASH,
    BREWING_PROCESS_STAGE_BOIL,
    BREWING_PROCESS_STAGE_DONE,
    BREWING_PROCESS_STAGE_COUNT
} brewing_process_stage_t;

typedef struct
{
    /** Current stage highlighted in the Active Brewing process strip. */
    brewing_process_stage_t current_stage;
    /** Product-facing short state text. */
    const char *state_text;
    /** Product-facing secondary detail text. */
    const char *detail_text;
    /** Current process progress shown in the circular progress readout. */
    uint8_t progress_percent;
    /** True when Pause should be presented as available. This remains false for now. */
    bool pause_enabled;
    /** True when Stop should be presented as available. This remains false for now. */
    bool stop_enabled;
} brewing_process_view_model_t;

void brewing_process_view_model_init(brewing_process_view_model_t *view_model);
void brewing_process_view_model_update(brewing_process_view_model_t *view_model,
                                       const status_screen_view_model_t *status_view_model);

#endif
