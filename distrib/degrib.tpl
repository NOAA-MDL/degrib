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
!define prjName "degrib"

Var shortDir

!define UninstLog "uninstall.log"
Var UninstLog_FP

!include "MUI2.nsh"

!define MUI_ICON c:/Arthur/svn/degrib/bin/ndfd.ico
!define MUI_UNICON c:/Arthur/svn/degrib/distrib/uninst.ico

;-----------------------------------------------------------------------------
Function .onInit
  ; Make sure there is only one instance running at a time.
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${prjName}Mutex") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
    Abort

;  System::Call 'kernel32::Beep(i 800,i 100) l'
FunctionEnd

;-----------------------------------------------------------------------------
Function un.onInit
  ; Make sure there is only one instance running at a time.
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${prjName}MutexUn") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
    MessageBox MB_OK|MB_ICONEXCLAMATION "The uninstaller is already running."
    Abort

  StrCpy $INSTDIR "$INSTDIR\.."
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

;-----------------------------------------------------------------------------
Function LaunchLink
;  Avoid the execShell with the lnk since user may not create the .lnk file
;  ExecShell "" "$SMPROGRAMS\$shortDir\tkdegrib.lnk"
  Exec "$INSTDIR\${prjName}\bin\tkdegrib.exe"
FunctionEnd

;-----------------------------------------------------------------------------
; LogIt
!macro LogIt Cmd Path
 FileWrite $UninstLog_FP "${Cmd} ${Path}$\r$\n"
!macroend
!define LogIt "!insertmacro LogIt"

;--------------------------------
; The name of the installer
Name "Degrib ${prjVer} (aka NDFD GRIB2 Decoder)"

; The file to write
OutFile "${prjName}-install.exe"

XPStyle on

BrandingText "${prjName} ${prjVer}"

; The default installation directory
InstallDir c:\ndfd

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

; Title to display on the top of the page.
    !define MUI_WELCOMEPAGE_TITLE "Welcome to the Degrib ${prjVer} Uninstall Wizard"
; Text to display on the page.
    !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the uninstallation of Degrib ${prjVer}$\n$\n\
      Before starting the uninstallation, make sure degrib, tcldegrib and tkdegrib are not running$\n$\n\
      Click Next to continue."
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
    !define MUI_FINISHPAGE_TITLE "Completing the Degrib ${prjVer} Uninstall Wizard"
    !define MUI_FINISHPAGE_TEXT "Degrib ${prjVer} has been uninstalled from your computer.$\n$\n\
      Click Finish to close this wizard."
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Section on opening log files
Section -openlogfile
 CreateDirectory "$INSTDIR"
 CreateDirectory "$INSTDIR\${prjName}"
 IfFileExists "$INSTDIR\${prjName}\${UninstLog}" +3
  FileOpen $UninstLog_FP "$INSTDIR\${prjName}\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${prjName}\${UninstLog}" NORMAL
  FileOpen $UninstLog_FP "$INSTDIR\${prjName}\${UninstLog}" a
  FileSeek $UninstLog_FP 0 END
SectionEnd

;--------------------------------
; The stuff to install
Section "Degrib" mainDegrib
  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\${prjName}
  WriteUninstaller "$INSTDIR\${prjName}\uninstall.exe"

  ; Write the uninstall keys for Windows
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prjName}" "DisplayName" "Degrib ${prjVer} (aka NDFD GRIB2 Decoder)"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prjName}" "UninstallString" '"$INSTDIR\${prjName}\uninstall.exe"'
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prjName}" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prjName}" "NoRepair" 1

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

PRJFILES
PRJUNINST_FILE
PRJUNINST_DIR

  # this part is only necessary if you used /checknoshortcuts
  StrCpy $R1 $shortDir 1
  StrCmp $R1 ">" skip
    CreateDirectory "$SMPROGRAMS\$shortDir"
    SetOutPath $INSTDIR\${prjName}\bin
    CreateShortCut "$SMPROGRAMS\$shortDir\tkdegrib.lnk" "$INSTDIR\${prjName}\bin\tkdegrib.exe" "" "$INSTDIR\${prjName}\bin\ndfd.ico" 0
    ${LogIt} Delete "$SMPROGRAMS\$shortDir\tkdegrib.lnk"
    CreateShortCut "$SMPROGRAMS\$shortDir\UnInstall ${prjName}.lnk" "$INSTDIR\${prjName}\uninstall.exe" "" "$INSTDIR\${prjName}\uninstall.exe" 0
    ${LogIt} Delete "$SMPROGRAMS\$shortDir\UnInstall ${prjName}.lnk"
    ${LogIt} RMDir "$SMPROGRAMS\$shortDir"
;      SetShellVarContext All

  skip:
SectionEnd

;--------------------------------
; Section on closing log files
Section -closelogfile
 FileClose $UninstLog_FP
 SetFileAttributes "$INSTDIR\${prjName}\${UninstLog}" READONLY|SYSTEM|HIDDEN
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
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"

;-----------------------------------------------------------------------------
; Uninstaller stuff.
Section Uninstall

 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${prjName}\${UninstLog}" +3
  MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
   Abort
 Push $R0
 Push $R1
 SetFileAttributes "$INSTDIR\${prjName}\${UninstLog}" NORMAL
 FileOpen $UninstLog_FP "$INSTDIR\${prjName}\${UninstLog}" r

 LoopOver:
  ClearErrors
  FileRead $UninstLog_FP $R0
; Remove trailing \r\n
   StrCpy $R0 $R0 -2
   StrCpy $R1 $R0 6
   StrCmp $R1 "Delete" 0 Continue
     StrCpy $R1 $R0 "" 7
     Delete "$R1"
     Goto Done
   Continue:
   StrCpy $R1 $R0 5
   StrCmp $R1 "RMDir" 0 Done
     StrCpy $R1 $R0 "" 6
     RMDir "$R1"
; RMDir causes error if directory couldn't be deleted.
     ClearErrors
     Goto Done
   Done:
   IfErrors 0 LoopOver

 FileClose $UninstLog_FP

 Delete "$INSTDIR\${prjName}\${UninstLog}"

 Delete $INSTDIR\${prjName}\uninstall.exe
 RMDir "$INSTDIR\${prjName}"
 RMDir "$INSTDIR"

 ; Remove registry keys
 DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prjName}"
 Pop $R0
 Pop $R1

SectionEnd
