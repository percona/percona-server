#! /usr/bin/env python
# -*- mode: python; indent-tabs-mode: nil; -*-
# vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
#
# Copyright (C) 2011 Patrick Crews
#
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import os
import time
import shutil
import signal
import subprocess

from lib.util.mysqlBaseTestCase import mysqlBaseTestCase
from lib.util.mysql_methods import execute_cmd


server_requirements = [[]]
servers = []
server_manager = None
test_executor = None
pamcfg = '/etc/pam.d/mysqld'

class basicTest(mysqlBaseTestCase):

    def test_pam_basic(self):
        percent_string = '%'
        opt_matrix_req = ['pam_plugin_dir']
        self.servers = servers
        logging = test_executor.logging
        master_server = servers[0]
        output_path = os.path.join(master_server.vardir, 'pam.out')
        test_executor.matrix_manager.matrix_check_req(opt_matrix_req)
        # This is a master
        if (test_executor.matrix_manager.option_matrix['pam_user']):
          pam_user = test_executor.matrix_manager.option_matrix['pam_user']
        else:
          pam_user = 'pamuser'

        # Create UNIX system account
        if (test_executor.system_manager.user_exists(pam_user)):
            pass
        else:
            subprocess.call(["useradd", pam_user])

        # Create PAM config
        if (os.path.isfile(pamcfg)):
            os.remove(pamcfg)

        pamcfg_fh = open("/etc/pam.d/mysqld", "wb")
        pamcfg_fh.write("auth\trequired\tpam_deny.so\n")
        pamcfg_fh.close();

        # Stop server
        master_server.stop()

        # Specify mysql plugin dir
        master_server.server_options.append('--plugin-dir=%s' %(test_executor.matrix_manager.option_matrix['pam_plugin_dir']))
	# Start server with new options
        master_server.start()
        self.assertEqual( master_server.status, 1, msg = 'Server failed to restart')
	# Install plugin
        query = "INSTALL PLUGIN auth_pam SONAME \'auth_pam.so\'"
        expected_result = ''
        cmd = "%s --protocol=tcp --port=%d -uroot -e \"%s\"" %(master_server.mysql_client
              , master_server.master_port
              , query )
        retcode, output = execute_cmd(cmd, output_path, None, True)
        self.assertEqual(retcode, 0, msg = cmd)
        self.assertEqual(output, expected_result, msg = "%s || %s" %(output, expected_result))
	# Create user
        query = "CREATE USER \'%s\'@\'%s\' IDENTIFIED WITH auth_pam;" %(pam_user, percent_string)
        expected_result = ''
        cmd = "%s --protocol=tcp --port=%d -uroot -e \"%s\"" %(master_server.mysql_client
              , master_server.master_port
              , query )
        retcode, output = execute_cmd(cmd, output_path, None, True)
        self.assertEqual(retcode, 0, msg = output)
        self.assertEqual(output, expected_result, msg = "%s || %s" %(output, expected_result))
	# Grant permissions
        query = "GRANT ALL ON test.* TO \'%s\'@\'%s\';" %(pam_user, percent_string)
        expected_result = ''
        cmd = "%s --protocol=tcp --port=%d --user=root -e \"%s\"" %(master_server.mysql_client
              , master_server.master_port
              , query )
        retcode, output = execute_cmd(cmd, output_path, None, True)
        self.assertEqual(retcode, 0, msg = output)
        self.assertEqual(output, expected_result, msg = "%s || %s" %(output, expected_result))
	# Test user login
        query = "SHOW TABLES;"
        expected_result = 'ERROR 1698 (28000): Access denied for user \'%s\'@\'localhost\'\n' %(pam_user)
        cmd = "%s --plugin-dir=/usr/lib/mysql/plugin/ --protocol=tcp --port=%d --user=%s --password=\'\' -e \"%s\" test" %(master_server.mysql_client
              , master_server.master_port
              , pam_user
              , query )
        retcode, output = execute_cmd(cmd, output_path, None, True)
        self.assertEqual(retcode, 1, msg = output)
        self.assertEqual(output, expected_result, msg = "%s || %s" %(output, expected_result))
