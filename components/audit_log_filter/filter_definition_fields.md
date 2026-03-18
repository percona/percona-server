# Audit Log Filter Definition Fields

This reference lists the canonical class, event, and field names accepted by
filter-definition validation through `audit_log_filter_set_filter()`.

## Notes

- The names below are filter-definition names, not necessarily the names used by
  the JSON log formatter.
- `Field Type` reflects the type accepted by the current validator in
  `get_event_field_value_type()`.
- Some numeric-looking fields are currently validated as `string` because they
  are not explicitly typed in `get_event_field_value_type()`.
- Filter-definition validation only accepts the class names documented below.
- When `audit_log_filter.event_mode=REDUCED` (the default), only the following
  events are tracked and accepted by filter-definition validation:
  - `general`: `status`
  - `connection`: `connect`, `disconnect`, `change_user`
  - `table_access`: `read`, `insert`, `update`, `delete`
  - `message`: `internal`, `user`

  Class names that have no allowed events in REDUCED mode (`global_variable`,
  `command`, `query`, `stored_program`, `authentication`, `parse`) are rejected
  entirely. Subclass names that are not in the list above (e.g. `general/log`,
  `connection/pre_authenticate`) are also rejected during filter validation.
  At runtime, events not in the REDUCED set are silently skipped.
- Lifecycle-related records with class names `audit`, `server_startup`, and
  `server_shutdown` are not valid filter-definition targets. Startup and
  shutdown lifecycle events are ignored by the audit log filter if they are
  received.
- For `connection.connection_type`, the validator accepts numeric values `0..5`
  and the pseudo-constants `::undefined`, `::tcp/ip`, `::socket`,
  `::named_pipe`, `::ssl`, and `::shared_memory`.

## `general`

Supported events: `log`, `error`, `result`, `status`
REDUCED mode: only `status`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `general_error_code` | integer | Event error code. |
| `general_thread_id` | unsigned integer | Event thread ID. Currently an alias of `general_connection_id`. |
| `general_connection_id` | unsigned integer | Event connection ID. |
| `general_user.str` | string | User name recorded for the general event. |
| `general_user.length` | unsigned integer | User name length. |
| `general_command.str` | string | General command text, for example `Query`. |
| `general_command.length` | unsigned integer | General command text length. |
| `general_query.str` | string | SQL statement text associated with the event. |
| `general_query.length` | unsigned integer | SQL statement text length. |
| `general_host.str` | string | Client host name. |
| `general_host.length` | unsigned integer | Client host name length. |
| `general_sql_command.str` | string | SQL command name associated with the statement, for example `select`. |
| `general_sql_command.length` | unsigned integer | SQL command name length. |
| `general_external_user.str` | string | External user or OS login associated with the event. |
| `general_external_user.length` | unsigned integer | External user or OS login length. |
| `general_ip.str` | string | Client IP address. |
| `general_ip.length` | unsigned integer | Client IP address length. |

## `connection`

Supported events: `connect`, `disconnect`, `change_user`, `pre_authenticate`
REDUCED mode: `connect`, `disconnect`, `change_user`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `status` | integer | Current connection event status. |
| `connection_id` | unsigned integer | Connection ID. |
| `user.str` | string | User name of this connection. |
| `user.length` | unsigned integer | User name length. |
| `priv_user.str` | string | Privileged user name. |
| `priv_user.length` | unsigned integer | Privileged user name length. |
| `external_user.str` | string | External user name or OS login. |
| `external_user.length` | unsigned integer | External user name length. |
| `proxy_user.str` | string | Proxy user used for the connection. |
| `proxy_user.length` | unsigned integer | Proxy user name length. |
| `host.str` | string | Connection host name. |
| `host.length` | unsigned integer | Connection host name length. |
| `ip.str` | string | Connection IP address. |
| `ip.length` | unsigned integer | Connection IP address length. |
| `database.str` | string | Default database specified at connection time. |
| `database.length` | unsigned integer | Default database name length. |
| `connection_type` | integer | Connection type code. |
|  |  | `0` or `::undefined`: Undefined |
|  |  | `1` or `::tcp/ip`: TCP/IP |
|  |  | `2` or `::socket`: Socket |
|  |  | `3` or `::named_pipe`: Named pipe |
|  |  | `4` or `::ssl`: TCP/IP with encryption |
|  |  | `5` or `::shared_memory`: Shared memory |

## `table_access`

Supported events: `read`, `insert`, `update`, `delete`
REDUCED mode: all events

| Field Name | Field Type | Description |
| --- | --- | --- |
| `connection_id` | unsigned integer | Event connection ID. |
| `sql_command_id` | integer | SQL command ID. |
| `query.str` | string | SQL statement text. |
| `query.length` | unsigned integer | SQL statement text length. |
| `table_database.str` | string | Database name associated with event. |
| `table_database.length` | unsigned integer | Database name length. |
| `table_name.str` | string | Table name associated with event. |
| `table_name.length` | unsigned integer | Table name length. |

## `global_variable` *(FULL mode only)*

Supported events: `get`, `set`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `connection_id` | string | Event connection ID. |
| `variable_name.str` | string | Variable name. |
| `variable_name.length` | string | Variable name length. |
| `variable_value.str` | string | Variable value. |
| `variable_value.length` | string | Variable value length. |

## `command` *(FULL mode only)*

Supported events: `start`, `end`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `status` | string | Command event status code. |
| `connection_id` | string | Event connection ID. |
| `command.str` | string | Command text. |
| `command.length` | string | Command text length. |

## `query` *(FULL mode only)*

Supported events: `start`, `nested_start`, `status_end`, `nested_status_end`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `status` | string | Query event status code. |
| `connection_id` | string | Event connection ID. |
| `sql_command_id` | string | SQL command string associated with the query event. The field name is retained as `sql_command_id` for compatibility. |
| `query.str` | string | SQL query text. |
| `query.length` | string | SQL query text length. |
| `query_charset` | string | SQL query character set name. |

## `stored_program` *(FULL mode only)*

Supported events: `execute`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `connection_id` | string | Event connection ID. |
| `database.str` | string | Database where the stored program is defined. |
| `database.length` | string | Database name length. |
| `name.str` | string | Stored program name. |
| `name.length` | string | Stored program name length. |

## `authentication` *(FULL mode only)*

Supported events: `flush`, `authid_create`, `credential_change`, `authid_rename`, `authid_drop`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `status` | string | Authentication event status. |
| `connection_id` | string | Event connection ID. |
| `user.str` | string | User name. |
| `user.length` | string | User name length. |
| `host.str` | string | Host name. |
| `host.length` | string | Host name length. |

## `message`

Supported events: `internal`, `user`
REDUCED mode: all events

| Field Name | Field Type | Description |
| --- | --- | --- |
| `connection_id` | string | Event connection ID. |
| `component.str` | string | Component name. |
| `component.length` | string | Component name length. |
| `producer.str` | string | Message producer name. |
| `producer.length` | string | Message producer name length. |
| `message.str` | string | Message text. |
| `message.length` | string | Message text length. |

## `parse` *(FULL mode only)*

Supported events: `preparse`, `postparse`

| Field Name | Field Type | Description |
| --- | --- | --- |
| `connection_id` | string | Event connection ID. |
| `flags` | string | Parse rewrite flags value. |
| `query.str` | string | Original SQL query text. |
| `query.length` | string | Original SQL query text length. |
| `rewritten_query.str` | string | Rewritten SQL query text. |
| `rewritten_query.length` | string | Rewritten SQL query text length. |
