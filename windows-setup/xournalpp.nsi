; Xournal++ NSIS installation script for Windows
; Author: The Xournal++ Team

;--------------------------------
; NSIS setup

Unicode true

;--------------------------------
; Includes

!include "MUI2.nsh"
!include x64.nsh
!include "FileAssociation.nsh"
!include nsDialogs.nsh
!include "xournalpp_version.nsh"

;--------------------------------
; Initialization

Function .onInit
	${If} ${RunningX64}
		# 64 bit code
		SetRegView 64
	${Else}
		# 32 bit code
		MessageBox MB_OK "Xournal++ requires 64-bit Windows. Sorry!"
		Abort
	${EndIf}
FunctionEnd

; Name and file
Name "Xournal++ ${XOURNALPP_VERSION}"
OutFile "xournalpp-setup.exe"

; Default installation folder
InstallDir $PROGRAMFILES64\Xournal++

; Get installation folder from registry if available
InstallDirRegKey HKLM "Software\Xournal++" ""

; Request admin privileges for installation
RequestExecutionLevel admin

;--------------------------------
; Variables

Var StartMenuFolder

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING

;--------------------------------
; Pages
!insertmacro MUI_PAGE_WELCOME
Page custom InstallScopePage InstallScopePageLeave
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

;Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Xournal++"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuEntry"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Xournal++"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Prompt for user or system-wide installation
Var ScopeDialog
Var ScopeAdminInstallRadio
Function InstallScopePage
	nsDialogs::Create 1018
	Pop $ScopeDialog
	${If} $ScopeDialog == error
		Abort
	${EndIf}

	${NSD_CreateRadioButton} 0 0 100% 12u "Install for all users"
	Pop $ScopeAdminInstallRadio
	${NSD_SetState} $ScopeAdminInstallRadio ${BST_CHECKED}

	; ${NSD_CreateRadioButton} 0 12u 100% 12u "Install for current user (beta)"
	; Pop $0

	nsDialogs::Show
FunctionEnd

Var IsUserInstall
Function InstallScopePageLeave
	${NSD_GetState} $ScopeAdminInstallRadio $0

	${If} $0 == 1
		SetShellVarContext all
		StrCpy $IsUserInstall ""
	${Else}
		SetShellVarContext current
		StrCpy $IsUserInstall 1
		; Set default install location
		ReadRegStr $INSTDIR HKCU "Software\Xournal++" ""
		${IF} $INSTDIR == ""
			StrCpy $INSTDIR "$LOCALAPPDATA\Programs\Xournal++"
		${ENDIF}
	${EndIf}
FunctionEnd

;-------------------------------
; Uninstall previous version

Var IsLegacyInstall
Section "" SecUninstallPrevious
	ReadRegStr $R0 SHCTX "Software\Xournal++" ""
	${If} $R0 == ""
		; check for legacy installation
		ReadRegStr $R0 HKCU "Software\Xournalpp" ""
		${If} $R0 != ""
			StrCpy $IsLegacyInstall 1
		${EndIf}
		SetRegView 64
	${ENDIF}
	${If} $R0 != ""
        DetailPrint "Removing previous version located at $R0"
		ExecWait '"$R0\Uninstall.exe /S"'

		${If} $IsLegacyInstall == 1
			DetailPrint "Detected legacy installation (version 1.0.20 and below), cleaning up old files."
			RMDir /r "$R0\bin"
			RMDir /r "$R0\lib"
			RMDir /r "$R0\share"
			RMDir /r "$R0\ui"
			RMDir "$R0"

			; delete old start menu entry
			DetailPrint "Removing old start menu entries"
			${If} "$IsUserInstall" == ""
				SetShellVarContext current
			${EndIf}
			!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
			Delete "$SMPROGRAMS\$StartMenuFolder\Xournal++.lnk"
			Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
			RMDir "$SMPROGRAMS\$StartMenuFolder"
			${If} "$IsUserInstall" == ""
				SetShellVarContext all
			${EndIf}
			
			DetailPrint "Removing old registry keys"
			DeleteRegKey HKLM "Software\Classes\Xournal++ file"
			DeleteRegKey HKLM "Software\Classes\Xournal++ Template Files"
			DeleteRegKey HKLM "Software\Classes\Xournal file"
			DeleteRegKey HKCU "Software\Xournalpp"
		${EndIf}
    ${EndIf}
