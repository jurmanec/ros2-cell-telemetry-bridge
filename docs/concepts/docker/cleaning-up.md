# Cleaning up containers, and when to keep images

What to remove after a demo, and what to leave in place. Companion to [docker-fundamentals.md](docker-fundamentals.md).

A **container** is one run of a program. An **image** is the blueprint that run was started from. Stopping a demo only needs the container to stop. The image is what the next run uses.

---

## After `docker compose up`

Ctrl+C stops the containers. Compose leaves them on disk in the exited state, including `weld_cell_publisher` started with `--profile weld`.

From `edge/`, remove the containers for the services you started:

```bash
docker compose --profile weld down
```

`down` removes containers and the Compose network. It does not delete images. Include `--profile weld` or the weld publisher is skipped, because that service is not part of a plain `up`.

`edge/scripts/run_publisher.sh` is different. It uses `docker run --rm`, so that container is deleted when the process exits. Nothing further is required for that script.

---

## When to remove a container

Remove it when the run is finished and you do not need its logs or filesystem.

| Situation | What to do |
|-----------|------------|
| Finished a Compose demo | `docker compose --profile weld down` from `edge/` |
| `run_publisher.sh` already exited | Nothing. `--rm` deleted the container |
| A fixed name is in the way (`p3-cell-publisher` is still listed) | `docker rm -f p3-cell-publisher` |
| You want the same demo again in a minute | Leave the stopped container. `docker compose start` or `up` reuses it |

A stopped container is a small writable layer plus a name. It is not the ROS install. Deleting it does not uninstall `p3-edge:jazzy`.

---

## When to leave the image

Leave `p3-edge:jazzy` in place. `cell_publisher`, `weld_cell_publisher`, and `cell_bridge` all start from that one image. The tag is set in `edge/docker-compose.yml` (`EDGE_IMAGE`, default `p3-edge:jazzy`).

The image is expensive to rebuild. `edge/docker/Dockerfile` starts from ROS 2 Jazzy and compiles `cell_msgs`, `cell_sim`, and `cell_bridge`. The next `docker compose up` or `./scripts/build.sh` reuses the image that is already on disk. A code change does not require deleting it first. `docker build` rebuilds the layers that changed and retags `p3-edge:jazzy`.

Delete an image only when you want the disk space back and you are willing to build again:

- A tag you created for a throwaway experiment, and nothing still runs from it.
- `p3-edge:jazzy` itself, after every container that uses it is stopped, and only if a full rebuild is acceptable.

`docker image prune` and `docker system prune` remove images that no container references. After `compose down`, `p3-edge:jazzy` can be one of those images. Do not prune it unless you intend to run `edge/scripts/build.sh` before the next demo.

---

## Quick reference

```bash
docker ps -a          # running and stopped containers
docker images         # local images, including p3-edge:jazzy
docker compose --profile weld down    # remove this project's containers
docker rm -f <name>   # remove one container
docker rmi <tag>      # remove one image; the next run must build or pull it
```
