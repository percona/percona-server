/*
   Copyright (c) 2003, 2021, Oracle and/or its affiliates.
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


#include <ndb_global.h>

#include <NdbMain.h>
#include <mgmapi.h>
#include <ConfigRetriever.hpp>
#include <Properties.hpp>
#include <NdbOut.hpp>

void usage(const char * prg){
  ndbout << "Usage " << prg 
	 << " host <mgm host> <mgm port> <node id> [<ver id>]" << endl;
  
  char buf[255];
  for(unsigned i = 0; i<strlen(prg); i++)
    buf[i] = ' ';
  buf[strlen(prg)] = 0;
  ndbout << "      " << buf << "  file <filename> <node id> [<ver id>]"
	 << endl;
}

NDB_COMMAND(printConfig, 
	    "printConfig", "printConfig", "Prints configuration", 16384){ 
  if(argc < 4){
    usage(argv[0]);
    return 0;
  }
  if(strcmp("file", argv[1]) != 0 && strcmp("host", argv[1]) != 0){
    usage(argv[0]);
    return 0;
  }
  
  if(strcmp("host", argv[1]) == 0 && argc < 5){
    usage(argv[0]);
    return 0;
  }
  
  ConfigRetriever c; 
  struct ndb_mgm_configuration * p = 0;

  if(strcmp("host", argv[1]) == 0){
    int verId = 0;
    if(argc > 5)
      verId = atoi(argv[5]);
    
    ndbout << "Getting config from: " << argv[2] << ":" << atoi(argv[3]) 
	   << " NodeId =" << atoi(argv[4]) 
	   << " VersionId = " << verId << endl;
    
    p = c.getConfig(argv[2], 
		    atoi(argv[3]), 
		    verId);
  } else if (strcmp("file", argv[1]) == 0){
    int verId = 0;
    if(argc > 4)
      verId = atoi(argv[4]);
    
    ndbout << "Getting config from: " << argv[2]
	   << " NodeId =" << atoi(argv[3]) 
	   << " VersionId = " << verId << endl;
    
    p = c.getConfig(argv[2], atoi(argv[3]), verId);
  }
  
  if(p != 0){
    //
    free(p);
  } else {
    ndbout << "Configuration not found: " << c.getErrorString() << endl;
  }

  return 0;
}
