FROM percona/percona-server:8.0

USER root

# Copy plugin compiled against Percona Server 8.0 headers (ABI compatible)
COPY authentication_appkey.so /usr/lib64/mysql/plugin/authentication_appkey.so
RUN chown mysql:mysql /usr/lib64/mysql/plugin/authentication_appkey.so

# Percona Server uses /etc/my.cnf.d/ for config includes
COPY my.cnf /etc/my.cnf.d/appkey.cnf

COPY init.sql /docker-entrypoint-initdb.d/01-appkey.sql

# Switch back to mysql user for server process
USER mysql
