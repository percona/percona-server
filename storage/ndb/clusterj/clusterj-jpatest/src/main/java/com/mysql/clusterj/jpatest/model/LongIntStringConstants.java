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

public abstract class LongIntStringConstants {

    /** The modulus used to create keys */
    static final int PK_MODULUS = 3;

    /** A large number for creating long values */
    static final long PRETTY_BIG_NUMBER = 1000000000000000L;

    protected static long getPK1(int index) {
        return PRETTY_BIG_NUMBER * ((index / PK_MODULUS / PK_MODULUS) % PK_MODULUS);
    }

    protected static int getPK2(int index) {
        return (index / PK_MODULUS) % PK_MODULUS;
    }

    protected static String getPK3(int index) {
        return "" + (index % PK_MODULUS);
    }

    protected static String getValue(int index) {
        return "Value " + index;
    }

}

