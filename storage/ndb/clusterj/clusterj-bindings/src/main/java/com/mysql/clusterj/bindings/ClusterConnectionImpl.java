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

import com.mysql.cluster.ndbj.Ndb;
import com.mysql.cluster.ndbj.NdbApiException;
import com.mysql.cluster.ndbj.NdbClusterConnection;
import com.mysql.clusterj.ClusterJDatastoreException;
import com.mysql.clusterj.core.store.Db;
import com.mysql.clusterj.core.util.I18NHelper;
import com.mysql.clusterj.core.util.Logger;
import com.mysql.clusterj.core.util.LoggerFactoryService;

/**
 *
 */
public class ClusterConnectionImpl
        implements com.mysql.clusterj.core.store.ClusterConnection {

    /** My message translator */
    static final I18NHelper local = I18NHelper.getInstance(ClusterConnectionImpl.class);

    /** My logger */
    static final Logger logger = LoggerFactoryService.getFactory()
            .getInstance(com.mysql.clusterj.core.store.ClusterConnection.class);

    /** Ndb_cluster_connection: one per factory. */
    protected NdbClusterConnection clusterConnection;

    public ClusterConnectionImpl(String connectString) {
        try {
            clusterConnection = NdbClusterConnection.create(connectString);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void connect(int connectRetries, int connectDelay, boolean verbose) {
        try {
            clusterConnection.connect(connectRetries, connectDelay, verbose);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public Db createDb(String database, int maxTransactions) {
        try {
            Ndb ndb = clusterConnection.createNdb(database, maxTransactions);
            return new DbImpl(ndb);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

    public void waitUntilReady(int connectTimeoutBefore, int connectTimeoutAfter) {
        try {
            clusterConnection.waitUntilReady(connectTimeoutBefore, connectTimeoutAfter);
        } catch (NdbApiException ndbApiException) {
            throw new ClusterJDatastoreException(local.message("ERR_Datastore"),
                    ndbApiException);
        }
    }

}
