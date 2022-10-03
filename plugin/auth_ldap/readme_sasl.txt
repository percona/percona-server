The MySQL LDAP SASL authentication isn't that trivial.
This readme tries to explain how it works at a high level.

The LDAP library has two high level methods for SASL authentication:

* ldap_sasl_interactive_bind(_s), and
* ldap_sasl_bind(_s)

Most examples on the internet use the ldap_sasl_interactive_s method, because
that's the higher level "do all magic" method. It deals with the SASL library,
executes all steps required for the specified SASL authentication method, and
allows specifying the callback to provide the required inputs (e.g. usernames
and passwords).

It also makes the assumption that we only have one client and one (ldap) server
machine, which isn't neccessarily true with MySQL: we can have a separate mysql
client machine, a separate mysql server machine, and a separate ldap server.

Because of this, MySQL has to use the more complex ldap_sasl_bind_s.

ldap_sasl_bind_s only performs one (of the possibly many) authentication steps.
It either retuns failure, success, or "in-progress" (more steps needed).

ldap_sasl_bind_s doesn't deal with libsasl: all SASL calls have to be done by 
the user code. The idea is that the SASL library responses are sent to the LDAP
library using the cred parameter, and the LDAP library responses are given 
using the last out parameter (servercredp). This out parameter has to be
specified as the input parameter for the next SASL library call, which again
sends another response...

In our case, the SASL communication is already implemented: the client
plugin calls the SASL library, and sends its responses to the server plugin.
The server plugin has to send this data to the LDAP library, and forward the
library's response to the client plugin. The client plugin performs the next
SASL step using this response as the input, and sends the data for the next
LDAP step...

In theory, SASL supports many authentication methods. In practice the MySQL
client side plugin only supports SCRAM-SHA-1 and GSSAPI (Kerberos). 
Currently the PS server side plugin is only tested with SCRAM-SHA-1, and 
because of this, it is hardcoded into the plugin. In the future we should
try to test it with GSSAPI, and enable that as an option (system variable).


Test setup
====

There is an MTR testcase in the tests directory. The testcase requires an
LDAP server with SASL configured. There are no simple docker images or example
servers with this setup, so it has to be created manually.

The tests directory also contains two ldif files: one for the user, and one for
enabling the SCRAM-SHA-1 authentication method.

After adding these files with ldapmodify, a simple test can be executed with the
following command:

ldapsearch -LLL -H ldapi:/// -s "base" -b "" supportedSASLMechanisms -Y SCRAM-SHA-1 -U john3

The password is "secret". If authentication succeeds, and it prints a list of the
authentication methods supported by the server, the MTR testcase should work too.

