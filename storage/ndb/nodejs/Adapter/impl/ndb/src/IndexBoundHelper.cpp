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


#include <NdbApi.hpp>

#include <node_buffer.h>

#include "adapter_global.h"
#include "js_wrapper_macros.h"
#include "JsWrapper.h"

using namespace v8;


enum {
  BOUND_LOW_KEY = 0,
  BOUND_LOW_KEY_COUNT,
  BOUND_LOW_INCLUSIVE,
  BOUND_HIGH_KEY,
  BOUND_HIGH_KEY_COUNT,
  BOUND_HIGH_INCLUSIVE,
  BOUND_RANGE_NO
};

Envelope IndexBoundEnvelope("IndexBound");


Handle<Value> newIndexBound(const Arguments &args) {
  HandleScope scope;

  const Local<Object> spec = args[0]->ToObject();
  Local<Value> v;
  Local<Object> o;

  NdbIndexScanOperation::IndexBound * bound = 
    new NdbIndexScanOperation::IndexBound;
  Local<Object> jsBound = IndexBoundEnvelope.newWrapper();
  wrapPointerInObject(bound, IndexBoundEnvelope, jsBound);

  bound->low_key = 0;
  v = spec->Get(BOUND_LOW_KEY);
  if(! v->IsNull()) {
    o = v->ToObject();
    bound->low_key = node::Buffer::Data(o);
  }

  bound->low_key_count = 0;
  v = spec->Get(BOUND_LOW_KEY_COUNT);
  if(! v->IsNull()) {
    bound->low_key_count = v->Uint32Value();
  }
  
  bound->low_inclusive = false;
  v = spec->Get(BOUND_LOW_INCLUSIVE);
  if(! v->IsNull()) {
    bound->low_inclusive = v->BooleanValue();
  }
  
  bound->high_key = 0;
  v = spec->Get(BOUND_HIGH_KEY);
  if(! v->IsNull()) {
    o = v->ToObject();
    bound->high_key = node::Buffer::Data(o);
  }
  
  bound->high_key_count = 0;
  v = spec->Get(BOUND_HIGH_KEY_COUNT);
  if(! v->IsNull()) {
    bound->high_key_count = v->Uint32Value();
  }
  
  bound->high_inclusive = false;
  v = spec->Get(BOUND_HIGH_INCLUSIVE);
  if(! v->IsNull()) {
    bound->high_inclusive = v->BooleanValue();
  }
  
  bound->range_no = 0;
  v = spec->Get(BOUND_RANGE_NO);
  if(! v->IsNull()) {
    bound->range_no = v->Uint32Value();
  }
  
  return scope.Close(jsBound);
}



void IndexBound_initOnLoad(Handle<Object> target) {
  Persistent<Object> ibObj = Persistent<Object>(Object::New());
  Persistent<String> ibKey = Persistent<String>(String::NewSymbol("IndexBound"));
  target->Set(ibKey, ibObj);

  DEFINE_JS_FUNCTION(ibObj, "create", newIndexBound);

  Persistent<Object> BoundHelper = Persistent<Object>(Object::New());
  ibObj->Set(Persistent<String>(String::NewSymbol("helper")), BoundHelper);

  DEFINE_JS_INT(BoundHelper, "low_key", BOUND_LOW_KEY);
  DEFINE_JS_INT(BoundHelper, "low_key_count", BOUND_LOW_KEY_COUNT);
  DEFINE_JS_INT(BoundHelper, "low_inclusive", BOUND_LOW_INCLUSIVE);
  DEFINE_JS_INT(BoundHelper, "high_key", BOUND_HIGH_KEY);
  DEFINE_JS_INT(BoundHelper, "high_key_count", BOUND_HIGH_KEY_COUNT);
  DEFINE_JS_INT(BoundHelper, "high_inclusive", BOUND_HIGH_INCLUSIVE);
  DEFINE_JS_INT(BoundHelper, "range_no", BOUND_RANGE_NO);
}

