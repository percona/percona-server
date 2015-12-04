# -*- coding: utf-8 -*-
"""
    The Percona Server domain.

    :copyright: Copyright 2011 by Percona Inc.
    :license: GPL3, see LICENSE for details.
"""

import re
import string

from docutils import nodes
from docutils.parsers.rst import directives

from sphinx import addnodes
from sphinx.roles import XRefRole
from sphinx.locale import l_, _
from sphinx.domains import Domain, ObjType, Index
from sphinx.directives import ObjectDescription
from sphinx.util.nodes import make_refnode
from sphinx.util.compat import Directive
from sphinx.util.docfields import Field, GroupedField, TypedField


# RE to split at word boundaries
wsplit_re = re.compile(r'\W+')
tern_re = re.compile(r'.*(\.|\:\:).*(\.).*')

class PSschemaObject(ObjectDescription):
    """
    Description of a general PS object.
    """
    option_spec = {
        'noindex': directives.flag,
    }


    def handle_signature(self, sig, signode):
        """Transform a PSschema signature into RST nodes."""
        # first try the function pointer signature regex, it's more specific

        name = sig

        signode += addnodes.desc_name('', '')

        objectname = self.env.temp_data.get('psdom:object')
        ot = self.objtype
        ws = wsplit_re.split(sig)

        if ot == 'db':
            sig_prefix = "database "
        else:
            sig_prefix =  ot + " "

        signode += addnodes.desc_annotation(sig_prefix, sig_prefix)

        # for part in filter(None, ws):
        #     tnode = nodes.Text(part, part)
        #     pnode = addnodes.pending_xref(
        #         '', refdomain='psdom', reftype='type', reftarget=part,
        #         modname=None, classname=None)
        #     pnode = tnode
        #     signode += pnode
        
        if len(ws) > 2:
            dbname, tablename, columnname = ws
            name = columnname
            fullname = tablename + "." + columnname
        elif len(ws) == 2:
            if ot == 'table':
                dbname, tablename = ws
                dbname += "."
                name = tablename
                signode['table'] = tablename
                signode += addnodes.desc_addname(dbname, dbname)
                signode += addnodes.desc_name(tablename, tablename)
                # fullname = dbname + "." + tablename 
                fullname = tablename
            if ot == 'column':
                tablename, columnname = ws
                tablename += " "
                signode += addnodes.desc_addname(tablename, tablename)
                signode += addnodes.desc_name(columnname, columnname)
                signode['table'] = tablename
                fullname = tablename + "." + columnname
        else:
            if ot == 'table':
                tablename = ws[0]
                signode['table'] = tablename
                dbname = self.options.get(
                    'db', self.env.temp_data.get('psdom:db'))
                dbname += "."
                self.env.temp_data['psdom:table'] = tablename
                signode += addnodes.desc_addname(dbname, dbname)
                signode += addnodes.desc_name(tablename, tablename)
                signode['table'] = tablename
            if ot == 'column':
                columnname = ws[0]
                signode['column'] = columnname
                tablename = self.options.get(
                    'table', self.env.temp_data.get('psdom:table'))
                tablename += "."
                signode += addnodes.desc_addname(tablename, tablename)
                signode += addnodes.desc_name(columnname, columnname)
            if ot == 'db':
                dbname = ws[0]
                signode['db'] = dbname
                name = dbname
                self.env.temp_data['psdom:db'] = dbname
                signode += addnodes.desc_name(dbname, dbname)
            fullname = ws[0]

        signode['fullname'] = fullname

        return fullname

    def get_index_text(self, name):
        if self.objtype == 'db':
            return _('%s (database)') % name
        elif self.objtype == 'table':
            return _('%s (table)') % name
        elif self.objtype == 'column':
            return _('%s (column)') % name
        else:
            return ''

    def add_target_and_index(self, name, sig, signode):
        # note target
        if name not in self.state.document.ids:
            signode['names'].append(name)
            signode['ids'].append(name)
            signode['first'] = (not self.names)
            self.state.document.note_explicit_target(signode)
            inv = self.env.domaindata['psdom']['objects']
            if name in inv:
                self.env.warn(
                    self.env.docname,
                    'duplicate PSschema object description of %s, ' % name +
                    'other instance in ' + self.env.doc2path(inv[name][0]),
                    self.lineno)
            inv[name] = (self.env.docname, self.objtype)

        indextext = self.get_index_text(name)
        if indextext:
            self.indexnode['entries'].append(('single', indextext, name, ''))


