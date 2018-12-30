;NSIS Modern User Interface
;Start Menu Folder Selection Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI
!include "MUI2.nsh"
; 64 Bit check
!include x64.nsh
; File association
!include "FileAssociation.nsh"

;--------------------------------
;General

Function .onInit
	${If} ${RunningX64}
		# 64 bit code
	${Else}
		# 32 bit code
		MessageBox MB_OK "Xournal++ needs 64bit Operating System - quit installer"
		Abort
	${EndIf}
FunctionEnd

;Name and file
Name "Xournal++"
OutFile "xournalpp-setup.exe"

;Default installation folder
InstallDir $PROGRAMFILES64\Xournal++

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\Xournalpp" ""

;Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
;Variables

Var StartMenuFolder

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\LICENSE"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Xournalpp" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Xournal++"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Xournal++" SecXournalpp
	; Required
	SectionIn RO

	SetOutPath "$INSTDIR"

	; Files to put into the setup
	File /r "setup\*"

	;Store installation folder
	WriteRegStr HKCU "Software\Xournalpp" "" $INSTDIR

	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		;Create shortcuts
		CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
		CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Xournal++.lnk" "$INSTDIR\Bin\xournalpp.exe"
		CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	!insertmacro MUI_STARTMENU_WRITE_END

SectionEnd


Section "Assign .xopp files" SecFileXopp
	${registerExtension} "$INSTDIR\bin\xournalpp.exe" ".xopp" "Xournal++ file"
SectionEnd

Section "Assign .xopt files" SecFileXopt
	${registerExtension} "$INSTDIR\bin\xournalpp.exe" ".xopt" "Xournal++ Template Files"
SectionEnd

Section "Assign .xoj files" SecFileXoj
	${registerExtension} "$INSTDIR\bin\xournalpp.exe" ".xoj" "Xournal file"
SectionEnd

;--------------------------------
;Descriptions

	;Language strings
	LangString DESC_SecXournalpp ${LANG_ENGLISH} "Xournal++ executable"
	LangString DESC_SecFileXopp ${LANG_ENGLISH} "Open .xopp files with Xournal++"
	LangString DESC_SecFileXopt ${LANG_ENGLISH} "Open .xopt files with Xournal++"
	LangString DESC_SecFileXoj ${LANG_ENGLISH} "Open .xoj files with Xournal++"

	;Assign language strings to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${SecXournalpp} $(DESC_SecXournalpp)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecFileXopp} $(DESC_SecFileXopp)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecFileXopt} $(DESC_SecFileXopt)
		!insertmacro MUI_DESCRIPTION_TEXT ${SecFileXoj} $(DESC_SecFileXoj)
	!insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  DeleteRegKey /ifempty HKCU "Software\Xournalpp"

SectionEnd
