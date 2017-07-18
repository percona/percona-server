#!/bin/sh

set -xe

# boilerplace for "openssl ca" and /etc/ssl/openssl.cnf
rm -rf demoCA
mkdir demoCA demoCA/newcerts
touch demoCA/index.txt
echo 01 > demoCA/serial

# Generate a new self-signed CA certificate
openssl req -x509 -newkey rsa:2048 -keyout percona-cakey.pem -out percona-cacert.pem -days 7300 -nodes -subj '/CN=percona-cacert/C=US/ST=NC/L=Raleigh/O=Percona' -text
# The following operation althought looking redundant will remove 'passphrase' from the RSA key (which is currently unsupported in YaSSL)
openssl rsa -in percona-cakey.pem -out percona-cakey.pem

# Generating a new certificate signing request for the server
openssl req -newkey rsa:2048 -keyout percona-serversan-key.pem -out demoCA/percona-serversan.csr -days 7300 -nodes -subj '/CN=percona-server/C=US/ST=NC/L=Raleigh/O=Percona'
# The following operation althought looking redundant will remove 'passphrase' from the RSA key (which is currently unsupported in YaSSL)
openssl rsa -in percona-serversan-key.pem -out percona-serversan-key.pem
# Generate a new server certificate signed with new CA
# with SubjectAltName, only for OpenSSL 1.0.2+
cat > demoCA/sanext.conf <<EOF
subjectAltName = @alt_names
[ alt_names ]
DNS.1 = localhost
DNS.2 = localhost.localdomain
IP.1 = 127.0.0.1
IP.2 = ::1
EOF
openssl ca -keyfile percona-cakey.pem -extfile demoCA/sanext.conf -days 7300 -batch -cert percona-cacert.pem -policy policy_anything -out percona-serversan-cert.pem -infiles demoCA/percona-serversan.csr
