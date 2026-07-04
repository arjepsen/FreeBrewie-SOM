#include <stdio.h>
#include <unistd.h>

#include "Platform/Display.h"
#include "Platform/Time_base.h"
#include "UI/UI.h"

int main()
{
    display_t display;
    ui_t ui;
    status_screen_view_model_t vm;

    if (!display_init(&display))
    {
        fprintf(stderr, "display init failed in simulator\n");
        return 1;
    }

    ui_init(&ui);

    status_screen_view_model_init(&vm);
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

    for (;;)
    {
        /*
         * Keep one changing value visible so it is obvious the simulator event/render loop is
         * alive. This loop should later be aligned with the target app_update() cadence.
         */
        vm.heartbeat_count++;
        ui_update(&ui, &vm);
        display_update(&display, time_base_now_ms());
        usleep(5000);
    }

    return 0;
}
