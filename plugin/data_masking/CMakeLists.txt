# Copyright (c) 2018, 2019, Francisco Miguel Biete Banon. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

SET(DATAMASKING_SOURCES
  src/plugin.cc
  src/plugin_memory.cc
  src/udf/udf_gen_blacklist.cc
  src/udf/udf_gen_dictionary.cc
  src/udf/udf_gen_dictionary_drop.cc
  src/udf/udf_gen_dictionary_load.cc
  src/udf/udf_gen_range.cc
  src/udf/udf_gen_rnd_email.cc
  src/udf/udf_gen_rnd_pan.cc
  src/udf/udf_gen_rnd_ssn.cc
  src/udf/udf_gen_rnd_us_phone.cc
  src/udf/udf_mask_inner.cc
  src/udf/udf_mask_outer.cc
  src/udf/udf_mask_pan.cc
  src/udf/udf_mask_pan_relaxed.cc
  src/udf/udf_mask_ssn.cc
  src/udf/udf_registration.cc
  src/udf/udf_utils.cc
  src/udf/udf_utils_string.cc
)

### Configuration ###

# declare the plugin itself
MYSQL_ADD_PLUGIN(data_masking
                 ${DATAMASKING_SOURCES}
MODULE_ONLY MODULE_OUTPUT_NAME "data_masking")

INSTALL(FILES data_masking.ini DESTINATION ${INSTALL_PLUGINDIR} COMPONENT Test)
