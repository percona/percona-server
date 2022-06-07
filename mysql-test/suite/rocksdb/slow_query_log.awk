#!/bin/awk

/Query_time:/ {
  results["Rows_examined:"] = "uninit";

  for (i = 2; i <= NF; i = i+2) {
    results[$i] = $(i+1);
  }

  # If the output format has changed and we don't find these keys,
  # error out.
  if (results["Rows_examined:"] == "uninit") {
    exit(-2);
  }

  if (results["Rows_examined:"] == 0) {
    next
  }
}
