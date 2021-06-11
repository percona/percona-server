/*
 Copyright (c) 2013, 2021, Oracle and/or its affiliates. All rights
 reserved.
 
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
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 02110-1301  USA
*/


/**********************
  This is the standard TypeConverter class used with DATETIME columns 
  in the Ndb Adapter.
  
  DATETIME columns are read and written using a MySQLTime structure,
  which provides lossless interchange.
  
  This Converter converts between MySQLTime and standard JavaScript Date.  

  While convenient, such a conversion can result in lost precision. 
  JavaSCript Date supports millisecond precision, while MySQL DATETIME
  can support up to microsecond precision.

  An application can override this converter and use MySQLTime directly:
    sessionFactory.registerTypeConverter("DATETIME", null);
  
  Or replace this converter with a custom one:
    sessionFactory.registerTypeConverter("DATETIME", myConverterClass);
      
************************/


var path = require("path"),
    MySQLTime = require(path.join(mynode.fs.spi_dir,"common","MySQLTime.js")),
    udebug = unified_debug.getLogger("NdbDatetimeConverter.js");


exports.toDB = function(jsdate) {
  return new MySQLTime().initializeFromJsDateLocal(jsdate);
};

exports.fromDB = function(dbTime) {
  return MySQLTime.initializeFromNdb(dbTime).toJsDateLocal();
};

