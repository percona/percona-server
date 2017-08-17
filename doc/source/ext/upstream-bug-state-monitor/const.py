#!/usr/bin/env python3
"""This module contains constants and configuration options
which do not change through the life time of the program

:copyright: Copyright 2017 by Percona Inc.
:license: GPL3, see LICENSE for details.
"""

__all__ = [
    "BugAttrValue",
    "BugFixFormat",
    "BugState",
    "BugStateInfo",
    "Default",
    "FieldNames",
    "Prefixes",
    "Remote"]


class Default:

    class UpstreamBugFixes:
        DIR = "../../"
        FILE_NAME = "upstream-bug-fixes.rst"
        FILE_ENCODING = "utf-8"
        # If the COPY_EXTENSION constant is empty, the original upstream
        # bug fixes file will be overwritten!
        COPY_EXTENSION = "~"

class Prefixes:

    @staticmethod
    def extract_value(prefix, investigatable):
        """Extracts the value part from an 'investigatable` string.

        :params: prefix - the prefix like
        ':bug:`123`'. The extracted value might be a number like thing
        like a version number, for example

        :status: implemented & tested & no unit tests
        """

        value = None

        decorated_prefix = "{}{}{}".format(Prefixes.Token.LEFT_NAME_MARK,
                                           prefix,
                                           Prefixes.Token.RIGHT_NAME_MARK)

        if decorated_prefix == investigatable[0:len(decorated_prefix)]:

            value = investigatable[len(decorated_prefix):]

            l_delim = Prefixes.Token.LEFT_VALUE_MARK
            r_delim = Prefixes.Token.RIGHT_VALUE_MARK

            value = value.partition(l_delim)[-1].rpartition(r_delim)[0]

        return value

    class Token:
        """Symbols which delimit prefixes in data files"""
        LEFT_NAME_MARK = ":"
        RIGHT_NAME_MARK = ":"
        LEFT_VALUE_MARK = "`"
        RIGHT_VALUE_MARK = "`"

    class VersionNumber:
        """The names of roles (as defined in the reStructuredText
        standard)"""
        MYSQL_BUG = "mysqlbug"
        LAUNCHPAD_BUG = "bug"
        LAUNCHPAD_BP = "bug"
        JIRA_BUG = "bug"
        RELEASE_NUMBER = "rn"


class BugFixFormat:

    class Header:
        # Matches: '.. _upstream_bug_fixes:'
        DOC_REF_PATTERN = "[.]{2}\s+_(\S+)[:]"
        DOC_REF_PREFIX = ".. _"
        DOC_REF_ENDING = ":"

        DECORATION_TOKEN = "="
        LINE_ABOVE_PATTERN = "([{}]+)".format(DECORATION_TOKEN)
        LINE_BELOW_PATTERN = "([{}]+)".format(DECORATION_TOKEN)
        # Like in "|Percona Server|"
        PLACE_HOLDER_DELIMITER_TOKEN = "|"
        TITLE_TEMPLATE = "List of upstream |MySQL| bugs fixed in |{}| {}"

    class Line:
        DELIMITER_TOKEN = "-"
        CORNER_TOKEN = "+"
        LEFT_BORDER_MARK = "|"
        RIGHT_BORDER_MARK = "|"
        PADDING_CHAR = " "
        CONTENT_WIDTH = 109

    class Name:
        LEFT_BORDER_MARK = ":"
        RIGHT_BORDER_MARK = ":"

    class Value:
        """The static names of columns are found in the Prefixes
        constant"""
        # A regular expression --- for " - "
        INTERNAL_SEPARATOR = "\s+[-]\s+"
        # Cannot reference the constants of the
        # `Name` class; therefore, the required
        # symbols are placed at runtime.
        NAME_VALUE_SEPARATOR = "{}\s+"
        DATE_STAMP_PATTERN = "([0-9]{4}-[0-9]{2}-[0-9]{2})"


class FieldNames:
    """This class is useful when capturing information from parsed
    data. Its subclasses contain constants for names of fields
    or other static text which can serve as an anchor"""

    class BugFixFields:
        """The constants in this class capture names of bug fix related
        information.

        The first component of tuple is the key which should be reported
        by the parser; the second component is the normalized literal
        string.

        The normalization includes bringing text to lower case and
        replacing consecutive spaces with one space. It is up to the
        parser to implement these and maybe additional rules.
        """
        UPSTREAM_BUG = ("Upstream Bug", "upstream bug")
        LAUNCHPAD_BUG = ("Launchpad Bug", "launchpad bug")
        LAUNCHPAD_BP = ("Launchpad BP", "launchpad bp")
        UPSTREAM_STATE = ("Upstream State", "upstream state")
        FIX_RELEASED = ("Fix Released", "fix released")
        UPSTREAM_FIX = ("Upstream Fix", "upstream fix")

    class UpstreamBugReport:
        """The constants in this class capture the names of fields in
        the bug report returned by the MySQL bug reporting system.

        The value is the literal string which can be used to detect
        interesting components.
        """
        REPORT_ID = "bugheader"
        BUG_NUMBER_PREFIX = "Bug #"
        STATE = "State:"
        VERSION = "Version:"


class BugStateInfo:

    def __init__(self, output, is_final=False):
        """Creates a new bug state.
        
        The 'output' parameter refers to the textual representation 
        of the bug state in output. 
        The 'is_final' parameter tells if a date stamp is needed.
        If there are multiple versions of the bug state name, 
        use the 'add_variant' method to add them 
        one at a time.
        """
        self._output = output.strip()
        self._variants = []
        self._variants.append(self._output.lower())
        self._is_final = is_final

    def add_variant(self, variant):
        variant = variant.strip().lower()
        self._variants.append(variant)

    @property
    def output(self):
        return self._output

    @property
    def variants(self):
        return self._variants

    @property
    def is_final(self):
        return self._is_final


class BugState:
    """The 'is_final' parameter in the instantiated BugStateInfo class
    tells if the given state is final or not.
    If the bug state is final it does not carry a date stamp."""
    OPEN = BugStateInfo("Open")
    VERIFIED = BugStateInfo("Verified")

    NOT_REPRODUCIBLE = BugStateInfo("Can't Repeat")
    NOT_REPRODUCIBLE.add_variant("can't reproduce")
    NOT_REPRODUCIBLE.add_variant("cannot reproduce")
    NOT_REPRODUCIBLE.add_variant("not reproducible")

    CLOSED = BugStateInfo("Closed", is_final=True)
    REJECTED = BugStateInfo("Not a bug", is_final=True)
    UNKNOWN = BugStateInfo("N/A", is_final=True)
    NOT_FIXABLE = BugStateInfo("Won't fix", is_final=True)


class BugAttrValue:
    NOT_AVAILABLE = "N/A"
    DATE_STAMP_DECORATION = " (checked on {})"
    DATE_STAMP_FORMAT = "{:04}-{:02}-{:02}"


class Remote:

    class Connection:
        REMOTE_CATALOG = "https://bugs.mysql.com"
        REMOTE_AGENT = "bug.php?id={}"
        REMOTE_URL = "{}/{}".format(REMOTE_CATALOG, REMOTE_AGENT)

    class Mark:
        """A mark here is any static text, such as non-changeable text
        on web page, which helps locate relevant content"""
        CONTAINER_ID = "bugheader"
        BUG_ID = "Bug #"
        STATE = "Status:"
        VERSION = "Version:"
