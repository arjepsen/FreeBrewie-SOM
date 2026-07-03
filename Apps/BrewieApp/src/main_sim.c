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
        vm.heartbeat_count++;
        ui_update(&ui, &vm);
        display_update(&display, time_base_now_ms());
        usleep(5000);
    }

    return 0;
}
