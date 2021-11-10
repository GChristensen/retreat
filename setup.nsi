Unicode True

!define APPNAME "Enso Retreat"
!define VERSION "0.6.1"

!define APPNAMEANDVERSION "Enso Retreat ${VERSION}"

Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\Enso Retreat"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "enso-retreat-${VERSION}.exe"

SetCompressor LZMA

!include "MUI.nsh"

!define MUI_ICON "retreat/res/main.ico"
!define MUI_UNICON "retreat/res/main.ico"

!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_LEFT

!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP "media\setup.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "media\setup.bmp"

!define MUI_ABORTWARNING
#!define MUI_FINISHPAGE_RUN "$INSTDIR\retreat.exe"

!insertmacro MUI_PAGE_WELCOME
#!insertmacro MUI_PAGE_LICENSE "media/copying.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
#!insertmacro MUI_RESERVEFILE_LANGDLL

Section "Enso Retreat" Section1
	SetOverwrite on
	SetOutPath "$INSTDIR\"
	File "Release\retreat.exe"
    File "help\retreat.html"
	File "media\copying.rtf"
#	SetOutPath "$APPDATA\${APPNAME}"
#	File "retreat.cfg"
	
	CreateDirectory "$SMPROGRAMS\Enso Retreat"
	CreateShortCut "$SMPROGRAMS\Enso Retreat\Enso Retreat.lnk" "$INSTDIR\retreat.exe"
	CreateShortCut "$SMPROGRAMS\Enso Retreat\Uninstall.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" DisplayIcon $INSTDIR\uninstall.exe
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$INSTDIR\uninstall.exe"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run\" "${APPNAME}" "$INSTDIR\retreat.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
#!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
#	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} ""
#!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"
	DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run\" "${APPNAME}"

	Delete "$SMPROGRAMS\Enso Retreat\Enso Retreat.lnk"
	Delete "$SMPROGRAMS\Enso Retreat\Uninstall.lnk"

    Delete "$INSTDIR\uninstall.exe"

	RMDir /r "$INSTDIR"
SectionEnd

; On initialization
Function .onInit

	#!insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

BrandingText "Enso Retreat"

; eof