class PSconfigObject(ObjectDescription):
    """
    Description of a general PS Config object.
    """
    option_spec = {
        'noindex': directives.flag,
    }

    def handle_signature(self, sig, signode):
        """Transform a PSconfig signature into RST nodes."""
        # first try the function pointer signature regex, it's more specific

        name = sig
        ot = self.objtype

        sig_prefix =  ot + " "
        signode += addnodes.desc_annotation(sig_prefix, sig_prefix)

        signode += addnodes.desc_name(name, name)
        signode['fullname'] = name

        return name

    def get_index_text(self, name):
        if self.objtype == 'option':
            return _('%s (option)') % name
        elif self.objtype == 'variable':
            return _('%s (variable)') % name
        else:
            return ''

    def add_target_and_index(self, name, sig, signode):
        # note target
        if name not in self.state.document.ids:
            signode['names'].append(name)
            signode['ids'].append(name)
            signode['first'] = (not self.names)
            self.state.document.note_explicit_target(signode)
            inv = self.env.domaindata['psdom']['objects']
            if name in inv:
                self.env.warn(
                    self.env.docname,
                    'duplicate PSconfig object description of %s, ' % name +
                    'other instance in ' + self.env.doc2path(inv[name][0]),
                    self.lineno)
            inv[name] = (self.env.docname, self.objtype)

        indextext = self.get_index_text(name)
        if indextext:
            self.indexnode['entries'].append(('single', indextext, name, ''))


class PSTable(PSschemaObject):

    doc_field_types = [
        TypedField('column', label=l_('Columns'), rolename='obj',
                   names=('col', 'column', 'cols'),
                   typerolename='obj', typenames=('paramtype', 'type')),
        Field('inpatch', label=l_('Included in Patch'), has_arg=False,
              names=('inpatch')),
        TypedField('versioninfo', label=l_('Version Info'), rolename='obj',
                   names=('version', 'versioninfo'),
                   typerolename='obj', typenames=('paramtype', 'type')),
    ]

class PSDatabase(PSschemaObject):

    doc_field_types = [
        TypedField('tbl', label=l_('Tables'),
                   names=('tbl', 'table',),
                   typerolename='obj', typenames=('paramtype', 'type')),
        Field('engine', label=l_('Storage Engine'), has_arg=False,
              names=('engine')),
        Field('inpatch', label=l_('Included in Patch'), has_arg=False,
              names=('inpatch')),
    ]

class PSColumn(PSschemaObject):

    doc_field_types = [
        TypedField('coltype', label=l_('Type'), rolename='obj',
                   names=('coltype', 'type'),
                   typerolename='obj', typenames=('paramtype', 'type')),
        Field('inpatch', label=l_('Included in Patch'), has_arg=False,
              names=('inpatch')),
    ]

class PSVariable(PSconfigObject):

    doc_field_types = [
        Field('scope', label=l_('Scope'), has_arg=False,
              names=('scope', 'varscope')),
        Field('cmdline', label=l_('Command Line'), has_arg=False,
              names=('cmdline', 'cline', 'cli')),
        Field('configfile', label=l_('Config File'), has_arg=False,
              names=('conffile', 'configfile', 'conf', 'cfile')),
        Field('dynamic', label=l_('Dynamic'), has_arg=False,
              names=('dynvar', 'dyn')),
        Field('vartype', label=l_('Variable Type'), has_arg=False,
              names=('vartype', 'vtype')),
        Field('default', label=l_('Default Value'), has_arg=False,
              names=('default', 'df')),
        Field('range', label=l_('Range'), has_arg=False,
              names=('range', 'range')),
        Field('allowed', label=l_('Allowed Values'), has_arg=False,
              names=('allowed', 'av')),
        Field('unit', label=l_('Units'), has_arg=False,
              names=('unit', 'un')),
        Field('inpatch', label=l_('Included in Patch'), has_arg=False,
              names=('inpatch')),
        TypedField('versioninfo', label=l_('Version Info'), rolename='obj',
                   names=('version', 'versioninfo'),
                   typerolename='obj', typenames=('paramtype', 'type')),
    ]