SectionEnd

;-------------------------------
; File association macros
; see https://docs.microsoft.com/en-us/windows/win32/shell/fa-file-types#registering-a-file-type

!macro SetDefaultExt EXT PROGID
	WriteRegStr SHCTX "Software\Classes\${EXT}" "" "${PROGID}"
!macroend

!macro RegisterExt EXT PROGID
	WriteRegStr SHCTX "Software\Classes\${EXT}\OpenWithProgIds" "${PROGID}" ""
	WriteRegStr SHCTX "Software\Classes\Applications\xournalpp.exe\SupportedTypes" "${EXT}" ""
!macroend

!macro AddProgId PROGID CMD DESC
	; Define ProgId. See https://docs.microsoft.com/en-us/windows/win32/shell/fa-progids
	WriteRegStr SHCTX "Software\Classes\${PROGID}" "" "${DESC}"
	WriteRegStr SHCTX "Software\Classes\${PROGID}\DefaultIcon" "" '"${CMD}",0'
	WriteRegStr SHCTX "Software\Classes\${PROGID}\shell" "" "open"
	WriteRegStr SHCTX "Software\Classes\${PROGID}\shell\open\command" "" '"${CMD}" "%1"'
	WriteRegStr SHCTX "Software\Classes\${PROGID}\shell\edit" "" "Edit with Xournal++"
	WriteRegStr SHCTX "Software\Classes\${PROGID}\shell\edit\command" "" '"${CMD}" "%1"'
!macroend

!macro DeleteProgId PROGID
	; See https://docs.microsoft.com/en-us/windows/win32/shell/fa-file-types#deleting-registry-information-during-uninstallation
	DeleteRegKey SHCTX "Software\Classes\${PROGID}"
!macroend

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNE_CREATE 0x2
!define SHCNE_DELETE 0x4

!define SHCNF_IDLIST 0x0
!define SHCNF_PATH 0x1
!define SHCNF_FLUSH 0x1000
!macro RefreshShellIcons
	; Refresh shell icons. See https://nsis.sourceforge.io/Refresh_shell_icons
	DetailPrint "Refreshing shell file associations"
	System::Call "shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_FLUSH} | ${SHCNF_IDLIST}, i 0, i 0)"
!macroend

!macro RefreshShellIconCreate FILEPATH
	DetailPrint "Refreshing shell icon create ${FILEPATH}"
	System::Call 'shell32::SHChangeNotify(i ${SHCNE_CREATE}, i ${SHCNF_FLUSH} | ${SHCNF_PATH}, w "${FILEPATH}", i 0)'
!macroend

!macro RefreshShellIconDelete FILEPATH
	DetailPrint "Refreshing shell icon delete ${FILEPATH}"
	System::Call 'shell32::SHChangeNotify(i ${SHCNE_DELETE}, i ${SHCNF_FLUSH} | ${SHCNF_PATH}, w "${FILEPATH}", i 0)'
!macroend

;-------------------------------
; Installer Sections

Section "Associate .xopp files with Xournal++" SecFileXopp
	!insertmacro SetDefaultExt ".xopp" "Xournal++.File"
SectionEnd

Section "Associate .xopt files with Xournal++" SecFileXopt
	!insertmacro SetDefaultExt ".xopt" "Xournal++.Template"
SectionEnd

Section "Associate .xoj files with Xournal++" SecFileXoj
	!insertmacro SetDefaultExt ".xoj" "Xournal++.Xournal"
SectionEnd

