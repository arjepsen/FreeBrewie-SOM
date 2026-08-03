#include <stdio.h>

#include "Logic/Brewing_process_view_model.h"
#include "Logic/Status_view_model.h"
#include "Platform/Display.h"
#include "Platform/Time_base.h"
#include "UI/UI.h"

#define SIM_UI_REFRESH_PERIOD_MS 250U
#define SIM_HEARTBEAT_PERIOD_MS 1000U

int main()
{
    display_t display;
    ui_t ui;
    status_screen_view_model_t vm;
    brewing_process_view_model_t brewing_process_vm;
    uint64_t now_ms;
    uint64_t last_ui_update_ms;
    uint64_t last_heartbeat_ms;

    if (!display_init(&display))
    {
        fprintf(stderr, "display init failed in simulator\n");
        return 1;
    }

    ui_init(&ui, NULL, NULL);

    status_screen_view_model_init(&vm);
    brewing_process_view_model_init(&brewing_process_vm);
    /*
     * The simulator does not talk to the MCU yet. Fill the view model with fixed values so
     * UI layout and navigation can be tested locally without serial hardware.
     */
    vm.display_text = "simulator";
    vm.serial_text = "not used";
    vm.heartbeat_text = "simulator only";
    vm.last_rx_text = "none";
    vm.link_text = "n/a";
    vm.mcu_status_text = "sim";
    vm.pressure_text = "0";
    vm.pump_text = "off / off";
    vm.solenoid_text = "closed";
    vm.fault_text = "none";
    vm.heartbeat_count = 0;
    last_ui_update_ms = 0U;
    last_heartbeat_ms = time_base_now_ms();

    for (;;)
    {
        now_ms = time_base_now_ms();

        /*
         * Keep the simulator's visible data cadence close to the target. The render/touch loop
         * still runs every pass through display_update(), but UI values are refreshed at the
         * same 250 ms pace as the appliance build.
         */
        if ((now_ms - last_heartbeat_ms) >= SIM_HEARTBEAT_PERIOD_MS)
        {
            vm.heartbeat_count++;
            last_heartbeat_ms = now_ms;
        }

        if ((now_ms - last_ui_update_ms) >= SIM_UI_REFRESH_PERIOD_MS)
        {
            brewing_process_view_model_update(&brewing_process_vm,
                                              &vm,
                                              APP_ORCHESTRATOR_STATE_IDLE,
                                              "Simulator",
                                              NULL);
            ui_update(&ui, &vm, &brewing_process_vm);
            last_ui_update_ms = now_ms;
        }

        display_update(&display, now_ms);
    }

    return 0;
}
