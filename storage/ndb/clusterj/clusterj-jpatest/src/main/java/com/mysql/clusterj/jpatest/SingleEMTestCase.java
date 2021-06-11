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

import javax.persistence.EntityManager;
import javax.persistence.EntityTransaction;

/**
 * A base test case for a single EntityManager at any given time.
 */
public abstract class SingleEMTestCase extends SingleEMFTestCase {

    protected EntityManager em;

    protected EntityTransaction tx;

    @Override
    public void setUp() {
        super.setUp();
        em = emf.createEntityManager(); 
    }

    @Override
    public void setUp(Object... props) {
        super.setUp(props);
        em = emf.createEntityManager(); 
    }

    @Override
    public void tearDown() throws Exception {
        rollback();
        close();
        super.tearDown();
    }

    /** 
     * Start a new transaction if there isn't currently one active. 
     * @return true if a transaction was started, false if one already existed
     */
    protected boolean begin() {
        EntityTransaction tx = em.getTransaction();
        if (tx.isActive())
            return false;
        tx.begin();
        return true;
    }

    /** 
     * Commit the current transaction, if it is active. 
     * @return true if the transaction was committed
     */
    protected boolean commit() {
        EntityTransaction tx = em.getTransaction();
        if (!tx.isActive())
            return false;

        tx.commit();
        return true;
    }

    /** 
     * Roll back the current transaction, if it is active. 
     * @return true if the transaction was rolled back
     */
    protected boolean rollback() {
        if (em != null && em.isOpen()) {
            EntityTransaction tx = em.getTransaction();
            if (!tx.isActive()) {
                return false;
            } else {
                tx.rollback();
                return true;
            }
        } else {
            return false;
        }
    }

    /** 
     * Closes the current EntityManager if it is open. 
     * @return false if the EntityManager was already closed
     */
    protected boolean close() {
        if (em == null)
            return false;

        rollback();

        if (!em.isOpen())
            return false;

        em.close();
        return !em.isOpen();
    }

}