Section "Xournal++" SecXournalpp
	; Required
	SectionIn RO

	SetOutPath "$INSTDIR"

	; Files to put into the setup
	File /r "dist\*"

	; Set install information
	WriteRegStr SHCTX "Software\Xournal++" "" '"$INSTDIR"'

	; Set program information
	WriteRegStr SHCTX "Software\Classes\Applications\xournalpp.exe" "" '"$INSTDIR\bin\xournalpp.exe"'
	WriteRegStr SHCTX "Software\Classes\Applications\xournalpp.exe" "FriendlyAppName" "Xournal++"
	WriteRegExpandStr SHCTX "Software\Classes\Applications\xournalpp.exe" "DefaultIcon" '"$INSTDIR\bin\xournalpp.exe",0'
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\App Paths\xournalpp.exe" "" '"$INSTDIR\bin\xournalpp.exe"'

	; Add file type information
	!insertmacro RegisterExt ".xopp" "Xournal++.File"
	!insertmacro RegisterExt ".xopt" "Xournal++.Template"
	!insertmacro RegisterExt ".xoj" "Xournal++.Xournal"
	!insertmacro RegisterExt ".pdf" "Xournal++.AnnotatePdf"
	push $R0
	StrCpy $R0 "$INSTDIR\bin\xournalpp.exe"
	!insertmacro AddProgId "Xournal++.File" "$R0" "Xournal++ file"
	!insertmacro AddProgId "Xournal++.Template" "$R0" "Xournal++ template file"
	!insertmacro AddProgId "Xournal++.Xournal" "$R0" "Xournal file"
	!insertmacro AddProgId "Xournal++.AnnotatePdf" "$R0" "PDF file"
	pop $R0

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	; Add uninstall entry. See https://docs.microsoft.com/en-us/windows/win32/msi/uninstall-registry-key
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "DisplayIcon" '"$INSTDIR\bin\xournalpp.exe"'
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "DisplayName" "Xournal++"
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "DisplayVersion" "${XOURNALPP_VERSION}"
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "Publisher" "The Xournal++ Team"
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "URLInfoAbout" "https://xournalpp.github.io"
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "InstallLocation" '"$INSTDIR"'
	WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "NoModify" 1
	WriteRegDWORD SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++" "NoRepair" 1

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		;Create shortcuts
		CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
		CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Xournal++.lnk" '"$INSTDIR\bin\xournalpp.exe"'
		CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" '"$INSTDIR\Uninstall.exe"'
		
		!insertmacro RefreshShellIconCreate "$SMPROGRAMS\$StartMenuFolder\Xournal++.lnk"
	!insertmacro MUI_STARTMENU_WRITE_END

	!insertmacro RefreshShellIcons
SectionEnd

;--------------------------------
; Descriptions

; Language strings
LangString DESC_SecXournalpp ${LANG_ENGLISH} "Xournal++ executable"
LangString DESC_SecFileXopp ${LANG_ENGLISH} "Open .xopp files with Xournal++"
LangString DESC_SecFileXopt ${LANG_ENGLISH} "Open .xopt files with Xournal++"
LangString DESC_SecFileXoj ${LANG_ENGLISH} "Open .xoj files with Xournal++"

; Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecXournalpp} $(DESC_SecXournalpp)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecFileXopp} $(DESC_SecFileXopp)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecFileXopt} $(DESC_SecFileXopt)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecFileXoj} $(DESC_SecFileXoj)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller

Section "Uninstall"

	SetRegView 64

	; FIXME: ask if the user wants to uninstall the user or system wide install
	ReadRegStr $0 HKCU "Software\Xournal++" ""
	${IF} $0 == ""
		SetShellVarContext all
	${ELSE}
		SetShellVarContext current
	${ENDIF}

	; Remove registry keys
	DeleteRegKey SHCTX "Software\Xournal++"
	DeleteRegKey SHCTX "Software\Classes\Applications\xournalpp.exe"
	DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\App Paths\xournalpp.exe"
	DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Xournal++"

	!insertmacro DeleteProgId "Xournal++.File"
	!insertmacro DeleteProgId "Xournal++.Template"
	!insertmacro DeleteProgId "Xournal++.Xournal"
	!insertmacro DeleteProgId "Xournal++.AnnotatePdf"

	; Clean up start menu
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
	Delete "$SMPROGRAMS\$StartMenuFolder\Xournal++.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
	RMDir "$SMPROGRAMS\$StartMenuFolder"

	; Remove files
	RMDir /r "$INSTDIR\bin"
	RMDir /r "$INSTDIR\lib"
	RMDir /r "$INSTDIR\share"
	RMDir /r "$INSTDIR\ui"
	Delete "$INSTDIR\Uninstall.exe"
	RMDir "$INSTDIR"

	!insertmacro RefreshShellIconDelete "$SMPROGRAMS\$StartMenuFolder\Xournal++.lnk"
	!insertmacro RefreshShellIcons
SectionEnd
