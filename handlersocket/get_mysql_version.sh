#!/bin/bash
grep 'define VERSION' $1/my_config*.h | cut -d ' ' -f 3 | cut -d '"' -f 2
