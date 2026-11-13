#!/bin/sh
# Runs only when influxdb-data is first created, after the weld bucket exists.
# An existing volume does not re-run this script. Create the bucket there with:
#   influx bucket create --name facility --org p3 --retention 7d --token "$DOCKER_INFLUXDB_INIT_ADMIN_TOKEN"
set -e
influx bucket create \
  --name facility \
  --org "${DOCKER_INFLUXDB_INIT_ORG}" \
  --retention 7d \
  --token "${DOCKER_INFLUXDB_INIT_ADMIN_TOKEN}"
