Unicode True

!define APPNAME "Enso Retreat"
!define VERSION "2.0"

!define APPNAMEANDVERSION "Enso Retreat ${VERSION}"

Name "${APPNAMEANDVERSION}"
; retreat.exe and sentinel.exe are x64: install into the 64-bit Program Files and use the 64-bit
; registry view (see .onInit). InstallDirRegKey is not used because it is read before .onInit
; switches the view, so it would look in WOW6432Node.
InstallDir "$PROGRAMFILES64\Enso Retreat"
OutFile "enso-retreat-${VERSION}.exe"

SetCompressor LZMA

!include "MUI.nsh"
!include "x64.nsh"

!define MUI_ICON "..\src\win32\res\main.ico"
!define MUI_UNICON "..\src\win32\res\main.ico"

!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_LEFT

!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"

!define MUI_WELCOMEFINISHPAGE_BITMAP "..\media\setup.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "..\media\setup.bmp"

!define MUI_ABORTWARNING
#!define MUI_FINISHPAGE_RUN "$INSTDIR\retreat.exe"

!insertmacro MUI_PAGE_WELCOME
#!insertmacro MUI_PAGE_LICENSE "../media/copying.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Stop a running copy: the sentinel first (it refuses during a break), so that it does
; not restart Retreat, then Retreat itself (closes normally unless a break is under way).
!macro StopRetreat
	IfFileExists "$INSTDIR\sentinel.exe" 0 +2
		nsExec::Exec '"$INSTDIR\sentinel.exe" --quit'
	nsExec::Exec 'taskkill /im retreat.exe'
	Sleep 500
!macroend
#!insertmacro MUI_RESERVEFILE_LANGDLL

Section "Enso Retreat" Section1
	SetOverwrite on
	SetOutPath "$INSTDIR\"
	!insertmacro StopRetreat
	File "..\build\win-msvc-release\src\win32\retreat.exe"
	File "..\build\win-msvc-release\src\win32\sentinel.exe"
	File "..\help\retreat.html"
	File "..\media\copying.rtf"
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

	!insertmacro StopRetreat

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

	${IfNot} ${RunningX64}
		MessageBox MB_OK|MB_ICONSTOP "${APPNAME} requires 64-bit Windows."
		Abort
	${EndIf}

	SetRegView 64

	; reinstall into the folder of a previous 64-bit installation
	ReadRegStr $0 HKLM "Software\${APPNAME}" ""
	StrCmp $0 "" +2
		StrCpy $INSTDIR $0

FunctionEnd

Function un.onInit

	SetRegView 64

FunctionEnd

BrandingText "Enso Retreat"

; eof