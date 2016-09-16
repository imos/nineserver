#!/bin/bash

set -e -x

echo -n > /etc/sudoers.d/isucon-users
for user in takiba imos okuta; do
  adduser $user
  mkdir -p /home/$user/.ssh
  curl -o /home/$user/.ssh/authorized_keys http://imoz.jp/private/isucon.pub
  echo "$user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/isucon-users
done

chmod 666 /var/log/nginx/access.log
chmod 666 /var/log/nginx/error.log
chmod 755 /var/lib/nginx
chmod 755 /var/log/nginx
chmod 777 /var/lib/nginx/tmp
chmod 777 /var/lib/nginx/tmp/client_body

if [ ! -x /usr/local/bin/bazel ]; then
  if [ -f /usr/local/bin/bazel.gz ]; then
    rm -f /usr/local/bin/bazel.gz
  fi
  if which yum 2>/dev/null; then
    curl -o /usr/local/bin/bazel.gz \
        'https://storage.googleapis.com/archive-imoz-jp/Repository/Bazel/bazel-0.3.0-centos.gz'
  else
    curl -o /usr/local/bin/bazel.gz \
        'https://storage.googleapis.com/archive-imoz-jp/Repository/Bazel/bazel-0.3.0.gz'
  fi
  gunzip /usr/local/bin/bazel.gz
  chmod +x /usr/local/bin/bazel
fi

if which yum 2>/dev/null; then
  yum update
  yum install java-1.8.0-openjdk-devel.x86_64 gcc48-c++.x86_64 httpd-tools
  if [ ! -e /usr/lib64/libmysqlclient.so ]; then
    ln -s /usr/lib64/mysql/libmysqlclient.so.18 /usr/lib64/libmysqlclient.so
  fi
else
  echo oracle-java8-installer shared/accepted-oracle-license-v1-1 \
      select true | debconf-set-selections
  add-apt-repository ppa:webupd8team/java
  add-apt-repository ppa:ubuntu-toolchain-r/test
  apt-get update
  apt-get install -y \
      oracle-java8-installer oracle-java8-set-default \
      git zip make libfcgi-dev google-perftools g++-4.9 apache2-utils
fi

# update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-4.9 100
# update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-4.9 100
# update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 100
# update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 100

cat <<'EOM' > /etc/sysctl.conf
net.core.netdev_max_backlog=32768
net.core.rmem_max = 16777216
net.core.somaxconn=32768
net.core.wmem_max = 16777216
net.ipv4.ip_local_port_range= 10000 65535
net.ipv4.tcp_fin_timeout=10
net.ipv4.tcp_max_syn_backlog=32768
net.ipv4.tcp_rmem = 4096 349520 16777216
net.ipv4.tcp_timestamps = 0
net.ipv4.tcp_tw_recycle=1
net.ipv4.tcp_tw_reuse=1
net.ipv4.tcp_wmem = 4096 65536 16777216
net.ipv4.tcp_rfc1337=1
net.ipv4.tcp_keepalive_probes=5
net.ipv4.tcp_slow_start_after_idle=0
net.core.somaxconn=65535
EOM
sysctl -p
