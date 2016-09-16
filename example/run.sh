#!/bin/bash
# run.sh

export PATH="$(pwd)/bin:${PATH}"

source imosh || exit 1

DEFINE_int v 0 'Verbosity level.'
DEFINE_int port 8080 'Port number to listen.'
DEFINE_bool --alias=n dry_run false 'Dry run.'

DEFINE_int --group=php php_port 0 'Port for PHP-FPM.'

DEFINE_int --group=nginx nginx_port 8000 'Port.'
DEFINE_int --group=nginx nginx_worker_processes 4 '# of worker processes.'
DEFINE_int --group=nginx nginx_worker_connections 2048 \
    '# of worker connections.'

eval "${IMOSH_INIT}"

set -e -u

MAIN="$(echo */main)"
ROOT_DIRECTORY="${PWD}/${MAIN%/main}"

PHP_PATH="/tmp/$(sub::time).$$.php-fpm.sock"
sub::atexit "rm -f ${PHP_PATH} || true"

generate_nginx_conf() {
  echo "worker_processes ${FLAGS_nginx_worker_processes};"
  echo 'daemon off;'
  echo 'error_log /dev/stderr debug;'
  echo 'events {'
  {
    echo "worker_connections ${FLAGS_nginx_worker_connections};"
    echo 'multi_accept on;'
  }
  echo '}'

  echo 'http {'
  {
    cat "${ROOT_DIRECTORY}/config/mime.types"
    echo 'default_type application/octet-stream;'
    echo 'sendfile on;'
    echo 'keepalive_timeout 120;'

    echo 'upstream fcgi {'
    {
      if (( FLAGS_php_port == 0 )); then
        echo "server unix:${PHP_PATH};"
      else
        echo "server 127.0.0.1:${FLAGS_php_port};"
      fi
    }
    echo '}'

    echo 'server {'
    {
      if (( FLAGS_nginx_port == 0 )); then
        echo "listen unix:${PROXY_SOCKET};"
      else
        echo "listen ${FLAGS_nginx_port};"
      fi
      echo 'server_name localhost;'
      # echo 'root data/www;'
      echo "root ${ROOT_DIRECTORY}/data/nginx;"

      # Matches static files.
      echo 'location ~ .*\.(css|js|ico|html|jpg)$ {}'

      echo 'location ~ [^/]\.php$ {'
      {
        echo 'fastcgi_pass fcgi;'
        cat "${ROOT_DIRECTORY}/config/fcgi.conf"
      }
      echo '}'
    }
    echo '}'
  }
  echo '}'
}

run_nginx() {
  local tmpfile
  func::tmpfile tmpfile
  generate_nginx_conf > "${tmpfile}"
  nginx -p "${ROOT_DIRECTORY}" -c "${tmpfile}"
}

generate_fpm() {
  echo 'daemonize = no'
  echo 'error_log = /dev/stderr'
  echo '[www]'
  echo 'pm = static'
  if (( FLAGS_php_port == 0 )); then
    echo "listen = ${PHP_PATH}"
  else
    echo "listen = 0.0.0.0:${FLAGS_php_port}"
  fi
  echo 'pm.max_children = 8'
  echo 'pm.process_idle_timeout = 10s'
  echo 'pm.max_requests = 0'
  echo 'request_terminate_timeout = 10'
  echo 'pm.status_path = /status'
  echo 'ping.path = /ping'
  echo 'catch_workers_output = yes'
  echo 'php_flag[display_errors] = on'
  echo 'php_admin_flag[log_errors] = on'
}

generate_php_ini() {
  echo 'error_log = /dev/stderr'
  echo 'date.timezone = "Asia/Tokyo"'
}

run_fpm() {
  local php_fpm_conf
  func::tmpfile php_fpm_conf
  generate_fpm > "${php_fpm_conf}"

  local php_ini_conf
  func::tmpfile php_ini_conf
  generate_php_ini > "${php_ini_conf}"

  php-fpm -F -n -y "${php_fpm_conf}" -c "${php_ini_conf}"
}

COMMAND=(
    "${MAIN}"
    --port="${FLAGS_port}"
    --alsologtostderr
    --v="${FLAGS_v}")

if (( FLAGS_nginx_port == 0 )); then
  PROXY_SOCKET="$(sub::tmpfile).socket"
  COMMAND+=(--proxy="unix://${PROXY_SOCKET}")
else
  COMMAND+=(--proxy="tcp://127.0.0.1:${FLAGS_nginx_port}")
fi

if (( FLAGS_dry_run )); then
  echo '======================================================================='
  generate_nginx_conf
  echo '======================================================================='
  generate_fpm
  echo '======================================================================='
  generate_php_ini
  echo '======================================================================='
  echo "${COMMAND[@]}"
else
  run_nginx &
  run_fpm &
  "${COMMAND[@]}"
  sub::exit
fi
