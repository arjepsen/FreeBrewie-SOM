# FreeBrewie UI/UX Philosophy
_Date: 2026-07-04_

## Purpose
This document defines how we should make UI and UX decisions for the FreeBrewie SOM app.

The short version:

**Start by imitating the original Brewie user experience closely enough that the machine
still feels like a Brewie, but rebuild every screen with clean structure, clear code,
readable modules, and performance-first LVGL behavior.**

The old codebases and graphics are references for product feel, screen roles, navigation
ideas, wording, icons, and visual tone. They are not implementation models for the new app.

---

## Core direction
The first real UI pass should lean deliberately toward the original Brewie interface:

- similar warm orange and charcoal color direction
- similar appliance-focused menu and workflow structure
- similar screen roles where they still make sense
- similar large, touch-friendly controls
- similar brewing/cleaning/settings mental model
- reuse or recreate original-looking graphics where that helps the product feel familiar

But this is not a clone project.

Whenever the original behavior conflicts with clarity, safety, maintainability, or speed, the
new app should choose the better design.

---

## What "imitate the old UI" means
Imitating the old UI means using it as the first UX vocabulary:

- which screens existed
- what the main menu offered
- how brewing, cleaning, settings, and extras were grouped
- what colors, icons, and visual hierarchy made the appliance recognizable
- what graphical elements made the UI feel like a product rather than a debug tool
- what common user journeys the old app expected

It does **not** mean:

- copying old code structure
- copying old timing behavior
- copying sluggish transitions
- copying unclear or fiddly interactions
- copying implementation shortcuts
- preserving old screens that no longer make sense

The old UI is the reference sketch. The new app is the engineered version.

---

## Performance principles
The A13 SOM and 272x480 portrait UI are limited enough that performance is a design rule,
not a late optimization phase.

Prefer:

- stable layouts where small values update in place
- partial redraws
- short local animations
- pressed-state feedback on buttons
- simple popups over whole-screen movement when the content is small
- prebuilt screens or reusable components where it avoids repeated object churn
- dirty checks before setting unchanged LVGL label text
- raw values and screen-specific formatting for production screens

Avoid:

- continuous full-screen animation
- long sliding transitions that redraw most of the screen every frame
- animated backgrounds
- frequently rebuilding whole screens
- calling formatting functions every UI tick when values have not changed
- dynamic text layouts where fixed compact fields would work better

The UI can still feel polished. It should feel polished through responsiveness, clear touch
feedback, careful spacing, and short purposeful motion rather than heavy animation.

---

## Visual principles
The intended visual direction is:

- charcoal/black base
- warm orange primary actions, close to the original Brewie palette
- grey secondary text
- restrained green for ready/healthy states
- restrained red/orange for faults and warnings
- clear icons where icons are easier to recognize than text
- large touch targets
- minimal decorative noise

Buttons must look like buttons. Values and labels must not look tappable unless they are
tappable.

If a UI element opens a popup or navigates, it should have a clear affordance: button styling,
icon, chevron, pressed state, or other visual cue.

---

## Navigation principles
The navigation should begin from the original Brewie mental model, then simplify where useful.

Expected first-level destinations:

- Home
- Brew / recipes later
- Clean
- Manual service
- Settings
- Status / diagnostics

Home should be the normal landing screen. It should answer:

- Is the machine safe/ready?
- Is the MCU connected?
- What can I do next?

The status/diagnostics screen should remain available, but it should not become the normal
home screen. It is for curious users, development, service, and debugging.

---

## Status and diagnostics philosophy
The status screen is allowed to be denser than normal product screens, because it may grow
into a scrollable list of values.

It should still follow the same visual language as the rest of the app:

- same portrait layout
- same colors and typography direction
- same touch behavior
- same menu/back navigation style
- readable rows
- no raw "developer console" feeling unless absolutely needed

Diagnostic values can be text-heavy, but they should be grouped and named clearly. Over time,
the status screen can grow into sections such as:

- MCU link
- temperatures
- pumps
- valves/inlets
- faults
- display/touch
- build/runtime facts

---

## Safety principles
UI buttons must not directly move hardware.

The UI may request an action, but app logic and MCU safety rules must decide whether that
action is allowed.

Manual-service and cleaning screens must be treated carefully:

- placeholders are acceptable early
- real actuator controls need logic-side permission checks
- unsafe actions need confirmations or lockouts
- fault states must override normal navigation and actions

The UI should make safe paths obvious and unsafe paths unavailable.

---

## Code structure principles
The UI should stay divided by responsibility:

- `UI/` owns LVGL objects, screens, visual layout, and screen-local cached display text
- `Logic/` owns app state, workflow decisions, and view models
- `Comms/` owns serial transport, protocol frames, heartbeat, and received MCU facts
- `Platform/` owns display, touch, timing, logging, and target/simulator glue
- `App.c` owns update order and subsystem coordination

Screen code should not parse protocol frames.
Comms code should not know about widgets.
Platform display code should not decide brewing behavior.
Button callbacks should stay tiny and should request actions rather than perform complex
work directly.

---

## How to use old assets and screens
Use the old codebases and graphics as reference material only.

Good uses:

- identify original screen list and navigation structure
- inspect original graphics and color direction
- understand user-facing words and workflow names
- recreate useful visual motifs in the new UI
- compare whether the new app still feels like the same appliance

Bad uses:

- port old UI code directly
- copy old coupling between UI and machine control
- copy old sluggish animation patterns
- copy old layout if it does not fit the portrait target or touch usability

When in doubt, document what the old UI did, then decide whether the new UI should follow it,
adapt it, or replace it.

---

## Practical decision rule
For each screen, ask these questions:

1. What did the original Brewie UI do here?
2. What user need was it trying to serve?
3. Does that behavior still make sense on the new system?
4. Can we make it clearer or safer?
5. Can LVGL render it cheaply on the A13 SOM?
6. Does the code keep UI, logic, comms, and platform responsibilities separate?

If the answer to question 5 or 6 is no, redesign before implementing.

