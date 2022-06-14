

Overview
---------

DbgCM provides almost the same functionality as ULINK2 including the same configuration dialogs and Debug/Trace/Flash functionality also with ULINK2 custom Trace dialogs (Records, Exceptions, Counters).

DbgCM provides ULINK2 complete functionality (tightly integrated with uV) and requires only the low level interface for the custom debugger to be implemented (described below). All the middle/upper layer functionality is already there including complex trace handling so that engineers need to focus only the low layer. This is ideal for usual debug units which provide low level functions for Debug/Access Port R/W and Memory R/W. All the run/stop/step , breakpoint/watchpoint, trace control is already implemented in the middle layer through Memory R/W. 


Following are details on how to adjust DbgCM to custom Debug Unit:
-	Expose Debug Unit communication layer to the driver
-	Implement low level JTAG functions in JTAG.cpp module (Detect Chain, Debug/Access Port R/W, Memory R/W, Register R/W, Sys Calls)
-	Implement low level SWD functions in SWD.cpp module (Debug/Access Port R/W, Memory R/W, Register R/W, Sys Calls)
-	Implement low level SWV functions in SWV.cpp (Setup, Read, Flush)
-	Implement debug unit detection and selection with target detection (JTAG/SWD) in SetupDbg.cpp module which controls the Setup dialogs (see TODO in functions: Update, OnSelchangeConfigUnit)
-	Implement debug unit init/uninit and target detection in AGDI.cpp (see TODO in functions: InitTarget, StopTarget)
-	Implement target reset in AGDI.cpp (see TODO in function ResetTarget)
 
As already mentioned the driver provides a template with full ULINK2 functionality but it can be also easy stripped down when less functionality is required (for example if only SWD interface is supported and no JTAG …).



Important Note:
---------------

As a help, all the "TODO" Sections are marked by ---TODO, AND with a Message (what to do). The display Format (text message in output window ov UV4 or Message Box) can be choosen in the Headerfile 'Collect.h':

// Choose between this two #defines when developing the driver
#define DEVELOP_MSG             AfxMessageBox
//#define DEVELOP_MSG             txtout

Messages during Setup Dialogs do not appear in the UV4 Window, they should be tested via MessageBoxes.

To find all places where code must be added or functions must be written, scan all files for 'DEVELOP_MSG'.




















========================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : DbgCM Project Overview
========================================================================


AppWizard has created this DbgCM DLL for you.  This DLL not only
demonstrates the basics of using the Microsoft Foundation classes but
is also a starting point for writing your DLL.

This file contains a summary of what you will find in each of the files that
make up your DbgCM DLL.

DbgCM.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

DbgCM.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

DbgCM.h
    This is the main header file for the DLL.  It declares the
    CDbgCMApp class.

DbgCM.cpp
    This is the main DLL source file.  It contains the class CDbgCMApp.

DbgCM.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++.

res\DbgCM.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++.  You should place all resources not editable by
    the resource editor in this file.

DbgCM.def
    This file contains information about the DLL that must be
    provided to run with Microsoft Windows.  It defines parameters
    such as the name and description of the DLL.  It also exports
    functions from the DLL.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named DbgCM.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
