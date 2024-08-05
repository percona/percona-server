--disable_query_log

#
# Check if the variable JS_LANG_COMPONENT is set
#
if (!$JS_LANG_COMPONENT) {
  --skip component requires the environment variable \$JS_LANG_COMPONENT to be set (normally done by mtr), see the file plugin.defs
}

#
# Check if --plugin-dir was setup for component_js_lang
#
if (`SELECT CONCAT('--plugin-dir=', REPLACE(@@plugin_dir, '\\\\', '/')) != '$JS_LANG_COMPONENT_OPT/'`) {
  --skip component_js_lang requires that --plugin-dir is set to the component_js_lang dir (the .opt file does not contain \$JS_LANG_COMPONENT_OPT)
}

--enable_query_log
