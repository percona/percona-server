#!/usr/bin/env python3
""" This program update the *upstream_bug_fixes.rst* file based on the
information stored remotedly. This program aims to fully implement the
specification which defines the structure of *upstream_bug_fixes* as
well as the processing rules.

Although the file is in the *reStructuredText* format, it is a
deliberate decision to use a custom parser. The reasons are the
following:

1. *reStructuredText* format is only useful for the output. This format
   is not convenient as the storage format. YAML, or CSV would suit
   better to store the information of *upstream_bug_fixes*.
2. The the structure of the *upstream_bug_fixes* is not complex and is
   easy parse.
3. By implementing a custom parser, the logical structure of the
   information stored in *upstream_bug_fixes* is better visible and,
   therefore, it is easier to reason about.
4. The custom parser may be quite useful for similar processing: when
   a local file must be updated from one or more external sources.

.. note:: This program requires Python3

:copyright: Copyright 2017 by Percona Inc.
:license: GPL3, see LICENSE for details.
"""

from os import sep as os_sep
from os.path import isfile
from re import match, split as re_split, search as re_search
from datetime import datetime

from const import (BugAttrValue,
                   BugFixFormat,
                   BugState,
                   BugStateInfo,
                   Default,
                   FieldNames,
                   Prefixes)

from errors import TitleLineError, FilePathError, UnknownStateError
from remote import UpstreamBugCatalog

__version__ = {"major": 1, "minor": 0, "update": 1}

__all__ = [
    "BugFix",
    "BugFixAttr",
    "BugFixCatalog",
    "BugFixCatalogHeader",
    "LaunchpadBug",
    "UpstreamBug"]


class BugFixCatalogHeader:
    """This class contains attributes of the header and methods for
    parsing and outputting them. """

    @staticmethod
    def parse(header_lines, header_components):
        """This static method extracts header components from the
        supplied 'header_lines' parameter.

        This method works recursively on the lines of the
        'header_lines'. It collects the header components into the
        'header_components' parameter. As soon as all header components
        have been collected, this method terminates and returns the
        components.

        :returns: a tuple with four elements:

        1. document reference
        2. document title
        3. product reference
        4. version number

        If these components cannot be retrieved this method terminates
        with an exception.
        """
        # Capturing the starting point with no components collected and
        # the 'header_lines' being non-empty.
        if not header_components and header_lines:
            # The document reference must be
            # the first collected component in
            # the document. Otherwise,
            # the method exists with an exception.
            doc_ref = match(BugFixFormat.Header.DOC_REF_PATTERN,
                            header_lines[0])

            if doc_ref:
                header_components.append(doc_ref.groups()[0])
                return BugFixCatalogHeader.parse(header_lines[2:],
                                                 header_components)
            else:

                raise TitleLineError
        # It only makes sense to extract other components if the
        # document reference has been retrieved
        elif len(header_components) == 1 and header_lines:

            title_line = header_lines[0].strip()

            if not match(BugFixFormat.Header.LINE_ABOVE_PATTERN,
                         title_line) and title_line:

                header_components.append(title_line)

                elements = (title_line.split(BugFixFormat
                                             .Header
                                             .PLACE_HOLDER_DELIMITER_TOKEN))
                header_components.append(elements[-2])
                header_components.append(elements[-1])

                return header_components

            else:

                return BugFixCatalogHeader.parse(header_lines[1:],
                                                 header_components)
        else:
            raise TitleLineError

    def __init__(self, document_header):
        """ Receives a list of strings which belong to the header
        according to the header definition in the specification.
        """

        hd = BugFixCatalogHeader
        doc_ref, title, product, version = hd.parse(document_header, [])

        self._title = title
        self._doc_ref = doc_ref
        self._product = product
        self._version = version

    def __str__(self):
        """Formats the header according to the specification. """
        # Improving readability
        header = BugFixFormat.Header
        # Making the standard heading
        title = header.TITLE_TEMPLATE.format(self._product,
                                             self._version)
        title_width = len(title)
        # In reStructuredText the decoration lines should be at least as
        # wide as the text in the heading.
        decoration_line = header.DECORATION_TOKEN * title_width
        doc_ref = "".join([BugFixFormat.Header.DOC_REF_PREFIX,
                           self._doc_ref,
                           BugFixFormat.Header.DOC_REF_ENDING])

        return "\n".join([doc_ref, "",
                          decoration_line,
                          title,
                          decoration_line, ""])


