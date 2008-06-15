; degrib.nsi -- Is the install script for degrib.
;
;  To connect to internet use dialer
;  To download via HTTP use NSISdl
;     To download via http/https/ftp use inetc plugin.
;  To patch, use VPatch 3
;  http://nsis.sourceforge.net/NSIS_Beyond_a_Traditional_Installation_II
;  To play music:
;    System::Call 'kernel32::Beep(i 800,i 100) l'
;    sleep 6000
;    /cygdrive/e/prj/etc/sound
;  Problem... We remove the uninstaller but we don't remove all registry entries?  if registry entries have version info

;--------------------------------
; Script Definitions

!define prjDate "PRJDATE"
!define prjVer "PRJVER"

Var shortDir

!include "MUI2.nsh"

!define MUI_ICON e:/svn/degrib/bin/ndfd.ico
!define MUI_UNICON e:/svn/degrib/bin/ndfd.ico

;-----------------------------------------------------------------------------
Function .onInit
  ; Make sure there is only one instance running at a time.
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "degribMutex") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort

;  System::Call 'kernel32::Beep(i 800,i 100) l'
FunctionEnd

;-----------------------------------------------------------------------------
; Allow the user to change the StartMenuGroup
Function StartMenuGroupSelect
	Push $R1
	StartMenu::Select /checknoshortcuts "Don't create a start menu folder" /autoadd /lastused $shortDir "NDFD Degrib ${prjVer}"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		StrCpy $shortDir "NDFD Degrib ${prjVer}" # use default
		Return
	success:
	Pop $shortDir

	done:
	Pop $R1
FunctionEnd

Function LaunchLink
  MessageBox MB_OK "Reached LaunchLink $\r$\n \
                   SMPROGRAMS: $SMPROGRAMS  $\r$\n \
                   Start Menu Folder: $STARTMENU_FOLDER $\r$\n \
                   InstallDirectory: $INSTDIR "
;  ExecShell "" "C:\User\test.lnk"
FunctionEnd

;-----------------------------------------------------------------------------
; Create a log file so we can undo our install.

!define UninstLog "$INSTDIR\degrib\distrib\uninstall.log"
Var UninstLog

; AddItem macro
!macro AddItem Path
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"

; File macro
!macro File FilePath FileName
 IfFileExists "$OUTDIR\${FileName}" +2
  FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"

; Copy files macro
!macro CopyFiles SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 CopyFiles "${SourcePath}" "${DestPath}"
!macroend
!define CopyFiles "!insertmacro CopyFiles"

; Rename macro
!macro Rename SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 Rename "${SourcePath}" "${DestPath}"
!macroend
!define Rename "!insertmacro Rename"

; CreateDirectory macro
!macro CreateDirectory Path
 CreateDirectory "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"

; SetOutPath macro
!macro SetOutPath Path
 SetOutPath "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"

; WriteUninstaller macro
!macro WriteUninstaller Path
 WriteUninstaller "$OUTDIR\${Path}"
 FileWrite $UninstLog "$OUTDIR\${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"

Section -openlogfile
 CreateDirectory "$INSTDIR"
 IfFileExists "$INSTDIR\${UninstLog}" +3
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd

; Finished the log file so we can undo our install.
;-----------------------------------------------------------------------------

;--------------------------------
; The name of the installer
Name "Degrib ${prjVer} (aka NDFD GRIB2 Decoder)"

; The file to write
OutFile "degrib-install.exe"

XPStyle on

BrandingText "degrib ${prjVer}"

; The default installation directory
InstallDir c:\ndfd2

; Request application privileges for Windows Vista
RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
; Pages

; Title to display on the top of the page.
    !define MUI_WELCOMEPAGE_TITLE "Welcome to the Degrib ${prjVer} Setup Wizard"
; Extra space for the title area.
;   !define MUI_WELCOMEPAGE_TITLE_3LINES
; Text to display on the page.
    !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of Degrib ${prjVer}$\n$\n\
      Degrib ${prjVer} was created on ${prjDate}, by the$\n   Meteorological Development Laboratory$\n   \
      National Weather Service$\n   National Oceanic and Atmospheric Administration$\n   United States' Department of Commerce$\n$\n\
      It is recommended that you close all other applications$\n\
      before starting Setup.  This will make it possible to update$\n\
      relevant system files without having to reboot your$\n\
      computer.$\n$\n\
      Click Next to continue."
  !insertmacro MUI_PAGE_WELCOME
