#ifndef FREEBREWIE_APP_ORCHESTRATOR_H
#define FREEBREWIE_APP_ORCHESTRATOR_H

/****************************************************************************************
 * @file App_orchestrator.h
 * @brief Logic-side application state orchestrator.
 *
 * Responsibility: Keep app state coherent by routing MCU facts and user requests through
 * the right logic modules.
 * Owns: Future high-level app state, allowed-action routing, and workflow coordination.
 * Must not own: Widgets, serial transport, protocol parsing, or low-level hardware control.
 ****************************************************************************************/

/*
 * Reserved for the first real app-level routing step.
 *
 * The current UI only performs screen navigation, and the diagnostic status screen has its
 * own Status_view_model module. Do not put presentation-only data here just to make the
 * orchestrator active. Add public state/functions when a user request needs to be checked
 * against faults, machine state, startup state, or workflow permissions.
 */

#endif
