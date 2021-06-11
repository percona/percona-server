/*
 *  Copyright (c) 2010, 2021, Oracle and/or its affiliates.
 *  All rights reserved. Use is subject to license terms.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2.0,
 *  as published by the Free Software Foundation.
 *
 *  This program is also distributed with certain software (including
 *  but not limited to OpenSSL) that is licensed under separate terms,
 *  as designated in a particular file or component or in included license
 *  documentation.  The authors of MySQL hereby grant you an additional
 *  permission to link the program and your derivative works with the
 *  separately licensed software that they have included with MySQL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License, version 2.0, for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

package com.mysql.clusterj.bindings;

import com.mysql.cluster.ndbj.NdbApiException;
import com.mysql.cluster.ndbj.NdbIndexScanOperation;

import com.mysql.clusterj.ClusterJDatastoreException;
import com.mysql.clusterj.ClusterJFatalInternalException;

import com.mysql.clusterj.core.store.Column;
import com.mysql.clusterj.core.store.IndexScanOperation;
import com.mysql.clusterj.core.util.I18NHelper;
import com.mysql.clusterj.core.util.Logger;
import com.mysql.clusterj.core.util.LoggerFactoryService;

import java.math.BigDecimal;

import java.sql.Date;
import java.sql.Time;
import java.sql.Timestamp;

/**
 *
 */
class IndexScanOperationImpl extends ScanOperationImpl implements IndexScanOperation {

    /** My message translator */
    static final I18NHelper local = I18NHelper.getInstance(ClusterConnectionImpl.class);

    /** My logger */
    static final Logger logger = LoggerFactoryService.getFactory()
            .getInstance(IndexScanOperationImpl.class);

    private NdbIndexScanOperation ndbIndexScanOperation;

    public IndexScanOperationImpl(NdbIndexScanOperation selectIndexScanOperation,
            ClusterTransactionImpl transaction) {
        super(selectIndexScanOperation, transaction);
        this.ndbIndexScanOperation = selectIndexScanOperation;
    }

    public void setBoundByte(Column storeColumn, BoundType type, byte byteValue) {
        try {
            ndbIndexScanOperation.setBoundInt(storeColumn.getName(), convertBoundType(type), (int)byteValue);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundBytes(Column storeColumn, BoundType type, byte[] value) {
        try {
            ndbIndexScanOperation.setBoundBytes(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundDatetime(Column storeColumn, BoundType type, Timestamp value) {
        try {
            ndbIndexScanOperation.setBoundDatetime(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundDate(Column storeColumn, BoundType type, Date value) {
        try {
            Timestamp timestamp = new Timestamp(value.getTime());
            ndbIndexScanOperation.setBoundDatetime(storeColumn.getName(), convertBoundType(type), timestamp);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundTime(Column storeColumn, BoundType type, Time value) {
        try {
            Timestamp timestamp = new Timestamp(value.getTime());
            ndbIndexScanOperation.setBoundDatetime(storeColumn.getName(), convertBoundType(type), timestamp);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundDecimal(Column storeColumn, BoundType type, BigDecimal value) {
        try {
            ndbIndexScanOperation.setBoundDecimal(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundDouble(Column storeColumn, BoundType type, Double value) {
        try {
            ndbIndexScanOperation.setBoundDouble(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundFloat(Column storeColumn, BoundType type, Float value) {
        try {
            ndbIndexScanOperation.setBoundFloat(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundInt(Column storeColumn, BoundType type, Integer value) {
        try {
            ndbIndexScanOperation.setBoundInt(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundLong(Column storeColumn, BoundType type, long value) {
        try {
            ndbIndexScanOperation.setBoundLong(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundString(Column storeColumn, BoundType type, String value) {
        try {
            ndbIndexScanOperation.setBoundString(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void setBoundTimestamp(Column storeColumn, BoundType type, Timestamp value) {
        try {
            ndbIndexScanOperation.setBoundTimestamp(storeColumn.getName(), convertBoundType(type), value);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    private NdbIndexScanOperation.BoundType convertBoundType(BoundType type) {
        switch (type) {
            case BoundEQ:
                return NdbIndexScanOperation.BoundType.BoundEQ;
            case BoundGE:
                return NdbIndexScanOperation.BoundType.BoundGE;
            case BoundGT:
                return NdbIndexScanOperation.BoundType.BoundGT;
            case BoundLE:
                return NdbIndexScanOperation.BoundType.BoundLE;
            case BoundLT:
                return NdbIndexScanOperation.BoundType.BoundLT;
            default:
                throw new ClusterJFatalInternalException(
                        local.message("ERR_Implementation_Should_Not_Occur"));
        }
    }

}
