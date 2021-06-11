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

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import org.antlr.runtime.ANTLRStringStream;
import org.antlr.runtime.CharStream;

public class ANTLRNoCaseStringStream extends ANTLRStringStream {

    transient private String fileName;

    public ANTLRNoCaseStringStream() {
    	super();
    }
    
    public ANTLRNoCaseStringStream(final String string) {
        super(string);
    }

    public ANTLRNoCaseStringStream(final File file) throws IOException {
        this(file, null);
    }

    public ANTLRNoCaseStringStream(final File file, final String encoding)
        throws IOException {
        this.fileName = file.getName();
        load(file, encoding);
    }

    @Override
    public int LA(final int i) {
    	int idx = i;
        if (idx == 0) {
            return 0; // undefined
        }
        if (idx < 0) {
            idx++;
            if ((p + idx - 1) < 0) {
                return CharStream.EOF; // invalid; no char before first char
            }
        }

        if ((p + idx - 1) >= n) {
            return CharStream.EOF;
        }
        return Character.toUpperCase(data[p + idx - 1]);
    }

    /**
     * For use in semantic predicates that need to access the actual case of a
     * character for correct tokenization.
     * 
     * @param i
     *            Amount of lookahead characters
     * @return the character at that lookahead depth
     */
    public int trueCaseLA(final int i) {
        return super.LA(i);
    }

    private void load(final File file, final String encoding) throws IOException {
        if (file == null) {
            return;
        }
        final int size = (int) file.length();
        InputStreamReader isr;
        final FileInputStream fis = new FileInputStream(file);
        if (encoding != null) {
            isr = new InputStreamReader(fis, encoding);
        } else {
            isr = new InputStreamReader(fis);
        }
        try {
            data = new char[size];
            super.n = isr.read(data);
        } finally {
            isr.close();
        }
    }

    public String getSourceName() {
        return fileName;
    }
}
