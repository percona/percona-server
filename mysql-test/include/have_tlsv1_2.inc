if (!`SELECT COUNT(*) FROM information_schema.global_variables WHERE variable_name='have_tlsv1_2' and variable_value='YES'`) {
  --skip Need SSL library with TLSv1.2 support
}
