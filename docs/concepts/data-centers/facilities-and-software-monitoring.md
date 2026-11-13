# Data center monitoring: plant and software

A large data center runs two monitoring rotations. Facilities owns the plant. A software or platform rotation owns the control plane. They join the same incident when a failure crosses that line. They do not share one pager.

## Facilities

Facilities, often called critical environment or data-center engineering operations, watches the building as a plant.

- Utility power, generators, switchgear, UPS, and PDUs
- Cooling: chillers, CRAH or CRAC units, pumps, and liquid-cooling loops
- The room those systems condition: temperature, humidity, airflow, and leaks
- Fire and life safety, where that desk owns the signal

The screens are a building-management system and an electrical power monitor. A page can require someone to walk to a breaker, a valve, or a cooling unit. The escalation tree runs through facilities engineers, mechanics, and electricians. A wrong action on this side can drop power or cooling to a hall.

## Software

The software rotation watches the machines and the systems running on them.

- Hosts, network, and storage
- The cluster control plane
- The services those machines are there to run

A page here is a drain, a failover, a capacity move, or a rollback. The skill and the vendor on the other end of the call are different from the facilities desk. The tools are the usual service monitors, not the building system.

## Where the two views meet

The joint picture is real, and it sits above the two rotations.

A site incident pulls both desks onto one bridge. Capacity planning reads IT load next to power and cooling headroom. The software on-call sees hot hosts or machines taken out of service. Facilities sees the cooling unit or UPS that caused it. That shared timeline is a bridge and a handoff, not one team watching one board for the whole night.

Smaller sites blur the night shift. One operator may see both screens and then call the specialist. The owners underneath stay split: the plant on one side, the control plane on the other.

## What a plant-reliability example follows

A first monitoring shift aimed at plant reliability follows the facilities board. Power, cooling, environment, and the asset named on the page are the signals. The handoff into the software rotation is part of the story when the plant failure shows up as hot hosts or lost capacity. It is a second desk, not a second set of points on the same pager.
