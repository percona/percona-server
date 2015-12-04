#! /usr/bin/env python
# -*- mode: python; indent-tabs-mode: nil; -*-
# vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
#
# Copyright (C) 2013 Percona LLC and/or its affiliates
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
import re
import grp

from lib.util.mysqlBaseTestCase import mysqlBaseTestCase
from lib.util.mysql_methods import execute_cmd


server_requirements = [[]]
servers = []
server_manager = None
test_executor = None
pamcfg = '/etc/pam.d/mysqld'

def group_exists(groupname):
    try:
        grp.getgrnam(groupname)[0]
    except KeyError:
        return False
    return True

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
        if test_executor.matrix_manager.option_matrix['pam_user']:
          pam_user = test_executor.matrix_manager.option_matrix['pam_user']
        else:
          pam_user = 'pamuser'

        groups = ['grp%d' % (n) for n in xrange(3)]
        users = ['user1%d' % (n) for n in xrange(3)]

        for grp in groups:
            if not group_exists(grp):
                subprocess.call(["groupadd", grp])

        # Create UNIX system account
        if not test_executor.system_manager.user_exists(pam_user):
            subprocess.call(["useradd", pam_user, "-g", groups[0], "-G", ",".join(groups[1:]) ])
        else:
            subprocess.call(["usermod", "-g", groups[0], "-G", ",".join(groups[1:]), pam_user ])

        # Create PAM config
        if (os.path.isfile(pamcfg)):
            os.remove(pamcfg)

        pamcfg_fh = open("/etc/pam.d/mysqld", "wb")
        pamcfg_fh.write("auth\trequired\tpam_permit.so\n")
        pamcfg_fh.write("account\trequired\tpam_permit.so\n")
        pamcfg_fh.close();

        master_server.stop()

        # setup plugin, users, privileges
        groups.reverse()
        groups = [ "grp21", "grp22" ] + groups
        users = [ "usr21", "usr22" ] + users
        queries = [ "INSTALL PLUGIN auth_pam SONAME 'auth_pam.so';" ] + \
                  [ "CREATE USER '%s'@'localhost';" % (user) for user in users ] + \
                  [ "CREATE USER ''@'' IDENTIFIED WITH auth_pam AS 'mysqld, %s';" \
                    % ( ",".join([ user + "=" + group for user, group in zip(groups, users) ] ) ) ] + \
                  [ "GRANT PROXY ON '%s'@'localhost' TO ''@'';" % (user) for user in users ] + \
                  [ "SELECT user, host, authentication_string FROM mysql.user;", \
                    "FLUSH PRIVILEGES;", "SHOW VARIABLES LIKE 'plugin%'" ]

        master_server.server_options.append('--plugin-dir=%s' %(test_executor.matrix_manager.option_matrix['pam_plugin_dir']))

        master_server.start()
        self.assertEqual( master_server.status, 1, msg = 'Server failed to restart')

        cmd = "%s --protocol=tcp --port=%d -uroot -e \"%s\"" %(master_server.mysql_client
              , master_server.master_port
              , "\n".join(queries) )
        retcode, output = execute_cmd(cmd, output_path, None, True)

        query = "SELECT CONCAT(USER(), CURRENT_USER(), @@PROXY_USER) as res;"
        expected_result = "res%s@localhostuser10@localhost''@''" % (pam_user)
        cmd = "%s --plugin-dir=/usr/lib/mysql/plugin/ --protocol=tcp --port=%d --user=%s --password=\'\' -e \"%s\" test" %(master_server.mysql_client
              , master_server.master_port
              , pam_user
              , query )
        retcode, output = execute_cmd(cmd, output_path, None, True)
        output = re.sub(r'\s+', '', output)
        self.assertEqual(retcode, 0, msg = output)
        self.assertEqual(output, expected_result, msg = "%s || %s" %(output, expected_result))
