--copy_file $MYSQLTEST_VARDIR/std_data/$DATA_DIR_NAME.zip $MYSQL_TMP_DIR/$DATA_DIR_NAME.zip

--echo # Check that the file exists in the working folder.
--file_exists $MYSQL_TMP_DIR/$DATA_DIR_NAME.zip

--echo # Unzip the zip file.
--exec unzip -qo $MYSQL_TMP_DIR/$DATA_DIR_NAME.zip -d $MYSQL_TMP_DIR

