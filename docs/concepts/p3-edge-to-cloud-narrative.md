# How P3 represents edge-to-cloud telemetry

P3 models a telemetry path from equipment at the edge to services in AWS. In a factory, the edge computer could be a Linux industrial PC serving one workcell. In a robot fleet, it could be the onboard Linux computer on each robot. In either setting, that computer connects local equipment data to a cloud reporting system.

**Readings originate at the equipment.** A temperature probe, accelerometer, or drive fault signal connects to local hardware—not directly to AWS. The edge computer might read it through an I/O interface, USB, or serial connection. Alternatively, a PLC or robot controller acquires the reading, and software on the edge computer retrieves it through a controller interface or vendor library. A ROS publisher node then packages those values into a telemetry message.

P3 simulates this acquisition step. Its `cell_publisher` generates `temp_c`, `vibration`, and `fault` values on a timer instead of reading real sensors. It represents the software boundary where equipment readings become ROS messages. Connecting real equipment would mean replacing that simulated input with a hardware-facing publisher that preserves the message contract.

**The publisher and bridge communicate at the edge.** Both run on the same Linux computer, as separate processes—or, in the demo, separate containers. The publisher sends `CellTelemetry` messages on `cell/telemetry`. The bridge subscribes to that topic and receives samples through its `on_telemetry` callback when discovery succeeds and the message type and QoS settings are compatible.

This is intended to be a machine-local handoff. Its exact transport depends on the ROS middleware and configuration; it should not be assumed to use only loopback or to remain confined to the host simply because both nodes share a ROS domain. The demo’s Compose network provides connectivity between the containers. The publisher’s role ends at ROS publication; it does not send telemetry directly to AWS.

**The bridge creates the cloud handoff.** It still runs on the edge computer. In the design described here, it collects samples until five are available or approximately one second has elapsed, groups them by `cell_id`, and sends JSON batches over HTTPS to API Gateway. This is the point where the telemetry leaves the edge computer through its outbound network connection.

AWS handles the receiving side. API Gateway invokes Lambda, which writes telemetry to DynamoDB and emits a `CELL_ALERT` log entry in CloudWatch when a fault is reported. The cloud now holds a copy of information produced at the equipment; the physical sensors and local ROS processes remain at the edge.

A factory or fleet could repeat this pattern across multiple edge computers, each identified so its telemetry can be distinguished downstream. That is a possible deployment model for P3, not a claim that fleet-wide management is already implemented. The project illustrates telemetry export and cloud-side recording—not robot motion control, safety interlocks, or guaranteed delivery during network outages.