class BugFixCatalog:
    """This class represents the upstream_bug_fixes.rst file. Logically,
    this file consists of two components: the header and a bunch of
    entries.

    Entries are instances of the BugFix class. """

    def __init__(self, file_name=None, line_delimiter=None):
        """ Checks if the file is available and parses it.

        The valid file is viewed as a collection of lines which belong
        together until the line delimiter is encountered. The very first
        element is the header. All other elements are the entries.
        """
        fmt = BugFixFormat.Line

        self._line_delimiter = line_delimiter or "".join([
            fmt.CORNER_TOKEN,
            fmt.DELIMITER_TOKEN * fmt.CONTENT_WIDTH,
            fmt.CORNER_TOKEN])

        # Only checking if the supplied string is an existing file.
        self._file_name = self._validate(file_name)
        # The values of the entries and header properties
        # do not require the given types. The given types
        # are for documenting the return type of the "_parse"
        # private method.

        # The entries is a dictionary where the key is an integer
        # referring to the upstream number. The value is an instance of
        # the BugFix class.
        self._entries = dict()
        # A list to capture the initial order of bug fix entries. The
        # following attribute enables printing the entries either in the
        # order in which they were arranged in the input file or sorted
        # numerically. With the sorted enabled (default), the entry with
        # the greatest number appears at the top.
        self._entry_ids = []
        # The header will evantually be an instance of the
        # BugFixCatalogHeader( ) class. In one upstream_bug_fixes file
        # there may be only one header.
        self._header = None

        self._header, self._entries = self._parse()

    def _validate(self, file_name):
        """This function either returns a string
        which represents the path to the
        `upstream_bug_fixes` file or terminates the program
        with an exception: there is no point to continue
        if the file is not found. """
        validated = None
        default = os_sep.join([Default.UpstreamBugFixes.DIR,
                               Default.UpstreamBugFixes.FILE_NAME])

        if file_name is not None and isfile(file_name):
            validated = file_name

        elif isfile(default):
            validated = default

        else:

            raise FilePathError

        return validated

    def _parse(self):
        """Ensures that the file has the right structure (according to
        the specification) and fills the essential properties of the
        current instance

        :returns:
        1. an instance of BugFixCatalogHeader() instantiated
           based on the lines which belong to the header.
        2. a dictionary where the keys are the mysqlbug identifers
           and values are instances of the BugFix class.
        """

        delimiter = self._line_delimiter
        # - The accumulated list of lines which belong to tickets
        tickets = []
        # - The lines which belong to the given ticket
        curr_ticket = []
        # - A list of bug fixes where each item is an instance of
        #   BugFix. The dictionary key is the `upstream bug fix
        #   number`.
        entries = dict()

        file_encoding = Default.UpstreamBugFixes.FILE_ENCODING
        with open(self._file_name, "r",
                  encoding=file_encoding) as upstream_bug_fixes:

            for line in upstream_bug_fixes:
                line = line.strip()

                if line != delimiter:
                    curr_ticket.append(line)

                else:
                    tickets.append(curr_ticket)
                    curr_ticket = []

        for _ in tickets[1:]:
            bug_fix = BugFix(_)
            entry_id = int(bug_fix.upstream_bug.number)
            entries[entry_id] = bug_fix
            self._entry_ids.append(entry_id)

        return BugFixCatalogHeader(tickets[0]), entries

    @property
    def header(self):
        return self._header

    @property
    def entries(self):
        return self._entries

    def save(self, sorted=True):
        """Saves the changes to the file which was loaded by this
        instance.

        When saving the entries, make sure that the properties
        appear in the expected order.
        To format the header, use the defined __str__ method
        of the BugReportCatalogHeader class.
        To format the entries, use the defined __str__ method
        of the BugFix class. """
        header = str(self.header)

        if sorted:

            bug_numbers = self._entry_ids.copy()
            bug_numbers.sort()
            bug_numbers.reverse()

        else:

            bug_numbers = self._entry_ids

        # Forming a delimiter line
        ln = BugFixFormat.Line
        delimiter = "".join(["\n",
                             ln.CORNER_TOKEN,
                             ln.DELIMITER_TOKEN * ln.CONTENT_WIDTH,
                             ln.CORNER_TOKEN,
                             "\n"])
        # Collecting entries
        entries = delimiter.join([str(self._entries[_])
                                  for _ in bug_numbers])
        # Joining all components on the delimiter line
        document = delimiter.join([header, entries, ""])
        # Readibility convenience
        file_encoding = Default.UpstreamBugFixes.FILE_ENCODING
        copy_extension = Default.UpstreamBugFixes.COPY_EXTENSION
        file_name = "{}{}".format(self._file_name, copy_extension)
        # Writing the document to the output file
        with open(file_name, "w",
                  encoding=file_encoding) as upstream_bug_fixes:

            upstream_bug_fixes.write(document)


