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

import java.io.Serializable;
import java.util.List;
import javax.persistence.OneToMany;

/** Schema
 *
create table longlongstringpk (
 longpk1 bigint not null,
 longpk2 bigint not null,
 stringpk varchar(10) not null,
 stringvalue varchar(10),
        CONSTRAINT PK_longlongstringpk PRIMARY KEY (longpk1, longpk2, stringpk)

) ENGINE=ndbcluster DEFAULT CHARSET=latin1;
 */
@javax.persistence.Entity
@javax.persistence.Table(name="longlongstringpk")
@javax.persistence.IdClass(value=LongLongStringOid.class)
public class LongLongStringPK extends LongLongStringConstants implements Serializable {

    @javax.persistence.Id
    @javax.persistence.Column(name="longpk1")
    private long longpk1;

    @javax.persistence.Id
    @javax.persistence.Column(name="longpk2")
    private long longpk2;

    @javax.persistence.Id
    @javax.persistence.Column(name="stringpk")
    private String stringpk;

    @javax.persistence.Column(name="stringvalue")
    private String stringvalue;

    public LongLongStringPK() {
    }

    public long getLongpk1() {
        return longpk1;
    }

    public void setLongpk1(long value) {
        longpk1 = value;
    }

    public long getLongpk2() {
        return longpk2;
    }

    public void setLongpk2(long value) {
        longpk2 = value;
    }

    public String getStringpk() {
        return stringpk;
    }

    public void setStringpk(String value) {
        stringpk = value;
    }

    static public LongLongStringPK create(int id) {
        LongLongStringPK o = new LongLongStringPK();
        o.longpk1 = getPK1(id);
        o.longpk2 = getPK2(id);
        o.stringpk = getPK3(id);
        o.stringvalue = getValue(id);
        return o;
    }

    static public LongLongStringOid createOid(int id) {
        return new LongLongStringOid(id);
    }

    @Override
    public String toString() {
        StringBuffer result = new StringBuffer();
        result.append("LongLongStringPK[");
        result.append(longpk1);
        result.append(",");
        result.append(longpk2);
        result.append(",\"");
        result.append(stringpk);
        result.append("\"]: ");
        result.append(stringvalue);
        result.append(".");
        return result.toString();
    }

}

