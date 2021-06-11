/*
 Copyright (c) 2012, 2021, Oracle and/or its affiliates. All rights
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
"use strict";

var util        = require("util");
var udebug      = unified_debug.getLogger("GetMappingTest.js");

var expectedMappingFor_t_basic = {
    "table" : "t_basic",
    "database" : "test",
    "fields" : [{
      "fieldName"    : "id",
      "defaultValue" : undefined,
      "columnName"   : "id",
      "persistent"   : true
    }]
};

var verifyMapping = function(testCase, expected, result) {
  udebug.log('GetMappingTest result: ', util.inspect(result));
  testCase.errorIfNotEqual('Mapping.table mismatch', expected.table, result.table);
  testCase.errorIfNotEqual('Mapping.database mismatch', expected.database, result.database);
  testCase.errorIfNotEqual('Mapping.fields.fieldName mismatch', expected.fields[0].fieldName, result.fields[0].fieldName);
  testCase.errorIfNotEqual('Mapping.fields.columnName mismatch', expected.fields[0].columnName, result.fields[0].columnName);
  testCase.errorIfNotEqual('Mapping.fields.persistent mismatch', expected.fields[0].persistent, result.fields[0].persistent);
  testCase.errorIfNotEqual('Mapping.fields.defaultValue mismatch', expected.fields[0].defaultValue, result.fields[0].defaultValue);
};

var t1 = new harness.SerialTest("getMappingForConstructor");
t1.run = function() {
  fail_openSession(t1, t1.testGetMapping);
};

t1.testGetMapping = function(session, testCase) {
  session.getMapping(global.t_basic, function(err, result) {
    if (err) {
      testCase.fail(err);
      return;
    }
    verifyMapping(testCase, expectedMappingFor_t_basic, result);
    testCase.failOnError();
    });
};

var t2 = new harness.SerialTest("getMappingForTableName");
t2.run = function() {
  fail_openSession(t2, t2.testGetMapping);
};

t2.testGetMapping = function(session, testCase) {
  session.getMapping('t_basic', function(err, result) {
    if (err) {
      testCase.fail(err);
      return;
    }
    verifyMapping(testCase, expectedMappingFor_t_basic, result);
    testCase.failOnError();
  });
};

var t3 = new harness.SerialTest("getMappingForUnknownTableName");
t3.run = function() {
  fail_openSession(t3, t3.testGetMapping);
};

t3.testGetMapping = function(session, testCase) {
  session.getMapping('non_existent_table', function(err, result) {
    if (err) {
      testCase.pass();
      return;
    }
    testCase.fail('t3 getMapping for unknown table should fail');
  });
};

var t4 = new harness.SerialTest("getMappingForUnMappedConstructor");
t4.run = function() {
  fail_openSession(t4, t4.testGetMapping);
};

t4.testGetMapping = function(session, testCase) {
  var unmappedDomainClass = function() {
  };
  session.getMapping(unmappedDomainClass, function(err, result) {
    if (err) {
      testCase.pass();
      return;
    }
    testCase.fail('t4 getMapping for unmapped domain object should fail');
  });
};

var t5 = new harness.SerialTest("getMappingForNumber");
t5.run = function() {
  fail_openSession(t5, t5.testGetMapping);
};

t5.testGetMapping = function(session, testCase) {
  var unmappedDomainClass = function() {
  };
  try {
    session.getMapping(1, function(err, result) {
      if (!err) {
        testCase.fail('t5 getMapping for 1 should fail');
      } else {
        testCase.pass();
      }
    });
  } catch (err) {
    testCase.pass();
  }
};

var t6 = new harness.SerialTest("getMappingForBoolean");
t6.run = function() {
  fail_openSession(t6, t6.testGetMapping);
};

t6.testGetMapping = function(session, testCase) {
  var unmappedDomainClass = function() {
  };
  try {
    session.getMapping(true, function(err, result) {
      if (!err) {
        testCase.fail('t6 getMapping for true should fail');
      } else {
        testCase.pass();
      }
    });
  } catch (err) {
    testCase.pass();
  }
};

var t7 = new harness.SerialTest("getMappingForDomainObject");
t7.run = function() {
  fail_openSession(t7, t7.testGetMapping);
};

t7.testGetMapping = function(session, testCase) {
  session.getMapping(new global.t_basic(), function(err, result) {
    if (err) {
      testCase.fail(err);
    } else {
      verifyMapping(testCase, expectedMappingFor_t_basic, result);
      testCase.failOnError();
    }
  });
};

module.exports.tests = [t1, t2, t3, t4, t5, t6, t7];