class BugFixAttr:
    """This class represents an attribute of an bug fix entry.

    The constructor of the inheriting classes may introduce other
    properties which break the name or value (more likely) into more
    specific attributes.
    """
    def __init__(self, attr_line):
        """Parses the supplied attribute line and retrieves its
        essential components """

        name, value = self._parse(attr_line)
        # `name` without leading or trailing `:` ---
        self._name = name
        # `value` without any leading or trailing white space
        self._value = value

    @property
    def name(self):
        return self._name

    @property
    def value(self):
        return self._value

    def _parse(self, attr_line):
        """Extracts the name and value of an attribute
        from the supplied line.
        """

        fline = BugFixFormat.Line
        fval = BugFixFormat.Value
        fname = BugFixFormat.Name

        attr_line = (attr_line.strip()
                              .strip(fline.LEFT_BORDER_MARK)
                              .strip(fline.RIGHT_BORDER_MARK))
        name, value = (re_split(fval.NAME_VALUE_SEPARATOR.format(
            fname.RIGHT_BORDER_MARK), attr_line, maxsplit=1))

        return (name.strip(fname.LEFT_BORDER_MARK),
                value.strip(fline.PADDING_CHAR))


class UpstreamBug:
    """This class represents a collection of attribute lines
    related to the upstream bug
    """

    def __init__(self, info, state, fix_version):

        self._info = info

        num, desc = re_split(BugFixFormat.Value.INTERNAL_SEPARATOR,
                             self._info, maxsplit=1)
        self._number = Prefixes.extract_value(
            Prefixes.VersionNumber.MYSQL_BUG,
            num)
        self._description = desc.strip()

        status, date_stamp = self._parse_state(state)
        self._status = None
        self.status = status

        if self.status.is_final:
            self._date_stamp = None
        else:
            self._date_stamp = date_stamp

        self._fix_version = fix_version

    def _parse_state(self, status_value):
        """Breaks the provided state description
        into its components :returns: a tuple
        the first element of which refers
        to the token which represents the given state;
        the second component is the date stamp in the format YYYY-MM-DD
        """
        date_stamp = None
        status_value = [_ for _ in status_value.partition("(") if _]
        status = status_value[0]

        date_stamp = re_search(BugFixFormat.Value.DATE_STAMP_PATTERN,
                               status_value[-1])
        if date_stamp:
            date_stamp = date_stamp.groups()[0]
        else:
            date_stamp = None

        return status, date_stamp

    def update_date_stamp(self):

        if self.status.is_final:
            self._date_stamp = None
        else:
            d = datetime.today()
            stamp = BugAttrValue.DATE_STAMP_FORMAT.format(d.year,
                                                          d.month,
                                                          d.day)
            self._date_stamp = stamp

    @property
    def fix_version(self):
        return self._fix_version

    @property
    def number(self):
        return self._number

    @property
    def description(self):
        return self._description

    @property
    def info(self):
        return self._info

    @property
    def status(self):
        return self._status

    @status.setter
    def status(self, new_status):

        if isinstance(new_status, BugStateInfo):
            self._status = new_status

        else:
            new_status = new_status.strip().lower()

            if new_status in BugState.UNKNOWN.variants:
                new_status = BugState.UNKNOWN
            elif new_status in BugState.OPEN.variants:
                new_status = BugState.OPEN
            elif new_status in BugState.VERIFIED.variants:
                new_status = BugState.VERIFIED
            elif new_status in BugState.CLOSED.variants:
                new_status = BugState.CLOSED
            elif new_status in BugState.REJECTED.variants:
                new_status = BugState.REJECTED
            elif new_status in BugState.NOT_REPRODUCIBLE.variants:
                new_status = BugState.NOT_REPRODUCIBLE
            elif new_status in BugState.NOT_FIXABLE:
                new_status = BugState.NOT_FIXABLE
            else:
                raise UnknownStateError

            self._status = new_status
            self.update_date_stamp()

    @property
    def date_stamp(self):
        return self._date_stamp

    @date_stamp.setter
    def date_stamp(self, new_date_stamp):
        if match(BugFixFormat.Value.DATE_STAMP_FORMAT,
                 new_date_stamp):
            self._date_stamp = new_date_stamp

    @property
    def state(self):
        formatted_state = ""
        if not self.status.is_final:
            stamp = (BugAttrValue
                     .DATE_STAMP_DECORATION
                     .format(self._date_stamp))
            formatted_state = "{}{}".format(self.status.output, stamp)
        else:
            formatted_state = self.status.output

        return formatted_state


