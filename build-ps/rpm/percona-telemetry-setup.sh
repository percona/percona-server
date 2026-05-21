#!/bin/sh
# Setup Percona telemetry directory with proper ownership and SELinux context
PS_TELEMETRY=/usr/local/percona/telemetry/ps

mkdir -p "$PS_TELEMETRY"
chown mysql:percona-telemetry "$PS_TELEMETRY" 2>/dev/null || :
chmod 775 "$PS_TELEMETRY" 2>/dev/null || :
chmod g+s "$PS_TELEMETRY" 2>/dev/null || :
chmod u+s "$PS_TELEMETRY" 2>/dev/null || :
chcon -t mysqld_db_t "$PS_TELEMETRY" 2>/dev/null || :
chcon -u system_u "$PS_TELEMETRY" 2>/dev/null || :

# Setup telemetry UUID file
chgrp percona-telemetry /usr/local/percona/telemetry_uuid 2>/dev/null || :
chmod 664 /usr/local/percona/telemetry_uuid 2>/dev/null || :
