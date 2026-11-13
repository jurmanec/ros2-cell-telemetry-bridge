# Facility telemetry — initial notes

**Status:** Notes. The facility plan is done through the demo. Start it with [facility-demo.md](../how-to/facility-demo.md).  
**Date:** 2026-11-12  
**Follows:** the frozen weld path on [v2.0.0](https://github.com/jurmanec/ros2-cell-telemetry-bridge/releases/tag/v2.0.0). That path stays as it is.  
**Context:** [facilities-and-software-monitoring.md](../concepts/data-centers/facilities-and-software-monitoring.md)  
**Plan:** [facility-snapshot-plan.md](facility-snapshot-plan.md)

These notes describe how this repository can supply telemetry for a later monitoring shift. The shift watches a plant, pages a facilities desk, and keeps a record of the response. This repository stops at the cloud copy. It does not decide that an alert fired, and it does not run the shift.

The worked example is a data-center plant: power, cooling, and the room those systems condition. The product being shown is facility reliability. The data center is the example.

---

## What this slice supplies

The weld demo is the template.

| Weld path | Facility path |
|-----------|----------------|
| One sink per cell, `cell_id` on the snapshot | One sink per asset, asset id on the snapshot |
| Topic `cell/weld_snapshot` | A new topic. The weld topic stays the weld contract |
| InfluxDB keeps every snapshot | A site historian keeps every point |
| Bridge posts every fifth snapshot, plus any snapshot with an alarm | The same rule, per asset |
| DynamoDB holds the reduced copy | The same store, a new snapshot shape |

A downstream monitor can count faults for one asset and emit fire and resolve. The runbook link and the team sit on that monitor job. The building system does not know them.

One snapshot is one asset. A page names a unit: this cooling unit, this UPS, this leak sensor. The hall is a tag on the asset, so a query can still show the cooling unit and the room temperature together. A hall message that embeds every asset would send the whole room whenever one unit alarms.

---

## First hall

Three assets, one scripted cascade.

- A CRAH or CRAC unit goes to a cooling fault. Its supply-air temperature climbs.
- The hall environment point follows. Room temperature crosses a limit a few minutes later.
- A UPS in the same hall stays healthy. The page is cooling, not power.

ROS 2 stands in for the building-management system. The publishers invent the readings, stamp the asset id, and publish one snapshot per asset. The historian and the bridge subscribe. Detail and diagrams: [building-management-system.md](building-management-system.md). Asset identity wants a small set of tags: site, hall, and asset. A handful of halls and asset types, not a series per sensor.

The software desk is a later fixture. Lost cooling shows up there as hot hosts or lost capacity. That is a second emitter and a handoff, not more points on the facilities pager. See [facilities-and-software-monitoring.md](../concepts/data-centers/facilities-and-software-monitoring.md).

---

## Left frozen

- `cell/telemetry` and `schema_version` 1
- `cell/weld_snapshot` and `schema_version` 2
- The weld reduction policy, the weld historian, and the two-cell demo

A new topic and a new snapshot type carry the facility records. Averaging, a fleet manager, and a cloud time-series database stay out.
