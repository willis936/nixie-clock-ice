# nixie-clock-ice
 Hardware and software source for a GPS nixie tube clock with delay correction.

Uses a PICO-iCE v3 FPGA + MCU module.

 - Raspberry Pi RP2040 processor
 - Lattice UltraPlus ICE40UP5K FPGA with 5.3k LUTs, 1Mb SPRAM, 120Kb DPRAM, 8 Multipliers

# Requirements
1. Display the time with 1 microsecond accuracy.
2. Display the time with 1 second precision.
3. Use nixie tubes to display the time.
4. Do not require or allow the user to manually set the time.
5. Display the time within 30 seconds of power on.
6. Use a single power entry.
7. Use no more than 3 W of power.  [not a real requirement, also difficult to achive with brightness control]
8. Use a single radio receiver to establish local timebase.
9. Have a user-selectable mode for Daylight Savings Time (DST) checking.
10. Use longitude to estimate DST date range.
11. Have a user-selectable mode to force DST offset on (off when both this and DST check mode are off).
12. Use longitude to estimate timezone.
13. Have a user-selectable timezone offset.
14. Be able to use software to control nixie tube brightness.
15. Use an ambient light detector to automatically adjust nixie tube brightness.
16. Have a user-selectable manual brightness mode.
17. Use a potentiometer to set the brightness in manual brightness mode.
18. Run a nixie tube anti-poisoning routine once a day (cycle through all digits at high brightness for a minute).
19. Have a user-selectable mode to enable daily anti-poisoning routine
20. Have a user-selectable mode to force anti-poisoning routine on indefinitely.
21. Have a user-selectable 12-hour / 24-hour display mode.
22. Use a nixie tube decimal point to indicate AM/PM in 12-hour display mode.
23. Use a nixie tube decimal point to indicate that the radio receiver is locked.
 
 
