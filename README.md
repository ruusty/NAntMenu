# NAntMenu Shell Extension - Version 0.3 #

Copyright (C) 2003 Mathias Hasselmann <mathias@taschenorakel.de>



## INTRODUCTION ##

This package contains a Windows Shell Extension expanding the 
Windows Explorer's context menu with targets from the *NAnt* build script (`*.build`).

The context menu :-

* Lists the project's  targets with a description.
* Execute the selected project's target.
* Edit the build script
* Launch *NAnt*'s documentation.


## INSTALLATION INSTRUCTIONS ##

### Entry Criteria ###

*NAnt* is installed to `"%ProgramFiles(x86)%\<NAntReleaseName>"`. 
Location doesn't matter but `"%ProgramFiles(x86)%\NAntReleaseName\bin` must be on the PATH.

In a Windows Console.

~~~
> where nant
C:\Program Files (x86)\nant-0.93-nightly-2015-11-01\bin\NAnt.exe
~~~


### Installation ###


####  NAntMenu.dll install ####


Copy `NAntMenu.dll` to a directory of your choice and register.

**Example** 

In a Windows Console with administrator privilege.

~~~
regsvr32 NAntMenu.dll
~~~


#### Zip file install ####

Open a Windows Console with administrator privilege.

~~~
> unzip NAntMenu-0.3.0.zip -d "%PROGRAMFILES%\NAntMenu"
> regsvr32 "%PROGRAMFILES%\NAntMenu\bin\NAntMenu.dll
~~~


After restarting the Windows Explorer (for instance by terminating `explorer.exe` within the task manager) `NAntMenu.dll` will manage the context menu for *NAnt* build scripts.


#### Chocolately install ####

Coming soon.

### Configure *NAntMenu* Shell Extension ###

To enable the edit command to function, the action verb for `.build` files needs to be configured.

Windows 7 has no utilities to do this so it must be done by `regedit` or *Registration Entry* file.

**AddNantBuildScriptEditAction.reg**

Change `@` entry to your editor of choice.

~~~
Windows Registry Editor Version 5.00

[HKEY_CLASSES_ROOT\NAntBuildScript\shell]

[HKEY_CLASSES_ROOT\NAntBuildScript\shell\edit]

# Change to your editor
[HKEY_CLASSES_ROOT\NAntBuildScript\shell\edit\command]
@="\"C:\\Program Files\\SlickEdit Pro 19.0.2\\win\\vs.exe\" \"%L\""
~~~



## ISSUES ##

Windows 7 "Improvements"

The File Types tool found in earlier versions of Windows that lets you edit context menus from within Windows Explorer is missing in Windows 7.

The *Set Associations* (AllControl Panel Items -> Default Programs->Set Associations) replacement can only choose default applications for various file types, and in doing so, obliterates the applications' defaults or any custom context menus you've built.

So the options are :-

* Edit the registry
* Third Party programs

#### Context Menu ####

Registering `NAntMenu.dll` creates the required registry keys except for the edit action on `.build` files.

The registry key  `HKEY_CLASSES_ROOT\NAntBuildScript\shell\edit` defines the edit action for `.build` files. The text that appears in the context menu is defined in the action key's (Default) value; if the (Default) value is empty, Windows Explorer just uses the name of the key (e.g., edit). 

Unfortunately, Windows 7 has two competing systems that determine the default actions for your file types.

* Context Menu (above)
* UserChoice Override


#### Fixing UserChoice Override ####

Excerpt from *Windows 7 Annoyances (2010)*

>If you right-click a file, select Open With, and then select a default application, or if you use the *Set Associations* window —Windows doesn’t actually change the file type. Instead, Windows adds a new key for the filename extension in `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts`, and then in that new key, adds a UserChoice key with the full path of the program you’ve chosen, like this:
`HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\
FileExts\.wav\UserChoice`
The point of the UserChoice key is to prevent applications from changing your defaults without your permission; as long as that UserChoice key exists, Windows ignores all the actions defined in the ordinary file type key. Any custom context menu actions disappear, and even the icon is replaced with the icon of the newly-selected application. 

>The *UserChoice* system just doesn’t work that well. For one, the *UserChoice*-infected file types can’t have custom context menus or custom icons. And the old trick of reinstalling an application to restore its file types won’t work if the *UserChoice* key is present (unless the installer is smart enough to deal with it, which is unlikely). But the worst part is there’s no way to defeat the system without digging through the registry.

If the NAntMenu has dissapeared from the *Windows Explorer* menu because *UserChoice* has overridden `.build` context menu settings the solution is to open the *Registry Editor*, navigate to `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts` and delete the key extension `.build` to restore. (Or delete the FileExts key altogether to restore all file extensions). 

Thereafter, refrain from using the *Open With* menu or *Control Panel* to modify your file types, and your file associations will continue to behave in their full capacity.

**RemoveNantBuildScriptUserChoice.reg**

Remove the *UserChoice* settings for `.build`.

~~~
Windows Registry Editor Version 5.00

-HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.build

~~~


Another issue can be the NAntMentu CLSID ended up in 
~~~
[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Blocked]
"{2A57BF13-CB2B-4769-B18C-4133153F8D28}"=""
~~~

To remove NAntMentu CLSID from blocked user interface extensions.

~~~
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Blocked]
"{2A57BF13-CB2B-4769-B18C-4133153F8D28}"=-
~~~

## TOOLS ##

Tools to manage file extensions.

#### ShellExView ####


[www.nirsoft.net/utils/shexview.html](http://www.nirsoft.net/utils/shexview.html)


The ShellExView utility displays the details of shell extensions installed on your computer, and allows you to easily disable and enable each shell extension. 

[http://www.nirsoft.net](http://www.nirsoft.net)


#### ShellMenuView ####

[http://www.nirsoft.net/utils/shell_menu_view.html](http://www.nirsoft.net/utils/shell_menu_view.html)

ShellMenuView is a small utility that display the list of static menu items that appeared in the context menu when you right-click a file/folder on Windows Explorer, and allows you to easily disable unwanted menu items. 


#### FileTypesMan ####


[www.nirsoft.net/utils/file_types_manager.html](http://www.nirsoft.net/utils/file_types_manager.html)


FileTypesMan is an alternative to the 'File Types' tab in the 'Folder Options' of Windows. It displays the list of all file extensions and types registered on your computer. For each file type, the following information is displayed: Type Name, Description, MIME Type, Perceived Type, Flags, Browser Flags, and more. 
FileTypesMan also allows you to easily edit the properties and flags of each file type, as well as it allows you to add, edit, and remove actions in a file type. 




## LIMITATIONS ##


`NAntMenu.dll` 64-bit

* Tested on Windows 7 64-bit
* Tested in Windows Explorer
* Tested in Directory Opus

## CHANGES ##

[Changes.txt](Changes.txt)

## LICENCE CONDITIONS ##

<pre>

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

</pre>
