## HOW TO run KMIP test suite

The [PyKMIP](https://github.com/OpenKMIP/PyKMIP) server is used to run this test suite. It could be run locally or in the Docker. The source of the docker  image is [here](https://github.com/Percona-Lab/pxb-jenkins-images).

This is simple script to run this image:
```
#!/bin/bash
CERTS_DIR=/tmp/certs
docker stop kmip
docker run -d --security-opt seccomp=unconfined --cap-add=NET_ADMIN --rm -p 5696:5696  --name kmip satyapercona/kmip:latest
mkdir /tmp/certs
docker cp kmip:/opt/certs/root_certificate.pem $CERTS_DIR/vault-kmip-ca.pem
docker cp kmip:/opt/certs/client_key_jane_doe.pem $CERTS_DIR/mysql-client-key.pem
docker cp kmip:/opt/certs/client_certificate_jane_doe.pem $CERTS_DIR/mysql-client-cert.pem
```

Please note, that certificates and keys are copied to the certain location and renamed.This is important, because some tests from the suite are sensitive to that names.

Before running the test suite, the following environment variables should be exported:

```
CERTS_DIR=/tmp/certs
export  KMIP_ADDR="127.0.0.1"
export  KMIP_PORT="5696"
export  KMIP_CLIENT_CA="$CERTS_DIR/mysql-client-cert.pem"
export  KMIP_CLIENT_KEY="$CERTS_DIR/mysql-client-key.pem"
export  KMIP_SERVER_CA="$CERTS_DIR/vault-kmip-ca.pem"
```
Now you can run KMIP keyring tests:

```
./mtr --big-test --suite component_keyring_kmip
```
If you want to run the test with other KMIP server, please note the following:
1. Copy keys and certificates to ```/tmp/certs``` directory with names as above and export variables.
2. Some servers does not support RSA-512 keys used by all keyring tests, so you can have some failing tests.
3. To setup KMIP servers, please refer to [this document](https://www.notion.so/percona/KMIP-Testing-1fb674d091f3809cbfbdf31369e22d35).