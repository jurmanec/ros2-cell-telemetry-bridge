# InfluxDB Data Model: Fields, Tags, Measurements, and Time Series

This document explains the core concepts of the InfluxDB data model, with a focus on **fields** and how they relate to time series.

## Core Concept: Time Series

InfluxDB is a **time series database**. The fundamental unit of data is a **time series** — a sequence of data points ordered by time.

A single, unique time series is identified by the combination of:

1. **Measurement** name
2. **Tag set** (all tag key-value pairs)
3. **Field key**

## The Four Components of a Data Point

Every point written to InfluxDB consists of four distinct parts:

| Component   | Role                                      | Indexed?     | Example                  |
|-------------|-------------------------------------------|--------------|--------------------------|
| **Measurement** | High-level name / category of the data | —            | `weld_telemetry`        |
| **Tags**        | Metadata that identifies the series    | Yes          | `cell=WELD-01`          |
| **Fields**      | The actual measured values             | No           | `current=142.5`         |
| **Timestamp**   | When the point was recorded            | Yes (primary)| `1731060000000000000`   |

### Timestamp Is Not a Field

The **timestamp** is a special, separate component. It is neither a field nor a tag. It serves as the primary time index for the data point.

In query results (Flux), you will see it as the `_time` column.

## What a Measurement Is (and Is Not)

A **measurement** is **not** an instance of a time series.

It is more like a **table name** or a high-level category that groups related time series (e.g., `temperature`, `weld_current`, `cpu`).

Under a single measurement you can have **many** individual time series, differentiated by their tags and fields.

### Example

```text
weld_telemetry,cell=WELD-01,station=A current=142.5,voltage=24.1 1731060000000000000
weld_telemetry,cell=WELD-02,station=A current=138.2,voltage=23.9 1731060000000000000
weld_telemetry,cell=WELD-01,station=B current=151.0,voltage=24.3 1731060001000000000
```

- **Measurement**: `weld_telemetry`
- **Tags**: `cell`, `station`
- **Fields**: `current`, `voltage`
- Each unique combination of measurement + tags + field is its own **time series**.

## How to Think About Fields

**Fields** are the actual measured values you care about (numeric, string, or boolean).

### Idiomatic Usage

InfluxDB is generally designed so that you:

- View or query **one field at a time** (or a small set of related fields)
- Use the **tags** to filter, group, or distinguish which series you’re looking at

This is different from a traditional relational table or spreadsheet, where you often have many columns (fields) side-by-side in the same row for the same timestamp.

### Why This Design?

- Each field is stored and indexed independently as its own series.
- Tags are highly optimized for filtering and grouping.
- Fields are not indexed in the same way, so the engine is happiest when you ask for a specific field (or a few) rather than “give me everything.”

### Querying Multiple Fields

You are not strictly limited to one field:

- In the UI Data Explorer you can select multiple fields.
- In Flux you can query multiple fields at once.
- Results usually come back as separate rows (with `_field` and `_value` columns) rather than as columns next to each other.
- If you want a more spreadsheet-like result (fields as columns), use the `pivot()` function in Flux.

## Practical Takeaway

Think in terms of:

> “Show me `current` for `cell=WELD-01`”  
> or  
> “Show me `voltage` grouped by `station`”

rather than:

> “Show me every field for every cell in one wide table.”

You can still produce a wide view when you need it, but the system is optimized for the first approach.

---

*Document generated from discussion of InfluxDB 2.x data model concepts.*
