# Hall fault — escalation and a transient warning

**Status:** Runnable. Demo file: [`edge/scenarios/002.yaml`](../../../edge/scenarios/002.yaml).  
**Date:** 2026-11-21  
**How-to:** [facility-demo.md](../../how-to/facility-demo.md) (default `SCENARIO`)

A utility sag puts the UPS on battery, and that warning clears. Later a cooling unit stops holding its supply-air temperature. The hall warms, first past the recommended band, then to the allowable ceiling. The UPS event is not part of the warmup.

The publishers invent these readings. Each one follows its own clock. The room sensor does not read the cooling unit.

---

## Nine minutes

At one snapshot per second:

For the first minute, all three assets are healthy. `CRAH-01` reports supply air at 16 °C, `RUNNING`, with an empty alarm list. `ROOM-01` reports the hall at 22 °C, `RUNNING`. `UPS-01` reports 40 percent load, `RUNNING`.

At one minute, tick 60, `UPS-01` goes on battery for 30 seconds. Status is `FAULTED`. The alarm is `UPS_ON_BATTERY`, severity `warning`, message "UPS on battery after a utility sag." The load stays 40 percent. At tick 90 the alarm clears and the status returns to `RUNNING`. This is the only asset in the script that recovers.

At three minutes, tick 180, `CRAH-01` steps from 16 °C to 28 °C and stays there. Status becomes `FAULTED`. The alarm is `COOLING_SUPPLY_HIGH`, severity `warning`, message "Supply air above setpoint." There is no ramp. Every later snapshot repeats 28 °C and that warning.

At six minutes, tick 360, `ROOM-01` steps from 22 °C to 27 °C. Status becomes `FAULTED`. The alarm is `ROOM_TEMP_HIGH`, severity `warning`, message "Hall temperature above recommended limit." The hall has left the recommended band. The cooling unit is still blowing hot supply air.

At nine minutes, tick 540, `ROOM-01` steps from 27 °C to 32 °C and stays there. The same alarm code remains. Severity becomes `error`, message "Hall temperature at allowable limit." The item carries `error` in place of `warning`. From then on the hall is at the ceiling and the cooling unit is still in warning. `UPS-01` is healthy at 40 percent.

The publishers do not stop. They keep sending one snapshot per second until the demo container is stopped. `CRAH-01` and `ROOM-01` stay `FAULTED`.

---

## What each store shows

The local historian records every second, including the healthy minutes, the UPS episode, and the snapshots the bridge skips.

The cloud copy is about one item in five while an asset is healthy. An alarm is always posted, so the UPS warning is a short run of items and then the one-in-five rule returns. `CRAH-01` posts every snapshot after tick 180. `ROOM-01` posts every snapshot after tick 360. At tick 540 those room items change from `warning` to `error` and stay `error`.

A collector that keeps one series per severity sees three warning series and one error series. The UPS warning series goes quiet when the cabinet recovers, which is the resolve. The room warning series goes quiet when the alarm becomes `error`, and the room error series starts. The cooling warning series keeps growing. A building system would often leave the room warning active under the error. This script carries one alarm, and that alarm changes severity.

The simpler latch without UPS recovery or room escalation is [001](001-hall-fault-narrative.md).
