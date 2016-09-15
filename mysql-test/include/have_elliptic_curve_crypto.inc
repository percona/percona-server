if (!`SELECT COUNT(*) FROM information_schema.global_variables WHERE variable_name='have_elliptic_curve_crypto' and variable_value='YES'`) {
  --skip Need SSL library with elliptic curve support
}
