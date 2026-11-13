# Cost — facility demo writes to DynamoDB

**Date:** 2026-11-13  
**Prices:** [DynamoDB on-demand](https://aws.amazon.com/dynamodb/pricing/), Standard table, `us-east-1`. Write request units were **$0.625 per million**. Confirm the page before treating these figures as a bill.

The facility demo publishes three snapshots a second. DynamoDB does not receive all three. The bridge posts every fifth snapshot per asset, and every snapshot that carries an alarm. See [facility-demo.md](../how-to/facility-demo.md).

| While the demo is running | Writes per second | Writes in a full month | DynamoDB writes |
|---|---|---|---|
| All three assets healthy | 0.6 | about 1.6 million | about $1 |
| `CRAH-01` and `ROOM-01` latched `FAULTED`, `UPS-01` still every fifth | 2.2 | about 5.7 million | about $4 |
| Every publisher tick, no reduction | 3 | about 7.8 million | about $5 |

A facility item is under 1 KB, so one post is one write request unit. The table is on-demand (`PAY_PER_REQUEST` in `cloud/serverless.yml`). The first 25 GB of Standard storage is not charged. This demo stays inside that.

The same posts also hit API Gateway, at about $1 per million requests. That is the same order of cost as the table.

A stopped demo writes nothing. A run of half an hour is cents. The rate that adds up is leaving the demo up after a fault has latched, because the cooling unit and the room then post every second instead of every fifth. That is still a few dollars for a month of continuous running.