class PSOption(PSconfigObject):
    pass

class PSReleaseNotes(Directive):
    """
    Directive to mark description of Release Notes.
    """

    has_content = False
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {
        'platform': lambda x: x,
        'noindex': directives.flag,
    }

    def run(self):
        env = self.state.document.settings.env
        rnname = self.arguments[0].strip()
        noindex = 'noindex' in self.options
        env.temp_data['psdome:rn'] = rnname
        ret = []
        if not noindex:
            env.domaindata['psdom']['objects'][rnname] = \
                (env.docname, self.options.get('platform', ''))
            # make a duplicate entry in 'objects' to facilitate searching for the
            # module in PythonDomain.find_obj()
            env.domaindata['psdom']['objects'][rnname] = (env.docname, 'rn')
            targetnode = nodes.target('', '', ids=['release-' + rnname])
            self.state.document.note_explicit_target(targetnode)
            # the platform and synopsis aren't printed; in fact, they are only used
            # in the modindex currently
            ret.append(targetnode)
            indextext = _('%s (release notes)') % rnname
            inode = addnodes.index(entries=[('single', indextext,
                                             'module-' + rnname, '')])
            ret.append(inode)
        return ret

class PSXRefRole(XRefRole):
    def process_link(self, env, refnode, has_explicit_title, title, target):
        if not has_explicit_title:
            target = target.lstrip('~') # only has a meaning for the title
            # if the first character is a tilde, don't display the module/class
            # parts of the contents
            if title[0:1] == '~':
                title = title[1:]
                dot = title.rfind('.')
                if dot != -1:
                    title = title[dot+1:]
        return title, target


class PerconaServerDomain(Domain):
    """Percona Server domain."""
    name = 'psdom'
    label = 'Percona Server'
    object_types = {
        'db':        ObjType(l_('db'),       'db'),
        'table':     ObjType(l_('table'),    'table'),
        'column':    ObjType(l_('column'),   'column'),
        'option':    ObjType(l_('option'),   'option'),
        'variable':  ObjType(l_('variable'), 'data'),
    }

    directives = {
        'db':       PSDatabase,
        'table':    PSTable,
        'column':   PSColumn,
        'option':   PSVariable,
        'variable': PSVariable,
        'rn':       PSReleaseNotes
    }
    roles = {
        'db' :      PSXRefRole(),
        'table':    PSXRefRole(),
        'column':   PSXRefRole(),
        'option':   PSXRefRole(),
        'variable': PSXRefRole(),
        'rn': PSXRefRole(),
    }
    initial_data = {
        'objects': {},  # fullname -> docname, objtype
    }

    def clear_doc(self, docname):
        for fullname, (fn, _) in self.data['objects'].items():
            if fn == docname:
                del self.data['objects'][fullname]

    def resolve_xref(self, env, fromdocname, builder,
                     typ, target, node, contnode):
        # strip pointer asterisk
        target = target.rstrip(' *')
        if target not in self.data['objects']:
            return None
        obj = self.data['objects'][target]
        return make_refnode(builder, fromdocname, obj[0], target,
                            contnode, target)

    def get_objects(self):
        for refname, (docname, type) in self.data['objects'].iteritems():
            yield (refname, refname, type, docname, refname, 1)

    def find_obj(self, env, obj, name, typ, searchorder=0):
        if name[-2:] == '()':
            name = name[:-2]
        objects = self.data['objects']
        newname = None
        if searchorder == 1:
            if obj and obj + '.' + name in objects:
                newname = obj + '.' + name
            else:
                newname = name
        else:
            if name in objects:
                newname = name
            elif obj and obj + '.' + name in objects:
                newname = obj + '.' + name
        return newname, objects.get(newname)

def setup(app):
    app.add_config_value('databases', {}, 'env')
    app.add_domain(PerconaServerDomain)
