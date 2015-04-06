NAntMenu Shell Extension - Version 0.1
Copyright (C) 2003 Mathias Hasselmann <mathias@taschenorakel.de>


INTRODUCTION

This package contains a tiny Windows Shell Extension extending the
Windows Explorer's context menu shown for NAnt build scripts by some
useful entries. The context menu will list the project's main targets.
It will allow to edit the build script and to access NAnt's online
documentation.


INSTALLATION INSTRUCTIONS

First of all a recent version of NAnt has to be installed in 
"%PROGRAMFILES%\nant". After that it's enough to copy NAntMenu.dll
to a directory of your choice and register that library by calling
"regsvr32 NAntMenu.dll" from that directory. After restarting the
Windows Explorer (for instance by terminating explorer.exe within
the task manager) NAntMenu.dll will manage the context menu for
NAnt build scripts.

To select another editor for NAnt build scripts open the 
"Folder Options" dialog from the Windows Explorers "Extras" menu.
Within that dialog go to the "File Types" page. This page contains
a list of all registered file types. Select the entry labeled
"NAnt-Build-Script" from that list and open the extended file
association editor by press "Extended". Within that editor it is
possible to modify the "edit" action.


LIMITATIONS

NAntMenu.dll never has been tested on the DOS based versions of
Windows like Windows 98 or Windows ME.


LICENCE CONDITIONS

 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
