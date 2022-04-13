# ==== Purpose ====
#
# Check if xml file formatting is valid.
#
# ==== Usage ====
#
# --let $xml_file_path = xml file path
# --let $xml_file_name = xml file name
# --source validate_xml_file.inc
#
# Parameters:
#
# $xml_file_path
#   The xml file path.
# $xml_file_name
#   The xml file name.

--let xml_file_path=$xml_file_path
--let xml_file_name=$xml_file_name

perl;
  eval "use XML::Parser; 1" or exit 0;
  $p = new XML::Parser;
  $p->parsefile("$ENV{'xml_file_path'}$ENV{'xml_file_name'}");
EOF
