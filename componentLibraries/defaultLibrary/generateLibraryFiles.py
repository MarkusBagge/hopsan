#!/usr/bin/python
# Script to generate all library files from teh existing directory structure and component hpp file
# Author: Peter Nordin
# Date:  20150319
# $Id$

import os
import sys
import re
import collections

licenseheader = str()

def readFile(filepath):
    data = str()
    try:
        with open(filepath, "r") as myfile:
            data = myfile.read()
    except IOError as e:
        print(e.strerror+': '+filepath)
    return data

def pathjoinlist(path, thelist):
    newlist = list()
    for item in thelist:
        newlist.append(os.path.join(path, item))
    return newlist

def findduplicatesinlist(thelist):
    results = list()
    for item, count in collections.Counter(thelist).items():
        if count > 1:
            results.append(item)
    return results

class ComponentDir:
    def __init__(self):
        self.dirPath = ''
        self.dirName = ''
        self.compHeaders = list()
        self.compSources = list()
        self.files = list()
        self.typeNames = list()
        self.cciFile = ''
        self.headerFile = ''
        self.subDirs = list()
        self.isRootDir = False

    def isEmpty(self):
        if self.dirPath == '':
            return True
        else:
            return False

    def allTypenames(self):
        tnames = list()
        if len(self.typeNames) > 0:
            tnames += self.typeNames
        for subdir in self.subDirs:
            tnames = tnames + subdir.allTypenames()
        tnames = sorted(tnames)
        return tnames

    def allCCIFiles(self):
        ccis = list()
        if self.cciFile != '':
            ccis.append(os.path.join(self.dirPath, self.cciFile))
        for subdir in self.subDirs:
            ccis = ccis + subdir.allCCIFiles()
        ccis = sorted(ccis)
        return ccis

    def allHeaderFiles(self):
        headers = list()
        if self.headerFile != '':
            headers.append(os.path.join(self.dirPath, self.headerFile))
        for subdir in self.subDirs:
            headers = headers + subdir.allHeaderFiles()
        headers = sorted(headers)
        return headers

    def allCompHeaderFiles(self):
        compheaders = list()
        for ch in self.compHeaders:
            compheaders.append(os.path.join(self.dirPath, ch))
        for subdir in self.subDirs:
            compheaders = compheaders + subdir.allCompHeaderFiles()
        return sorted(compheaders)

    def emmediateHeaders(self):
        headers = list()
        for subdir in self.subDirs:
            if subdir.headerFile != '':
                headers.append(os.path.join(subdir.dirName, subdir.headerFile))
            else:
                headers = headers + pathjoinlist(subdir.dirName, subdir.emmediateHeaders())
        return sorted(headers)

    def emmediateCCIs(self):
        ccis = list()
        for subdir in self.subDirs:
            if subdir.cciFile != '':
                ccis.append(os.path.join(subdir.dirName, subdir.cciFile))
            else:
                ccis = ccis + pathjoinlist(subdir.dirName, subdir.emmediateCCIs())
        return sorted(ccis)

    def findTypenameEnteries(self, typename):
        results = list()
        if typename in self.typeNames:
            results.append(self)
        for subdir in self.subDirs:
            results = results + subdir.findTypenameEnteries(typename)
        return results

def generateCCIFileForComponentRegistration(dirPath, filename, componentTypeNames, includeFiles):
    filepath = os.path.join(dirPath, filename)
    file = open(filepath,  'w+')
    if not file.closed:
        print('Generating: '+filepath)
        if licenseheader != '':
            file.write(licenseheader)
            file.write("\n")
        file.write("//This file has been automatically generated by generateLibraryFiles.py!\n")
        for typeName in componentTypeNames:
            file.write(r'pComponentFactory->registerCreatorFunction("'+typeName+r'",'+typeName+r'::Creator);'+"\n")
        for includeFile in includeFiles:
            file.write(r'#include "'+includeFile.replace('\\', '/')+'"\n')
    file.close()
    return filepath

def generateHeaderFileForComponentHeaderInclusion(dirPath, filename, copmonentHeaders):
    filepath = os.path.join(dirPath, filename)
    file = open(filepath,  'w+')
    if not file.closed:
        print('Generating: '+filepath)
        if licenseheader != '':
            file.write(licenseheader)
            file.write("\n")
        file.write("//This file has been automatically generated by generateLibraryFiles.py!\n")
        for header in copmonentHeaders:
            file.write(r'#include "'+header.replace('\\', '/')+'"\n')
    file.close()
    return filepath

def generatePRI(dirPath, filename, includeFiles, componentHeaders, componentSrcFiles, componentCCIFiles):
    filepath = os.path.join(dirPath,filename)
    file = open(filepath,  'w+')
    if not file.closed:
        print('Generating: '+filepath)

        file.write("#This file has been automatically generated by generateLibraryFiles.py!\n")
        for includeFile in includeFiles:
            file.write(r'include($${PWD}/'+os.path.relpath(includeFile, dirPath).replace('\\', '/')+r')'+"\n")
        file.write("\n")
        file.write("\n")

        file.write(r'HEADERS += ')
        for header in componentHeaders:
            file.write(r' \ '+'\n'+r' $${PWD}/'+os.path.relpath(header, dirPath).replace('\\', '/'))
        file.write("\n")
        file.write("\n")

        file.write(r'SOURCES += ')
        for source in componentSrcFiles:
            file.write(r' \ '+"\n"+r' $${PWD}/'+os.path.relpath(source, dirPath).replace('\\', '/'))
        file.write("\n")
        file.write("\n")

        file.write(r'OTHER_FILES += ')
        for other in componentCCIFiles:
            file.write(r' \ '+"\n"+r' $${PWD}/'+os.path.relpath(other, dirPath).replace('\\', '/'))
        file.write("\n")
        file.write("\n")
    file.close()
    return filepath
    

