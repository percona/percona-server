/*
   Copyright (c) 2010, 2021, Oracle and/or its affiliates.
   All rights reserved. Use is subject to license terms.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

package com.mysql.clusterj.jpatest;

import java.util.Date;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import com.mysql.clusterj.jpatest.model.TimeAsUtilDateTypes;
import com.mysql.clusterj.jpatest.model.IdBase;

/** Test that Dates can be read and written. 
 * case 1: Write using JDBC, read using JPA.
 * case 2: Write using JPA, read using JDBC.
 * case 3: Write using JDBC, read using JDBC.
 * case 4: Write using JPA, read using JPA.
 * Schema
 * 
drop table if exists timetypes;
create table timetypes (
id int not null primary key,

time_not_null_hash time,
time_not_null_btree time,
time_not_null_both time,
time_not_null_none time

) ENGINE=ndbcluster DEFAULT CHARSET=latin1;

create unique index idx_time_not_null_hash using hash on timetypes(time_not_null_hash);
create index idx_time_not_null_btree on timetypes(time_not_null_btree);
create unique index idx_time_not_null_both on timetypes(time_not_null_both);

*/
public class TimeAsUtilDateTest extends AbstractJPABaseTest {

    static int NUMBER_OF_INSTANCES = 10;

    @Override
    protected boolean getDebug() {
        return false;
    }

    @Override
    protected int getNumberOfInstances() {
        return NUMBER_OF_INSTANCES;
    }

    @Override
    protected String getTableName() {
        return "timetypes";
    }

    /** Subclasses override this method to provide the model class for the test */
    @Override
    protected Class<? extends IdBase> getModelClass() {
        return TimeAsUtilDateTypes.class;
    }

    /** Subclasses override this method to provide values for rows (i) and columns (j) */
    @Override
    protected Object getColumnValue(int i, int j) {
        return new Date(getMillisFor(0, i, i, j));
    }

    @Override
    /** Subclasses must override this method to implement the model factory for the test */
    protected IdBase getNewInstance(Class<? extends IdBase> modelClass) {
        return new TimeAsUtilDateTypes();
    }

    public void testWriteJDBCReadJPA() {
        writeJDBCreadJPA();
        failOnError();
    }

    public void testWriteJPAReadJDBC() {
        writeJPAreadJDBC();
        failOnError();
   }

    public void testWriteJDBCReadJDBC() {
        writeJDBCreadJDBC();
        failOnError();
    }

    public void testWriteJPAReadJPA() {
        writeJPAreadJPA();
        failOnError();
   }

   static ColumnDescriptor not_null_hash = new ColumnDescriptor
            ("time_not_null_hash", new InstanceHandler() {
        public void setFieldValue(IdBase instance, Object value) {
            ((TimeAsUtilDateTypes)instance).setTime_not_null_hash((Date)value);
        }
        public Object getFieldValue(IdBase instance) {
            return ((TimeAsUtilDateTypes)instance).getTime_not_null_hash();
        }
        public void setPreparedStatementValue(PreparedStatement preparedStatement, int j, Object value)
                throws SQLException {
            java.sql.Time time = new java.sql.Time(((Date)value).getTime());
            preparedStatement.setTime(j, time);
        }
        public Object getResultSetValue(ResultSet rs, int j) throws SQLException {
            return rs.getTime(j);
        }
    });

    static ColumnDescriptor not_null_btree = new ColumnDescriptor
            ("time_not_null_btree", new InstanceHandler() {
        public void setFieldValue(IdBase instance, Object value) {
            ((TimeAsUtilDateTypes)instance).setTime_not_null_btree((Date)value);
        }
        public Object getFieldValue(IdBase instance) {
            return ((TimeAsUtilDateTypes)instance).getTime_not_null_btree();
        }
        public void setPreparedStatementValue(PreparedStatement preparedStatement, int j, Object value)
                throws SQLException {
            java.sql.Time time = new java.sql.Time(((Date)value).getTime());
            preparedStatement.setTime(j, time);
        }
        public Object getResultSetValue(ResultSet rs, int j) throws SQLException {
            return rs.getTime(j);
        }
    });
    static ColumnDescriptor not_null_both = new ColumnDescriptor
            ("time_not_null_both", new InstanceHandler() {
        public void setFieldValue(IdBase instance, Object value) {
            ((TimeAsUtilDateTypes)instance).setTime_not_null_both((Date)value);
        }
        public Date getFieldValue(IdBase instance) {
            return ((TimeAsUtilDateTypes)instance).getTime_not_null_both();
        }
        public void setPreparedStatementValue(PreparedStatement preparedStatement, int j, Object value)
                throws SQLException {
            java.sql.Time time = new java.sql.Time(((Date)value).getTime());
            preparedStatement.setTime(j, time);
        }
        public Object getResultSetValue(ResultSet rs, int j) throws SQLException {
            return rs.getTime(j);
        }
    });
    static ColumnDescriptor not_null_none = new ColumnDescriptor
            ("time_not_null_none", new InstanceHandler() {
        public void setFieldValue(IdBase instance, Object value) {
            ((TimeAsUtilDateTypes)instance).setTime_not_null_none((Date)value);
        }
        public Date getFieldValue(IdBase instance) {
            return ((TimeAsUtilDateTypes)instance).getTime_not_null_none();
        }
        public void setPreparedStatementValue(PreparedStatement preparedStatement, int j, Object value)
                throws SQLException {
            java.sql.Time time = new java.sql.Time(((Date)value).getTime());
            preparedStatement.setTime(j, time);
        }
        public Object getResultSetValue(ResultSet rs, int j) throws SQLException {
            return rs.getTime(j);
        }
    });

    protected static ColumnDescriptor[] columnDescriptors = new ColumnDescriptor[] {
            not_null_hash,
            not_null_btree,
            not_null_both,
            not_null_none
        };

    @Override
    protected ColumnDescriptor[] getColumnDescriptors() {
        return columnDescriptors;
    }

}
