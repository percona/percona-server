#! /usr/bin/env python
# -*- mode: python; indent-tabs-mode: nil; -*-
# vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
#
# Copyright (C) 2012 Valentine Gostev 
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

"""matrix_manager.py
   class which imports opt-matrix option set and stores in a separate dict

"""

class matrixManager:
    def __init__(self, variables):
        if not variables['optmatrix']:
            return None
        matrix_list=[]
        for i in variables['optmatrix'].split(','):
            option_tuple = i.split('=')[0], i.split('=')[1]
            matrix_list.append(option_tuple)
        self.option_matrix = dict(matrix_list)

    def matrix_check_req(self, matrix_reqlist):
        """Check requirements method assures that options required
        for test were provided to --opt-matrix
        
        """
        while matrix_reqlist != []:
          req_shift = matrix_reqlist.pop()
          if not self.option_matrix[req_shift]:
            try:
              raise KeyError('The required option is missing from opt-matrix')
            except KeyError:
              print('Option %s is missing' %(req_shift))
              raise



