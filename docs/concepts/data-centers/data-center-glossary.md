> Retired 2026-11-12. Telemetry source is the P3 facility slice. Do not extend.

# Data center glossary

**building.** The structure that holds one or more data halls. It sits between the hall and the campus.

**campus (site).** The whole data center. Utility power arrives here and is conditioned in the electrical plant. A campus has one or more buildings and the shared heat rejection outside. Site is another name for the same unit. This simulation stays inside one hall and does not model the campus above it.

**CDU.** Coolant distribution unit. A cooling asset that meters coolant from the loop to its rack rows. Pump speed, valve position, and fouling set that flow. In this simulation CDU 01 feeds rows A and B. CDU 02 feeds rows C and D.

**hall.** The room in a data center building where a block of racks runs. It is the unit where a known IT load, a power feed, and a cooling loop meet. A building holds one or more halls. This simulation stays inside one hall.

**inlet.** The temperature of the coolant, or air, as it enters the equipment being cooled. On a rack row it is the cooling delivered to that row. A rising inlet means heat is arriving faster than the loop is removing it.

**IT load.** The electrical power drawn by the information technology equipment in the racks: servers, GPUs, storage, and network gear. IT stands for information technology. That power leaves the racks as heat, which the cooling loop has to remove. Power used by pumps and heat rejection is a separate facility share, not part of the IT load.

**row.** A line of racks that share a cooling branch. In this simulation a row is one aggregated asset, not each rack. Rows A and B are fed by CDU 01. Rows C and D are fed by CDU 02. The row is where IT load becomes heat.

**UPS.** Uninterruptible power supply. It sits between the utility feed and the IT equipment and carries the load from its batteries when utility power fails or sags, until the generators start and take over. 
