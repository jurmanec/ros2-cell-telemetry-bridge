# Docker Fundamentals: Images, Containers, Compose, and Networks

A clear guide to the core concepts of Docker, based on common questions about how images, containers, `docker run`, Docker Compose, and networks fit together.

---

## 1. Images vs. Containers

### Images
An **image** is a static, immutable blueprint. It contains:
- A layered filesystem
- Metadata (default command, environment variables, exposed ports, etc.)

Images are built once and can be reused many times. You typically pull them from a registry (Docker Hub, etc.) or build them yourself from a Dockerfile.

### Containers
A **container** is a running (or stopped) instance of an image. It is:
- A lightweight, isolated process
- Has its own filesystem, network interface, and process space
- Shares the host’s kernel (unlike a virtual machine)
- Starts in seconds and uses relatively little overhead

**Key relationship**:
```
Image (blueprint)  →  Container (running instance)
```

You can create many containers from the same image. Changes inside a container do not affect the original image.

| Concept     | Analogy              | Nature          | Primary actions              |
|-------------|----------------------|-----------------|------------------------------|
| **Image**   | Blueprint / class    | Static template | Build, pull, push, tag       |
| **Container** | Running house / object | Live process | Run, stop, start, remove, exec |

---

## 2. Building a Custom Image vs. Using an Existing Image

### Building a Custom Image
You create a `Dockerfile` that defines the environment, then build it:

```dockerfile
FROM node:20-alpine
WORKDIR /app
COPY package*.json ./
RUN npm ci --only=production
COPY . .
EXPOSE 3000
CMD ["node", "server.js"]
```

```bash
docker build -t myapp:1.0 .
```

**Use when**:
- Your application has unique dependencies or code
- You need a consistent, reproducible runtime
- You want control over security, size, or layer optimization

### Using an Existing Image
Many services already have high-quality official images (nginx, postgres, redis, etc.). You simply reference them.

**Use when**:
- A well-maintained public image already exists
- You mainly need configuration (ports, volumes, environment variables) rather than custom software

### Using Both Together
Docker Compose can mix both approaches in a single file:

```yaml
services:
  app:
    build: .              # builds a custom image from a local Dockerfile
    ports:
      - "3000:3000"
  db:
    image: postgres:16    # uses an existing official image
```

---

## 3. `docker run` vs. Docker Compose

Both tools create containers from images. The difference is scope and style.

### `docker run` — Single Container (Imperative)
Everything is specified on the command line:

```bash
docker run -d \
  --name my-web \
  -p 8080:80 \
  -v ./html:/usr/share/nginx/html \
  -e NGINX_HOST=example.com \
  --restart unless-stopped \
  nginx:alpine
```

**Characteristics**:
- One container per command
- Configuration via flags
- Low reproducibility (you must remember or script the full command)
- Manual networking and volume management
- Best for quick tests or simple one-off containers

### `--rm` — run one command, then remove the container

`--rm` starts a container from an image, runs a single command, and deletes that container when the command exits. The image stays on disk. Use it when you want an answer from inside the image and do not need the container afterward. Without `--rm`, the same command leaves a stopped container that still appears in `docker ps -a`.

Check that `p3-edge:jazzy` contains the historian executable:

```bash
docker run --rm p3-edge:jazzy ros2 pkg executables cell_historian
```

The image entrypoint sources ROS 2, then runs that `ros2` command. When the command finishes, Docker removes the container. `p3-edge:jazzy` is unchanged. A line `cell_historian weld_historian_node` means the image includes the historian. See [cleaning-up.md](cleaning-up.md) for when a leftover container should be removed by hand.

### Docker Compose — Multi-Container Application (Declarative)
You describe the full application in a `docker-compose.yml` file:

```yaml
services:
  web:
    image: nginx:alpine
    ports:
      - "8080:80"
    volumes:
      - ./html:/usr/share/nginx/html
    environment:
      NGINX_HOST: example.com
    restart: unless-stopped

  app:
    build: .
    ports:
      - "3000:3000"
    depends_on:
      - db

  db:
    image: postgres:16
    environment:
      POSTGRES_PASSWORD: secret
    volumes:
      - pgdata:/var/lib/postgresql/data

volumes:
  pgdata:
```

```bash
docker compose up -d      # start the entire application
docker compose down       # stop and clean up
docker compose logs -f    # view logs from all services
```

**Characteristics**:
- One file defines the whole stack
- Automatic networking and service discovery
- Easy to version-control and share
- Supports both `image:` and `build:`
- Best for real applications, teams, and reproducible environments

### Comparison

| Aspect              | `docker run`                          | Docker Compose                              |
|---------------------|---------------------------------------|---------------------------------------------|
| Scope               | Single container                      | Multi-container application                 |
| Configuration       | Command-line flags                    | YAML file                                   |
| Reproducibility     | Low                                   | High                                        |
| Networking          | Manual                                | Automatic                                   |
| Best for            | Quick tests, simple tools             | Multi-service apps, teams, local dev        |

---

## 4. Docker Networks

Networks are the virtual communication layer that connects containers. They are applied at runtime and are independent of the image itself.

### Core Behavior
- Containers on the **same network** can communicate with each other.
- Containers on **different networks** are isolated by default.
- Publishing ports (`-p` or `ports:`) exposes a container to the host/internet and is separate from container-to-container communication.

### With `docker run`
By default, containers join the `bridge` network (limited name resolution). For better isolation and DNS:

```bash
docker network create myapp-net

docker run -d --name web --network myapp-net nginx:alpine
docker run -d --name app --network myapp-net myapp:latest
```

Now the containers can reach each other by name (`web`, `app`).

### With Docker Compose
Compose automatically:
1. Creates a dedicated network for the project
2. Attaches every service to it
3. Provides DNS so services can reach each other by service name (e.g., the app connects to the database at hostname `db`)

No extra network commands are required for typical use.

### Summary of Relationships

| Concept          | Relationship to Networks                              |
|------------------|-------------------------------------------------------|
| **Image**        | Contains no network configuration                     |
| **Container**    | Is attached to one or more networks at startup        |
| **`docker run`** | Requires explicit network creation and attachment     |
| **Compose**      | Automatically creates a project network + DNS         |

---

## Quick Reference

```
Dockerfile  →  docker build  →  Image  →  docker run / docker compose  →  Container(s)
                                                              ↓
                                                         Networks connect them
```

- **Build an image** when you need a custom environment for your application.
- **Use existing images** for standard services (databases, proxies, caches).
- **Use `docker run`** for single containers and quick experiments. Add `--rm` when the container should disappear as soon as the command exits.
- **Use Docker Compose** for multi-service applications and reproducible setups.
- **Networks** are the plumbing that lets containers talk to each other; Compose handles them automatically.
- **Remove containers** when a demo is finished. **Leave images** such as `p3-edge:jazzy` so the next run does not rebuild ROS. See [cleaning-up.md](cleaning-up.md).

---

*Document generated from a discussion covering Docker images, containers, `docker run`, Docker Compose, and networking.*
