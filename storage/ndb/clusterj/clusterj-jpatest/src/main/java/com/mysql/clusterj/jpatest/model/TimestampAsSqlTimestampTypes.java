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

package com.mysql.clusterj.jpatest.model;

import java.sql.Timestamp;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

/** Schema
 *
drop table if exists timestamptypes;
create table timestamptypes (
 id int not null primary key,

 timestamp_not_null_hash timestamp,
 timestamp_not_null_btree timestamp,
 timestamp_not_null_both timestamp,
 timestamp_not_null_none timestamp

) ENGINE=ndbcluster DEFAULT CHARSET=latin1;

create unique index idx_timestamp_not_null_hash using hash on timestamptypes(timestamp_not_null_hash);
create index idx_timestamp_not_null_btree on timestamptypes(timestamp_not_null_btree);
create unique index idx_timestamp_not_null_both on timestamptypes(timestamp_not_null_both);

 */
@Entity
@Table(name="timestamptypes")
public class TimestampAsSqlTimestampTypes implements IdBase {

    private int id;
    private Timestamp timestamp_not_null_hash;
    private Timestamp timestamp_not_null_btree;
    private Timestamp timestamp_not_null_both;
    private Timestamp timestamp_not_null_none;

    @Id
    public int getId() {
        return id;
    }
    public void setId(int id) {
        this.id = id;
    }

    // Timestamp
    @Column(name="timestamp_not_null_hash")
    public Timestamp getTimestamp_not_null_hash() {
        return timestamp_not_null_hash;
    }
    public void setTimestamp_not_null_hash(Timestamp value) {
        this.timestamp_not_null_hash = value;
    }

    @Column(name="timestamp_not_null_btree")
    public Timestamp getTimestamp_not_null_btree() {
        return timestamp_not_null_btree;
    }
    public void setTimestamp_not_null_btree(Timestamp value) {
        this.timestamp_not_null_btree = value;
    }

    @Column(name="timestamp_not_null_both")
    public Timestamp getTimestamp_not_null_both() {
        return timestamp_not_null_both;
    }
    public void setTimestamp_not_null_both(Timestamp value) {
        this.timestamp_not_null_both = value;
    }

    @Column(name="timestamp_not_null_none")
    public Timestamp getTimestamp_not_null_none() {
        return timestamp_not_null_none;
    }
    public void setTimestamp_not_null_none(Timestamp value) {
        this.timestamp_not_null_none = value;
    }

}