class LaunchpadBug:
    """This class represents a collection of attribute lines related to
    the launchpad bug"""

    def __init__(self, info, fix_released, is_blueprint=False):

        num = Prefixes.VersionNumber

        self._number = Prefixes.extract_value(num.LAUNCHPAD_BUG,
                                              info)
        fix = Prefixes.extract_value(num.RELEASE_NUMBER,
                                     fix_released) or fix_released
        self._fix_released = fix
        self._info = info
        self._is_blueprint = is_blueprint

    @property
    def info(self):
        return self._info

    @property
    def number(self):
        return self._number

    @property
    def fix_released(self):
        return self._fix_released

    @property
    def is_blueprint(self):
        return self._is_blueprint


class BugFix:
    """This class represents a combination of
    related upstream bug and launchpad bug."""

    @staticmethod
    def format_output(name, value):

        l_line_border = BugFixFormat.Line.LEFT_BORDER_MARK
        r_line_border = BugFixFormat.Line.RIGHT_BORDER_MARK
        l_name_border = BugFixFormat.Name.LEFT_BORDER_MARK
        r_name_border = BugFixFormat.Name.RIGHT_BORDER_MARK
        entry_width = BugFixFormat.Line.CONTENT_WIDTH

        name = "{}{}{} ".format(l_name_border,
                                name,
                                r_name_border)

        content = "{}{}".format(name, value)
        content_width = len(content)
        padding_width = entry_width - content_width
        padding = BugFixFormat.Line.PADDING_CHAR * padding_width

        return "{}{}{}{}".format(l_line_border,
                                 content,
                                 padding,
                                 r_line_border)

    def __init__(self, lines):
        """Expects a list of lines extracted from a valid file.
        It is up to the implementation whether to terminate
        if the file contains defects, such as the following:

        - At least one of the expected lines is not found
        - Two or more duplicates are encountered
        - Duplicating keys
        - Not valid values
        """

        lines = [BugFixAttr(_) for _ in lines]

        self._entry = dict()
        self._upstream_bug = None
        self._launchpad_bug = None

        for _ in lines:
            self._entry[_.name.lower()] = _.value

        fields = FieldNames.BugFixFields

        upstream_bug = self._entry[fields.UPSTREAM_BUG[1]]
        upstream_state = self._entry[fields.UPSTREAM_STATE[1]]
        upstream_fix = self._entry[fields.UPSTREAM_FIX[1]]
        fix_released = self._entry[fields.FIX_RELEASED[1]]

        # The launch pad bug attribute may either refer to a registered
        # bug or a blue print
        launchpad_bug = None

        self._upstream_bug = UpstreamBug(
            info=upstream_bug,
            state=upstream_state,
            fix_version=upstream_fix)

        if self._entry.get(fields.LAUNCHPAD_BUG[1]):
            launchpad_bug = self._entry[fields.LAUNCHPAD_BUG[1]]

            self._launchpad_bug = LaunchpadBug(
                info=launchpad_bug,
                fix_released=fix_released)

        elif self._entry.get(fields.LAUNCHPAD_BP[1]):
            launchpad_bug = self._entry[fields.LAUNCHPAD_BP[1]]

            self._launchpad_bug = LaunchpadBug(
                info=launchpad_bug,
                fix_released=fix_released,
                is_blueprint=True)

    def __str__(self):
        """ Formats the string ready for output:

        1. recreates line borders
        2. applies attribute name formatting
        3. writes values correctly with subcomponents in the correct order.
        """

        f = FieldNames.BugFixFields

        upstream_bug = BugFix.format_output(f.UPSTREAM_BUG[0],
                                            self._upstream_bug.info)
        if self._launchpad_bug.is_blueprint:
            launchpad_bug = BugFix.format_output(f.LAUNCHPAD_BP[0],
                                                 self._launchpad_bug.info)
        else:
            launchpad_bug = BugFix.format_output(f.LAUNCHPAD_BUG[0],
                                                 self._launchpad_bug.info)
        upstream_state = BugFix.format_output(f.UPSTREAM_STATE[0],
                                              self._upstream_bug.state)

        fix_released = BugFix.format_output(f.FIX_RELEASED[0],
                                            self._launchpad_bug.fix_released)
        upstream_fix = BugFix.format_output(f.UPSTREAM_FIX[0],
                                            self._upstream_bug.fix_version)

        return "\n".join([upstream_bug,
                          launchpad_bug,
                          upstream_state,
                          fix_released,
                          upstream_fix])

    @property
    def entry(self):
        return self._entry

    @property
    def upstream_bug(self):
        return self._upstream_bug

    @property
    def launchpad_bug(self):
        return self._launchpad_bug


if __name__ == "__main__":

    bug_fix_catalog = BugFixCatalog()
    mysql_bug_catalog = UpstreamBugCatalog()
    mysql_bug_catalog.update(bug_fix_catalog.entries)
    bug_fix_catalog.save()
