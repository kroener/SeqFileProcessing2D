!include "MUI.nsh"

Name "SeqFileProcessing2D"

Icon "SeqFileProcessing2D.ico"

OutFile "SeqFileProcessing2DInstaller.exe"
InstallDir $PROGRAMFILES64\SeqFileProcessing2D

!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section ""
  SetOutPath $INSTDIR
  File build/SeqFileProcessing2D.exe
  File SeqFileProcessing2D.ico
  WriteUninstaller $INSTDIR\uninstall.exe
  CreateShortCut "$DESKTOP\SeqFileProcessing2D.lnk" "$INSTDIR\SeqFileProcessing2D.exe" "" "$INSTDIR\SeqFileProcessing2D.ico"
  CreateDirectory "$SMPROGRAMS\SeqFileProcessing2D" 
  CreateShortCut "$SMPROGRAMS\SeqFileProcessing2D\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\SeqFileProcessing2D.ico"
  CreateShortCut "$SMPROGRAMS\SeqFileProcessing2D\SeqFileProcessing2D.lnk" "$INSTDIR\SeqFileProcessing2D.exe" "" "$INSTDIR\SeqFileProcessing2D.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SeqFileProcessing2D" "DisplayName" "SeqFileProcessing2D (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SeqFileProcessing2D" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKCR "SeqFileProcessing2D\DefaultIcon" "" "$INSTDIR\SeqFileProcessing2D.ico"  
  WriteRegStr HKCR "seq_auto_file\Shell\open\command\" "" '"$INSTDIR\SeqFileProcessing2D.exe" "-i"  "%1"'
  WriteRegStr HKCR "SeqFileProcessing2D\Shell\open\command\" "" '"$INSTDIR\SeqFileProcessing2D.exe" "-i"  "%1"'
  WriteRegStr HKCR ".seq" "" "SeqFileProcessing2D.exe"
SectionEnd

Section "Uninstall"
  Delete "$DESKTOP\SeqFileProcessing2D.lnk"
  Delete "$SMPROGRAMS\SeqFileProcessing2D\*.*"
  RmDir  "$SMPROGRAMS\SeqFileProcessing2D"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\SeqFileProcessing2D"
  DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\SeqFileProcessing2D"  
  DeleteRegKey HKCR "SeqFileProcessing2D\DefaultIcon"  
  DeleteRegKey HKCR "seq_auto_file\Shell\open\command\"
  DeleteRegKey HKCR "SeqFileProcessing2D\Shell\open\command\"
  DeleteRegKey HKCR ".seq"
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\SeqFileProcessing2D.exe
  Delete $INSTDIR\SeqFileProcessing2D.ico
  RMDir $INSTDIR
SectionEnd