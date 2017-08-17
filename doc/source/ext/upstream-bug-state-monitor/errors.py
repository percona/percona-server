#!/usr/bin/env python3
"""This module contains domain specific exceptions.

:copyright: Copyright 2017 by Percona Inc.
:license: GPL3, see LICENSE for details.
"""


class TitleLineError(Exception):
    pass


class FilePathError(Exception):
    pass


class UnknownStateError(Exception):
    pass
