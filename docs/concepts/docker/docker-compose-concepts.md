# Docker Compose Concepts Explained

A clear explanation of three important Docker Compose concepts for a service: `env_file`, `volumes`, and `depends_on` with `condition: service_healthy`.

## 1. `env_file`

**What it does:** Loads environment variables from one or more files into the container.

**Syntax:**
```yaml
services:
  myapp:
    image: myapp:latest
    env_file:
      - .env
      - ./config/production.env
```

**Key points:**
- Variables from `env_file` are injected into the container’s environment.
- Values defined under the `environment:` key **override** anything from `env_file`.
- Useful for keeping secrets and config out of the Compose file itself.
- Relative paths are resolved from the Compose file’s directory.

**Precedence (highest to lowest):**
1. `environment:` in the service
2. `env_file`
3. Variables already set in the image (`ENV` instruction)

---

## 2. `volumes`

**What it does:** Mounts data into the container. This is how you persist data or share files between host and container (or between containers).

**Common forms:**

```yaml
services:
  myapp:
    image: myapp:latest
    volumes:
      # Bind mount (host path → container path)
      - ./app:/usr/src/app
      - /var/log/myapp:/var/log/app:ro   # read-only

      # Named volume (managed by Docker)
      - db_data:/var/lib/postgresql/data

      # Anonymous volume
      - /tmp/cache
```

**Types:**

| Type              | Example                     | Persistence          | Typical use                     |
|-------------------|-----------------------------|----------------------|---------------------------------|
| Bind mount        | `./data:/app/data`          | On the host         | Source code, configs            |
| Named volume      | `db_data:/var/lib/mysql`    | Managed by Docker   | Databases, durable data         |
| Anonymous volume  | `/container/path`           | Until container removed | Temporary/cache data         |

You can also use the long syntax for more control (permissions, propagation, etc.).

---

## 3. `depends_on` → `condition: service_healthy`

**What it does:** Controls startup order. The long form with `condition: service_healthy` makes Compose **wait until the dependency is actually healthy** before starting this service.

**Important distinction:**
- Short form (`depends_on: [db]`) only waits for the container to **start**.
- Long form with `service_healthy` waits for the dependency’s **healthcheck** to pass.

**Example (recommended pattern for databases):**

```yaml
services:
  web:
    build: .
    depends_on:
      db:
        condition: service_healthy
        # optional: restart this service if db is recreated
        restart: true

  db:
    image: postgres:16
    environment:
      POSTGRES_USER: app
      POSTGRES_PASSWORD: secret
      POSTGRES_DB: appdb
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U $${POSTGRES_USER} -d $${POSTGRES_DB}"]
      interval: 10s
      timeout: 5s
      retries: 5
      start_period: 30s
    volumes:
      - db_data:/var/lib/postgresql/data

volumes:
  db_data:
```

**Available conditions:**

| Condition                       | Meaning                                              |
|---------------------------------|------------------------------------------------------|
| `service_started`               | Container has started (same as short syntax)         |
| `service_healthy`               | Healthcheck reports healthy                          |
| `service_completed_successfully`| Container exited with code 0 (good for migrations)   |

**Notes:**
- The dependency **must** define a `healthcheck`, otherwise `service_healthy` will never succeed.
- This is the correct way to avoid the classic “app starts before Postgres is ready” race condition.
- `restart: true` (Compose ≥ 2.17) tells Compose to restart the dependent service if the dependency is recreated.

---

## Quick combined example

```yaml
services:
  api:
    image: myapi:latest
    env_file: .env
    volumes:
      - ./uploads:/app/uploads
    depends_on:
      db:
        condition: service_healthy

  db:
    image: postgres:16
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U postgres"]
      interval: 5s
      retries: 5
    volumes:
      - pgdata:/var/lib/postgresql/data

volumes:
  pgdata:
```

These three features together cover configuration (`env_file`), data persistence (`volumes`), and reliable startup ordering (`depends_on` + healthchecks).