;  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  Page custom StartMenuGroupSelect "" ": Start Menu Folder"
  !insertmacro MUI_PAGE_INSTFILES

    # These indented statements modify settings for MUI_PAGE_FINISH
    !define MUI_FINISHPAGE_TITLE "Completing the Degrib ${prjVer} Setup Wizard"
    !define MUI_FINISHPAGE_NOAUTOCLOSE
    !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN_NOTCHECKED
    !define MUI_FINISHPAGE_RUN_TEXT "Start tkdegrib"
    !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
;    !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
;    !define MUI_FINISHPAGE_SHOWREADME $INSTDIR\readme.txt
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; The stuff to install
Section "Degrib" mainDegrib
  SectionIn RO

  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR\degrib\distrib
  ${WriteUninstaller} "uninstall.exe"

  ; Write the uninstall keys for Windows
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\degrib" "DisplayName" "Degrib ${prjVer} (aka NDFD GRIB2 Decoder)"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\degrib" "UninstallString" '"$INSTDIR\degrib\distrib\uninstall.exe"'
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\degrib" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\degrib" "NoRepair" 1

;  WriteUninstaller "uninstall.exe"

  ; Set output path to the installation directory.
  ${SetOutPath} $INSTDIR

PRJFILES

;  ${SetOutPath} $INSTDIR\degrib\distrib
;  ${File} "" degrib.nsi

  # this part is only necessary if you used /checknoshortcuts
  StrCpy $R1 $shortDir 1
  StrCmp $R1 ">" skip
    ${CreateDirectory} "$SMPROGRAMS\$shortDir"
    CreateShortCut "$SMPROGRAMS\$shortDir\tkdegrib.lnk" "$INSTDIR\bin\tkdegrib.exe" "" "$INSTDIR\degrib.nsi" 0
    ${AddItem} "$SMPROGRAMS\$shortDir\tkdegrib.lnk"
    CreateShortCut "$SMPROGRAMS\$shortDir\UnInstall degrib.lnk" "$INSTDIR\degrib\distrib\uninstall.exe" "" "$INSTDIR\degrib\distrib\uninstall.exe" 0
    ${AddItem} "$SMPROGRAMS\$shortDir\UnInstall degrib.lnk"
;      SetShellVarContext All
  skip:
SectionEnd

;--------------------------------
;Descriptions
  ;Language strings
;  LangString DESC_mainDegrib ${LANG_ENGLISH} "Main Degrib install section."
;  LangString DESC_shortCut ${LANG_ENGLISH} "Create Shortcuts?"

  ;Assign language strings to sections
;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${mainDegrib} $(DESC_mainDegrib)
;    !insertmacro MUI_DESCRIPTION_TEXT ${shortCuts} $(DESC_shortCut)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;-----------------------------------------------------------------------------
; Uninstaller stuff.

Section -closelogfile
 FileClose $UninstLog
;   SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd

; Uninstall log file missing.
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"

Section Uninstall
 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${UninstLog}" +3
  MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
   Abort

 Push $R0
 Push $R1
 Push $R2
 SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
 FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
 StrCpy $R1 0

 GetLineCount:
  ClearErrors
   FileRead $UninstLog $R0
   IntOp $R1 $R1 + 1
   IfErrors 0 GetLineCount

 LoopRead:
  FileSeek $UninstLog 0 SET
  StrCpy $R2 0
  FindLine:
   FileRead $UninstLog $R0
   IntOp $R2 $R2 + 1
   StrCmp $R1 $R2 0 FindLine

   StrCpy $R0 $R0 -2
   IfFileExists "$R0\*.*" 0 +3
    RMDir $R0  #is dir
   Goto +3
   IfFileExists $R0 0 +2
    Delete $R0 #is file

  IntOp $R1 $R1 - 1
  StrCmp $R1 0 LoopDone
  Goto LoopRead
 LoopDone:
 FileClose $UninstLog
 Delete "$INSTDIR\degrib\distrib\${UninstLog}"

 Delete $INSTDIR\degrib\distrib\uninstall.exe

 RMDIR "$INSTDIR"

 ; Remove registry keys
 DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\degrib"

 Pop $R2
 Pop $R1
 Pop $R0

SectionEnd

