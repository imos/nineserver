#!/bin/bash

set -e -u -x

PORT="$(./nineserver/tools/free_port)"
TMPFILE="${TMPDIR:-/tmp}/scenario-${PORT}"

ToCrLf() {
  awk '{sub(/$/, "\r"); print}'
}

./nineserver/test/main --port="${PORT}" --alsologtostderr &
sleep 0.5

################################################################################
# /_/null should always return OK.
################################################################################
ToCrLf <<'EOM' | nc 127.0.0.1 "${PORT}" | grep -v ^X-Profile >"${TMPFILE}"
GET /_/null HTTP/1.1
User-Agent: curl/7.37.1
Host: localhost:8081
Accept: */*
Connection: close

EOM

cat <<'EOM' | diff --strip-trailing-cr - "${TMPFILE}"
HTTP/1.1 200 OK
Content-Type: text/plain; charset=UTF-8
Connection: close
Content-Length: 3

OK
EOM

################################################################################
# /_/undefined should return 404.
################################################################################
ToCrLf <<'EOM' | nc 127.0.0.1 "${PORT}" | grep -v ^X-Profile >"${TMPFILE}"
GET /_/undefined HTTP/1.1
User-Agent: curl/7.37.1
Host: localhost:8081
Accept: */*
Connection: close

EOM

cat <<'EOM' | diff --strip-trailing-cr - "${TMPFILE}"
HTTP/1.1 404 Not Found
Content-Type: text/plain; charset=UTF-8
Connection: close
Content-Length: 34

No such system handler: undefined
EOM

################################################################################
# /_/undefined should return 404.
################################################################################
ToCrLf <<'EOM' | nc 127.0.0.1 "${PORT}" | grep -v ^X-Profile >"${TMPFILE}"
GET /_/undefined HTTP/1.1
User-Agent: curl/7.37.1
Host: localhost:8081
Accept: */*
Connection: close

EOM

cat <<'EOM' | diff --strip-trailing-cr - "${TMPFILE}"
HTTP/1.1 404 Not Found
Content-Type: text/plain; charset=UTF-8
Connection: close
Content-Length: 34

No such system handler: undefined
EOM

################################################################################
# Test parameters
################################################################################
curl "http://localhost:${PORT}/_/var?key=query_parameters&foo=bar&%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A=%E3%81%8B%E3%81%8D%E3%81%8F%E3%81%91%E3%81%93" > "${TMPFILE}"
echo -n '{"foo":["bar"],"key":["query_parameters"],"あいうえお":["かきくけこ"]}' | diff - "${TMPFILE}"

curl --data-urlencode 'foo=bar' --data-urlencode 'あいうえお=かきくけこ' \
    "http://localhost:${PORT}/_/var?key=post_parameters" > "${TMPFILE}"
echo -n '{"foo":["bar"],"あいうえお":["かきくけこ"]}' | diff - "${TMPFILE}"

curl -F 'foo=bar' -F 'あいうえお=かきくけこ' \
    "http://localhost:${PORT}/_/var?key=file_parameters" > "${TMPFILE}"
echo -n '{"foo":{"Content-Body":["bar"],"Content-Disposition":["form-data; name=\"foo\""]},"あいうえお":{"Content-Body":["かきくけこ"],"Content-Disposition":["form-data; name=\"あいうえお\""]}}' | diff - "${TMPFILE}"

curl -c - "http://localhost:${PORT}/_/cookie?key=foo&value=bar" | \
curl -b - -c - \
    "http://localhost:${PORT}/_/cookie?key=あいうえお&value=かきくけこ" | \
curl -b - "http://localhost:${PORT}/_/var?key=cookie_parameters" \
    > "${TMPFILE}"
echo -n '{"foo":["bar"],"あいうえお":["かきくけこ"]}' | diff - "${TMPFILE}"

################################################################################
# ApachBench should work.
################################################################################
ab -n 1000 -c 10 -k "http://127.0.0.1:${PORT}/_/null" 2>/dev/null | \
    grep 'Complete requests' | grep 1000
ab -n 1000 -c 10 "http://127.0.0.1:${PORT}/_/null" 2>/dev/null | \
    grep 'Complete requests' | grep 1000

################################################################################
# Quit the server.
################################################################################
curl "http://127.0.0.1:${PORT}/_/quit"

wait
rm "${TMPFILE}"
