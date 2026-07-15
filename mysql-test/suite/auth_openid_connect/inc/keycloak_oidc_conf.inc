let $KEYCLOAK_OIDC_CONF =
JSON://
{
  \"oidc-idp\": {
    \"issuer-name\": \"https://idp-test.com/realms/dummy\",
    \"keys\": [
      {
        \"kid\": \"rsa-key-1\",
        \"kty\": \"RSA\",
        \"n\": \"ptR4YxjdrF2RrYiY9XYH3KcXKzlS6b2foGAeHN9dViAs5yKStKAucUf81kszq6LkzkOOPX5SbToLBRv5wwOtrBLKVkqhw4i5kS0b-7jpR9g7lER3qfDVYfe0nneaJzc-1Iz6id0oVj4ay8lGmWg5JX5Z8nBs52_7rfDIBtyAsyIQ2dF2tW4e72cypvskw_TwdbsqelY5ZezzTmKx8Ra4-o9Pu5_Vxe1Hhlkx1K_tfbGH9ZUsbFy90b0Vcw5w6EKPEkM9mZTUoj8TMeHTHaiOREK4llIJ6hokXbEG8BsKJuMuYrzx98e23JkaWaWQY27LUGK5sx-lsX_n8DO2kEASRw\",
        \"e\": \"AQAB\",
        \"use\": \"sig\",
        \"alg\": \"RS256\"
      },
      {
        \"kty\": \"EC\",
        \"kid\": \"ec-key-1\",
        \"use\": \"sig\",
        \"alg\": \"ES256\",
        \"crv\": \"P-256\",
        \"x\": \"bjQTXrTcw_1HKiiZm2Hqv41w7Vd44M9koyY_-VsP-SA\",
        \"y\": \"XqAzBfS0uQQwoemIKhNw4x8FsJxChCN1qT3_IsxMda0\"
      }
    ],
    \"audiences\": [
      \"ee2811b9-10b8\",
      \"https://api.example.com\"
    ],
    \"group-claim\": \"groups\",
    \"group-role\": [
      {
        \"acc\": \"accounting\"
      },
      {
        \"eng\": \"engineering\"
      }
    ]
  },
  \"my-keycloak\": {
    \"issuer-name\": \"$IDP_HOST/realms/master\",
    \"jwks-url\": \"$IDP_HOST/realms/master/protocol/openid-connect/certs\",
    \"audiences\": [
      \"myclient\"
    ],
    \"group-claim\": \"groups\",
    \"group-role\": [
      {
        \"/accounting\": \"accounting\"
      },
      {
        \"/marketing\": \"marketing\"
      }
    ]
  },
  \"unaccessible-idp\": {
    \"issuer-name\": \"https://dummy-host/realms/master\",
    \"jwks-url\": \"https://dummy-host/realms/master/protocol/openid-connect/certs\",
    \"audiences\": [
      \"account\"
    ]
  }
};