def findFiles2(parentDir, suffixes, excludeDirs):
    for dirpath, dirnames, filenames in os.walk(parentDir.dirPath):
        #print('Dirpath:'+dirpath)
        #print(dirnames)
        for dirname in dirnames:
            enter_dir = True
            for d in excludeDirs:
                if d in dirname:
                    enter_dir = False
            if enter_dir:
                #print('Dirname:'+dirname)
                new_dir = ComponentDir()
                new_dir.dirName = dirname
                new_dir.dirPath = os.path.join(dirpath, dirname)
                new_dir = findFiles2(new_dir, suffixes, excludeDirs)
                parentDir.subDirs.append(new_dir)
        # Now go through local files
        for filename in filenames:
            # print('Filename:'+filename)
            name, ext = os.path.splitext(filename)
#                print(r'name: '+name)
#                print(r'ext: '+ext)
#                print(r'suffixes: '+suffixes)
            if ext != '' and ext in suffixes:
                filepath = os.path.join(dirpath, filename)
                print(r'Found match:'+filepath)
                parentDir.files.append(filename)

        parentDir.files = sorted(parentDir.files)

        break

    return parentDir

def checkTypeName(filepath):
    typename = ''
    file_h = open(filepath,  'r')
    if not file_h.closed:
        found_creator = False
        # Read until we find the creator function
        for line in file_h:
            line = line.strip()
            if line.startswith('static Component *Creator()'):
                found_creator = True
            if found_creator:
                if line.startswith('return new'):
                    try:
                        found = re.search('return new(.+?)\(\);', line).group(1)
                    except AttributeError:
                        found = ''
                    typename = found.strip()
                    break
    file_h.close()
    return typename

def generateFiles(component_dir):
    #print(component_dir.dirPath)
    for subdir in component_dir.subDirs:
        generateFiles(subdir)

    for filename in component_dir.files:
        typename = checkTypeName(os.path.join(component_dir.dirPath, filename))
        if typename != '':
            filebasename, fileext = os.path.splitext(filename)
            #print('Typename:     '+typename)
            #print('fileBaseName: '+filebasename)
            #print('fileExt:      '+fileext)
            component_dir.typeNames.append(typename)
            component_dir.compHeaders.append(filename)

    if not component_dir.isRootDir:
        if len(component_dir.typeNames) > 0:
            hname = component_dir.dirName+'.h'
            cciname = component_dir.dirName+'.cci'
            generateHeaderFileForComponentHeaderInclusion(component_dir.dirPath, hname, component_dir.compHeaders +
                                                          component_dir.emmediateHeaders())
            generateCCIFileForComponentRegistration(component_dir.dirPath, cciname, component_dir.typeNames,
                                                    component_dir.emmediateCCIs())
            component_dir.headerFile = hname
            component_dir.cciFile = cciname

def generateRootFiles(root_dir):
    # Generate a list of all component, header and cci files
    compheaders = root_dir.allCompHeaderFiles()
    headers = root_dir.allHeaderFiles()
    ccis = root_dir.allCCIFiles()
    emmheaders = root_dir.emmediateHeaders()
    emmcci = root_dir.emmediateCCIs()
    rootccfiles = list()
    #rootccfiles.append('Component.cc')

    generateCCIFileForComponentRegistration(root_dir.dirPath, 'Components.cci', root_dir.typeNames, emmcci)
    generateHeaderFileForComponentHeaderInclusion(root_dir.dirPath, 'Components.h', emmheaders+root_dir.compHeaders)

    headers.append('Components.h')
    ccis.append('Components.cci')
    generatePRI(root_dir.dirPath, 'Components.pri', '', sorted(headers + compheaders), sorted(rootccfiles), sorted(ccis))


def main(rootDirPath):
    suffixes = list()
    suffixes.append('.hpp')
    excludeDirs = list()
    excludeDirs.append('Dependencies')

    if not os.path.isabs(rootDirPath):
        #print('cwd: '+os.getcwd())
        #print('fullpath: '+os.path.join(os.getcwd(), rootDirPath))
        rootDirPath = os.path.abspath(os.path.join(os.getcwd(), rootDirPath))

    if rootDirPath[-1] == '/' or rootDirPath[-1] == "\\":
        rootDirPath = rootDirPath[:-1]

    root_dir = ComponentDir()
    root_dir.isRootDir = True
    root_dir.dirName = os.path.basename(rootDirPath)
    #print('rootdirpath: '+rootDirPath)
    #print('dirname: '+os.path.basename(rootDirPath))
    root_dir.dirPath = rootDirPath
    root_dir = findFiles2(root_dir, suffixes, excludeDirs)

    generateFiles(root_dir)
    generateRootFiles(root_dir)

    typenames = root_dir.allTypenames()
    dupes = findduplicatesinlist(typenames)
    if len(dupes) > 0:
        print('')
        print('    WARNING Duplicate Typenames Found!')
        print('')
        for d in dupes:
            enteries = root_dir.findTypenameEnteries(d)
            print(d+' in: ')
            for e in enteries:
                print('    '+e.dirPath)
    print('')
    print('Done!')


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Error: You must give at least one argument, the root dir')
        exit()
    elif len(sys.argv) < 3:
        rootDir = sys.argv[1]
        print('Warning: No license file given, using: ../../licenseHeaderALv2')
        licenseheader = readFile('../../licenseHeaderALv2')
    else:
        rootDir = sys.argv[1]
        licenseheader = readFile(sys.argv[2])

    if licenseheader == '':
        print('Warning: No license available')

    main(rootDir)
