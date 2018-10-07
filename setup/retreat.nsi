; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "Enso Retreat"
!define VERSION "0.5.0"

!define APPNAMEANDVERSION "Enso Retreat ${VERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\Enso Retreat"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "enso-retreat-${VERSION}.exe"

; Use compression
SetCompressor LZMA

; Modern interface settings
!include "MUI.nsh"

!define MUI_ICON "..\media\tomoe.ico"
!define MUI_UNICON "..\media\tomoe.ico"

!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_LEFT

!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP ".\tomoe.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ".\tomoe.bmp"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\retreat.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "copying.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
#!insertmacro MUI_LANGUAGE "Russian"
#!insertmacro MUI_RESERVEFILE_LANGDLL

Section "Enso Retreat" Section1

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File "..\retreat\Release\retreat.exe"
	File "copying.rtf"
	#SetOutPath "$INSTDIR\language\"
	#File "russian.dll"
	SetOutPath "$APPDATA\${APPNAME}"
	File "retreat.cfg"
	
	#IntCmp $LANGUAGE 1049 +1 +2 +2
	#	WriteIniStr "$APPDATA\${APPNAME}\retreat.cfg" "General" "Language" "russian.dll"
		
	CreateDirectory "$SMPROGRAMS\Enso Retreat"
	CreateShortCut "$SMPROGRAMS\Enso Retreat\Enso Retreat.lnk" "$INSTDIR\retreat.exe"
	CreateShortCut "$SMPROGRAMS\Enso Retreat\Uninstall.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run\" "${APPNAME}" "$INSTDIR\retreat.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
#!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
#	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} ""
#!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"
	DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run\" "${APPNAME}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Delete Shortcuts
	Delete "$SMPROGRAMS\Enso Retreat\Enso Retreat.lnk"
	Delete "$SMPROGRAMS\Enso Retreat\Uninstall.lnk"

	; Clean up Enso Retreat
	Delete "$INSTDIR\retreat.exe"
	Delete "$INSTDIR\language\russian.dll"

	; Remove remaining directories
	RMDir "$SMPROGRAMS\Enso Retreat"
	RMDir "$INSTDIR\language\"
	RMDir "$INSTDIR\"

SectionEnd

; On initialization
Function .onInit

	#!insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

BrandingText "Enso Retreat"

; eof