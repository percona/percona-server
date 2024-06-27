if (`SELECT COUNT(*) = 0 FROM mysql.component WHERE component_urn='file://component_percona_telemetry'`) {
  --skip Test not supported when Percona Telemetry is not built-in
}
