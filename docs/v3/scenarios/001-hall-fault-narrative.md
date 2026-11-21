# Hall fault — six-minute latch

**Status:** Runnable. Demo file: [`edge/scenarios/001.yaml`](../../../edge/scenarios/001.yaml).  
**Date:** 2026-11-21  
**How-to:** [facility-demo.md](../../how-to/facility-demo.md) (`SCENARIO=001`)

A cooling unit stops holding its supply-air temperature. A few minutes later the hall crosses its temperature limit. The UPS in the same hall stays healthy. Nothing recovers: both cooling and room faults latch until the demo container is stopped.

The publishers invent these readings. Each one follows its own clock. The room sensor does not read the cooling unit.

---

## Six minutes

At one snapshot per second:

For the first three minutes, all three assets are healthy. `CRAH-01` reports supply air at 16 °C, `RUNNING`, with an empty alarm list. `ROOM-01` reports the hall at 22 °C, `RUNNING`. `UPS-01` reports 40 percent load, `RUNNING`.

At three minutes, tick 180, `CRAH-01` steps from 16 °C to 28 °C and stays there. Status becomes `FAULTED`. The alarm is `COOLING_SUPPLY_HIGH`, severity `warning`, message "Supply air above setpoint."

At six minutes, tick 360, `ROOM-01` steps from 22 °C to 32 °C and stays there. Status becomes `FAULTED`. The alarm is `ROOM_TEMP_HIGH`, severity `warning`, message "Hall temperature above limit." There is no intermediate warning band and no severity change.

`UPS-01` never faults. The publishers keep sending one snapshot per second until the container is stopped. `CRAH-01` and `ROOM-01` stay `FAULTED`.

---

## What each store shows

The local historian records every second, including the healthy minutes and the snapshots the bridge skips.

The cloud copy is about one item in five while an asset is healthy. An alarm is always posted, so `CRAH-01` posts every snapshot after tick 180 and `ROOM-01` posts every snapshot after tick 360. `UPS-01` stays on the one-in-five rule.

Scenario [002](002-hall-fault-narrative.md) adds a transient UPS warning and a room warning that escalates to `error`. This file is the simpler latch used to prove the first hall path.
