/*
 *  Copyright (c) 2011, 2021, Oracle and/or its affiliates.
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

/* Please see the following wiki for details of this functionality:
 * http://www.antlr.org/wiki/pages/viewpage.action?pageId=1782
 */

package com.mysql.clusterj.jdbc.antlr;

import java.io.IOException;
import java.io.File;
import java.io.InputStreamReader;
import java.io.FileInputStream;

/** This is a char buffer stream that is loaded from a file
 *  all at once when you construct the object.  This looks very
 *  much like an ANTLReader or ANTLRInputStream, but it's a special case
 *  since we know the exact size of the object to load.  We can avoid lots
 *  of data copying.
 *  The only difference to ANTLRFileStream is that it extends ANTLRNoCaseStringStream.
 */
public class ANTLRNoCaseFileStream extends ANTLRNoCaseStringStream {
    transient private String fileName;

    public ANTLRNoCaseFileStream(final String fileName) throws IOException {
        this(fileName, null);
    }

    public ANTLRNoCaseFileStream(final String fileName, final String encoding) throws IOException {
        this.fileName = fileName;
        load(fileName, encoding);
    }

    private void load(final String fileName, final String encoding)
        throws IOException
    {
        if ( fileName==null ) {
            return;
        }
        final File f = new File(fileName);
        final int size = (int)f.length();
        InputStreamReader isr;
        final FileInputStream fis = new FileInputStream(fileName);
        if ( encoding!=null ) {
            isr = new InputStreamReader(fis, encoding);
        }
        else {
            isr = new InputStreamReader(fis);
        }
        try {
            data = new char[size];
            super.n = isr.read(data);
        }
        finally {
            isr.close();
        }
    }

    public String getSourceName() {
        return fileName;
    }
}
