#!/usr/bin/env python3
""" This module contains classes which enable communication with
external resources 

:copyright: Copyright 2017 by Percona Inc.
:license: GPL3, see LICENSE for details.
"""

from requests import Session as req_session
from bs4 import BeautifulSoup as soup

from const import Remote, BugAttrValue


class UpstreamBugCatalog:
    """This class is responsible for extracting information from
    a remote source """
    def __init__(self):

        self._remote_url = Remote.Connection.REMOTE_URL
        self._session = req_session()

    def update(self, entries):

        for _ in entries:

            bug = entries[_].upstream_bug
            query = self._remote_url.format(bug.number)
            page_contents = self._session.get(query).text
            context = (soup(page_contents,
                            "html.parser")
                       .find(id=Remote.Mark.CONTAINER_ID))
            try:
                context = [_ for _ in context.get_text().split("\n") if _]
                status_location = context.index(Remote.Mark.STATE)
                status_value = context[status_location + 1].strip()
                bug.status = status_value
            except AttributeError:
                bug.status = BugAttrValue.NOT_AVAILABLE
            bug.update_date_stamp()

    @property
    def remote_url(self):
        return self._remote_url
