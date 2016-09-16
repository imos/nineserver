#!/bin/bash

set -e -u

PORT="$(./nineserver/tools/free_port)"
TMPFILE="${TMPDIR:-/tmp}/scenario-${PORT}"

./*/main --port="${PORT}" --alsologtostderr &

sleep 0.1
echo 'Benchmark result:' $(
    for i in `seq 500`; do
      echo $(
          ab -n 500 -c 10 -k "http://127.0.0.1:${PORT}/_/null" 2>/dev/null |
          grep Requests | cut -d: -f2 | cut -d. -f1
      )
    done | sort -n | tail -n 1) 'QPS'

################################################################################
# Quit the server.
################################################################################
curl --silent "http://127.0.0.1:${PORT}/_/quit" >/dev/null

wait
