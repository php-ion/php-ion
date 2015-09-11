#! /usr/bin/env python
# encoding: utf-8

##
# @package udpwaflib.gcov_to_clover
# @~english
# A module that converts gcov output files into Atlassian clover XML files.
#
# It is open-sourced at:
# https://bitbucket.org/atlassian/bamboo-gcov-plugin/
#
# @section gcov_to_clover_license License:
#
# Copyright (c) 2013, Martin M Reed
# Copyright (c) 2012, Matt Clarkson
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os
import time
from xml.dom import NotFoundErr
import xml.dom.minidom as xmldom
import re

##
# @~english
# A class that can convert @c gcov output into a clover.xml for parsing by
# Atlassian Bamboo
class gcov_to_clover():
    ##
    # @~english
    # The Clover XML document
    xml = xmldom.Document()

    ##
    # @~english
    # Returns (or creates) an element with an attribute set to a value
    # @param self the object pointer
    # @param node the node to search under
    # @param tag the tag to search for
    # @param attribute the attribute to match
    # @param value the attribute value to match
    # @throws xml.dom.NotFoundError
    # @returns an XML element
    def get_element(self, node, tag, attribute, value):
        ret = None
        elements = node.getElementsByTagName(tag)
        for element in elements:
            if value == element.getAttribute(attribute):
                ret = element
                break
        if not ret:
            raise NotFoundErr('Could not find <%s %s="%s">' %
                (tag, attribute, value))
        return ret

    ##
    # @~english
    # Gets the Clover @c coverage node
    # @param self the object pointer
    # @returns the @c coverage node
    def get_coverage_node(self):
        try:
            coverage_node = self.xml.getElementsByTagName('coverage')[0]
        except IndexError:
            coverage_node = self.xml.createElement('coverage')
            coverage_node.setAttribute('generated',
                str(int(time.time() * 1000)))
            coverage_node.setAttribute('clover', 'gcov_to_clover.py')
            self.xml.appendChild(coverage_node)
        return coverage_node

    ##
    # @~english
    # Gets the Clover @c project node
    # @param self the object pointer
    # @param project_name the project name to find
    # @param project_prefix the prefix for the node name.  Mainly used in
    #                       get_test_project_node
    # @returns the @c project node
    def get_project_node(self, project_name, project_prefix = ''):
        coverage_node = self.get_coverage_node()
        try:
            project_node = self.get_element(coverage_node, 'project', 'name',
                project_name)
        except NotFoundErr:
            project_node = self.xml.createElement('project')
            project_node.setAttribute('name', project_name)
            project_node.setAttribute('timestamp',
                str(int(time.time() * 1000)))
            coverage_node.appendChild(project_node)
        return project_node

    ##
    # @~english
    # Gets the Clover project @c metrics node
    # @param self the object pointer
    # @param project_name the project name to find
    # @param project_prefix the prefix for the node name.  Mainly used in
    #                       get_test_project_metrics_node
    # @returns the project @c metrics node
    def get_project_metrics_node(self, project_name, project_prefix = ''):
        coverage_node = self.get_coverage_node()
        project_node = self.get_project_node(project_name, project_prefix)
        try:
            project_metrics_node = coverage_node.getElementsByTagName(
                'metrics')[0]
        except IndexError:
            project_metrics_node = self.xml.createElement('metrics')
            self.set_project_metric_attributes(project_metrics_node)
            project_node.appendChild(project_metrics_node)
        return project_metrics_node

    ##
    # @~english
    # Gets the Clover @c testproject node
    # @param self the object pointer
    # @param project_name the project name to find
    # @returns the @c testproject node
    def get_test_project_node(self, project_name):
        return self.get_project_node(project_name, 'test')

    ##
    # @~english
    # Gets the Clover test project @c metrics node
    # @param self the object pointer
    # @param project_name the project name to find
    # @returns the test project @c metrics node
    def get_test_project_metrics_node(self, project_name):
        return self.get_project_metrics_node(project_name, 'test')

    ##
    # @~english
    # Gets the Clover @c package node
    # @param self the object pointer
    # @param project_name the project name to find
    # @param package_name the package name to find
    # @returns the @c package node
    def get_package_node(self, project_name, package_name):
        project_node = self.get_project_node(project_name)
        try:
            package_node = self.get_element(project_node, 'package',
                'name', package_name)
        except NotFoundErr:
            package_node = self.xml.createElement('package')
            package_node.setAttribute('name', package_name)
            project_node.appendChild(package_node)
        return package_node

    ##
    # @~english
    # Gets the Clover test package @c metrics node
    # @param self the object pointer
    # @param project_name the project name to find
    # @param package_name the package name to find
    # @returns the package @c metrics node
    def get_package_metrics_node(self, project_name, package_name):
        project_node = self.get_project_node(project_name)
        package_node = self.get_package_node(project_name, package_name)
        try:
            package_metrics_node = package_node.getElementsByTagName(
                'metrics')[0]
        except IndexError:
            package_metrics_node = self.xml.createElement('metrics')
            self.set_package_metric_attributes(package_metrics_node)
            package_node.appendChild(package_metrics_node)
        return package_metrics_node

    ##
    # @~english
    # Gets the Clover @c file node
    # @param self the object pointer
    # @param project_name the project name to find
    # @param package_name the package name to find
    # @param file_path the file path to find
    # @returns the @c file node
    def get_file_node(self, project_name, package_name, file_path):
        project_node = self.get_project_node(project_name)
        package_node = self.get_package_node(project_name, package_name)
        try:
            file_node = self.get_element(package_node, 'file', 'path',
                file_path)
        except NotFoundErr:
            file_node = self.xml.createElement('file')
            file_node.setAttribute('name', os.path.basename(file_path))
            file_node.setAttribute('path', file_path)
            package_node.appendChild(file_node)
        return file_node

    ##
    # @~english
    # Gets the Clover test file @c metrics node
    # @param self the object pointer
    # @param project_name the project name to find
    # @param package_name the package name to find
    # @param file_path the file path to find
    # @returns the file @c metrics node
    def get_file_metrics_node(self, project_name, package_name,
        file_path):
        project_node = self.get_project_node(project_name)
        package_node = self.get_package_node(project_name, package_name)
        file_node = self.get_file_node(project_name, package_name,
            file_path)
        try:
            file_metrics_node = file_node.getElementsByTagName(
                'metrics')[0]
        except IndexError:
            file_metrics_node = self.xml.createElement('metrics')
            self.set_file_metric_attributes(file_metrics_node)
            file_node.appendChild(file_metrics_node)
        return file_metrics_node

    ##
    # @~english
    # Sets the class metrics attributes on a Clover XML node
    # @param self the object pointer
    # @param xml_node the node to set the attributes on
    # @returns None
    def set_class_metric_attributes(self, xml_node):
        xml_node.setAttribute('complexity', '0')
        xml_node.setAttribute('elements', '0')
        xml_node.setAttribute('coveredelements', '0')
        xml_node.setAttribute('conditionals', '0')
        xml_node.setAttribute('coveredconditionals', '0')
        xml_node.setAttribute('statements', '0')
        xml_node.setAttribute('coveredstatements', '0')
        xml_node.setAttribute('methods', '0')
        xml_node.setAttribute('coveredmethods', '0')
        xml_node.setAttribute('testduration', '0')
        xml_node.setAttribute('testfailures', '0')
        xml_node.setAttribute('testpasses', '0')
        xml_node.setAttribute('testruns', '0')

    ##
    # @~english
    # Sets the file metrics attributes on a Clover XML node
    # @param self the object pointer
    # @param xml_node the node to set the attributes on
    # @returns None
    def set_file_metric_attributes(self, xml_node):
        self.set_class_metric_attributes(xml_node)
        xml_node.setAttribute('classes', '0')
        xml_node.setAttribute('loc', '0')
        xml_node.setAttribute('ncloc', '0')

    ##
    # @~english
    # Sets the package metrics attributes on a Clover XML node
    # @param self the object pointer
    # @param xml_node the node to set the attributes on
    # @returns None
    def set_package_metric_attributes(self, xml_node):
        self.set_file_metric_attributes(xml_node)
        xml_node.setAttribute('files', '0')

    ##
    # @~english
    # Sets the project metrics attributes on a Clover XML node
    # @param self the object pointer
    # @param xml_node the node to set the attributes on
    # @returns None
    def set_project_metric_attributes(self, xml_node):
        self.set_package_metric_attributes(xml_node)
        xml_node.setAttribute('packages', '0')

    ##
    # @~english
    # Sets the project metrics attributes on a Clover XML node
    # @param self the object pointer
    # @param file_path the gcov output file path
    # @param excludes files to exclude from parsing
    # @param project_name the project this file belongs to
    # @param package_name the package this file belongs to
    # @returns None
    def parse_file(self, file_path, excludes,
            project_name = "Unknown",
            package_name = "Unknown"):
        file_path = os.path.normpath(os.path.abspath(file_path))

        # Get the source path from the first line
        with open(file_path, 'r') as f:
            source_path = f.readline()[23:].strip()
            if re.search(excludes, source_path):
                print('Excluding %s' % source_path)
                return

        # Get the nodes we need for this file
        coverage_node = self.get_coverage_node()
        project_node = self.get_project_node(project_name)
        project_metrics_node = self.get_project_metrics_node(project_name)
        package_node = self.get_package_node(project_name, package_name)
        package_metrics_node = self.get_package_metrics_node(project_name,
            package_name)
        file_node = self.get_file_node(project_name, package_name, source_path)
        file_metrics_node = self.get_file_metrics_node(project_name,
            package_name, source_path)

        # TODO Can we realiably parse classes?

        # Attribute values
        file_metrics_attr_dict = {
            'loc': 0,
            'ncloc': 0,
            'classes': 0,
            'complexity': 0,
            'elements': 0,
            'coveredelements': 0,
            'methods': 0,
            'coveredmethods': 0,
            'statements': 0,
            'coveredstatements': 0,
            'conditionals': 0,
            'coveredconditionals': 0
        }

        # Parse the file
        with open(file_path, 'r') as f:
            for l in f.readlines():
                # Get the line number
                line_number = int(l[10:15].strip())
                if line_number > 0:
                    # Anything with a valid line number is a line of code
                    file_metrics_attr_dict['loc'] += 1

                # Strip the line text
                line_text = l[16:]

                # Parse the execution output
                execution_count = l[:9].strip()
                if execution_count == '-':
                    # Not a line
                    continue
                elif execution_count == '#####':
                    # Unreachable
                    execution_count = '0'
                elif execution_count == '=====':
                    # Unreachable without exception
                    execution_count = '0'
                execution_count = int(execution_count)

                # This is a non-comment line of code
                file_metrics_attr_dict['ncloc'] += 1

                # Decide what type of line this is
                #   stmt   - statement
                #   cond   - conditional
                #   method - method
                # TODO: Work out what each line is!!
                line_type = 'stmt'

                # Line attributes
                line_attr_dict = {
                    'type': line_type,
                    'num' : line_number
                }

                # Update the line attributes
                if (line_attr_dict['type'] == 'stmt' or
                    line_attr_dict['type'] == 'method'):
                    line_attr_dict['count'] = execution_count
                if line_type == 'method':
                    # TODO update this to work out the cyclomatic complexity
                    line_attr_dict['complexity']  = 1
                    # TODO use line_text to show the method signature
                    line_attr_dict['signature'] = ''
                    # TODO update this to show the method visibility
                    line_attr_dict['visibility'] = 'public'
                if line_type == 'cond':
                    # TODO work out the correct counts
                    line_dict['truecount'] = 1
                    line_dict['falsecount'] = 1

                # Add the line node
                line_node = self.xml.createElement('line')
                for attr in line_attr_dict:
                    line_node.setAttribute(attr, str(line_attr_dict[attr]))
                file_node.appendChild(line_node)

                # Bump the file metrics
                file_metrics_attr_dict['elements'] += 1
                file_metrics_attr_dict['coveredelements'] += execution_count > 0
                if line_attr_dict['type'] == 'stmt':
                    file_metrics_attr_dict['statements'] += 1
                    file_metrics_attr_dict['coveredstatements'] += execution_count > 0
                elif line_attr_dict['type'] == 'cond':
                    file_metrics_attr_dict['conditionals'] += 1
                    file_metrics_attr_dict['coveredstatements'] += execution_count > 0
                elif line_attr_dict['type'] == 'method':
                    file_metrics_attr_dict['methods'] += 1
                    file_metrics_attr_dict['coveredconditionals'] += execution_count > 0
                    file_metrics_attr_dict['complexity'] += line_attr_dict['complexity']

        # Update the attributes back up the Clover XML chain
        for attr in file_metrics_attr_dict:
            file_metrics_node.setAttribute(attr,
                str(file_metrics_attr_dict[attr]))
            package_metrics_node.setAttribute(attr,
                str(int(package_metrics_node.getAttribute(attr)) +
                file_metrics_attr_dict[attr]))
            project_metrics_node.setAttribute(attr,
                str(int(package_metrics_node.getAttribute(attr)) +
                file_metrics_attr_dict[attr]))
        package_metrics_node.setAttribute('files',
            str(int(package_metrics_node.getAttribute('files')) + 1))
        project_metrics_node.setAttribute('files',
            str(int(package_metrics_node.getAttribute('files')) + 1))

        # Update the number of packages
        project_metrics_node.setAttribute('packages',
            str(len(project_node.getElementsByTagName('package'))))

    ##
    # @~english
    # Writes the clover file to disk
    # @param self the object pointer
    # @param output_path the filesystem path to write the XML file to
    # @param indent the indentation characters to put into the XML
    # @param new_line the new line characters to put into the XML
    # @returns None
    def write(self, output_path, indent = '  ', new_line = '\n'):
        addindent = '  '
        with open(output_path, 'w') as f:
            self.xml.writexml(f,
                addindent = indent, newl = new_line, encoding= 'UTF-8')
        self.xml.unlink()

    ##
    # @~english
    # Parses a list of @c gcov output file paths
    # @param self the object pointer
    # @param file_paths a list of filesystem paths
    # @param excludes files to exclude from parsing
    # @returns None
    def parse(self, file_paths, excludes):
        if not isinstance(file_paths, list):
            file_paths = [file_paths]
        for file_path in file_paths:
            self.parse_file(file_path, excludes)

    ##
    # @~english
    # Processes list of @c gcov output filesystem paths and writes a Clover XML
    # file
    # @param self the object pointer
    # @param file_paths a list of filesystem paths
    # @param excludes files to exclude from parsing
    # @param output_path the filesystem path to write the XML file to
    # @returns None
    def process(self, file_paths, output_path, excludes):
        self.parse(file_paths, excludes)
        self.write(output_path)

## @cond internal

# Executable stuff
if __name__ == '__main__':
    import argparse
    import sys

    ##
    # @~english
    # The main function if we are executing this as a standalone file
    def main():
        parser = argparse.ArgumentParser(description = __doc__,
            epilog = 'example: %s example.cpp.gcov' % sys.argv[0])
        default = 'clover.xml'
        parser.add_argument('-o, --output', dest = 'output', default = default,
            help = 'the file path to output the Atlassian Clover XML (%s)' % default)
        parser.add_argument('--exclude', dest = 'exclude', default = default,
            help = 'source paths to exclude')
        parser.add_argument('file_paths', metavar='file_path',
            type=str, nargs='+', help='file paths to gcov output files (.gcov)')
        args = parser.parse_args()
        gtc = gcov_to_clover()
        gtc.process(args.file_paths, args.output, args.exclude)

    main()

## @endcond