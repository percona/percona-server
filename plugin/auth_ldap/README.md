## Notes:
- The server certificate won't be validated unless ca_path is set.
- TLS won't work over the SSL port ()

## Oracle Enterpise differences
We can define combinations of LDAP groups for one proxy account

```
#ad_group_1+ad_group_2=mysql_proxy_1
```
This means that the user must be member of ad_group_1 and ad_group_2 in LDAP to proxy with mysql_proxy_1
