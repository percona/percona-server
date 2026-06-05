if ($MTR_OIDC_IDP_HOST)
{
  --let $IDP_HOST = $MTR_OIDC_IDP_HOST
}

if (!$IDP_HOST)
{
  --let $IDP_HOST = https://keycloak.int.percona.com
}

if ($MTR_OIDC_USER_ID)
{
  --let $IDP_USER_ID = $MTR_USER_ID
}

if (!$IDP_USER_ID)
{
  --let $IDP_USER_ID = e3039939-719c-4ba7-99d9-d9efecf5caeb
}