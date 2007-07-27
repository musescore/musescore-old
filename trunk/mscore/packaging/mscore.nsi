;--------------------------------------------------------------------
;     MuseScore nsis installer script
;     $Id: mscore.nsi,v 1.1 2005/08/21 21:29:11 wschweer Exp $
;--------------------------------------------------------------------

SetCompressor /SOLID lzma

Name "MuseScore"

OutFile "MuseScore-07.exe"
InstallDir $PROGRAMFILES\MuseScore_07

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\MuseScore_07" "Install_Dir"

;--------------------------------------------------------
Page license
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles
;--------------------------------------------------------

LicenseData "COPYING"
LicenseForceSelection checkbox

Section "mscore07 (required)"
   SectionIn RO

      SetOutPath $INSTDIR
      File mscore.exe
      File C:\qt\4.3.0\bin\mingwm10.dll
      File C:\qt\4.3.0\bin\QtGui4.dll
      File C:\qt\4.3.0\bin\QtCore4.dll
      File C:\qt\4.3.0\bin\QtSvg4.dll
      File C:\qt\4.3.0\bin\QtXml4.dll
      SetOutPath $INSTDIR\templates
      File share\templates\a4piano.msc
      File share\templates\pianoa5demo.msc
      File share\templates\violina5demo.msc
      File share\templates\instruments.xml
      File share\templates\instruments_de.xml
      SetOutPath $INSTDIR\wallpaper
      File share\wallpaper\paper1.png
      File share\wallpaper\paper2.png
      File share\wallpaper\paper3.png
      SetOutPath $INSTDIR\demos
      File demos\adeste.msc
      File demos\inv1.msc
      File demos\inv6.msc
      File demos\inv10.msc
      File demos\praeludium1.msc
      File demos\prelude.msc
      File demos\promenade.msc
      File demos\sonata16.msc
      File demos\sarabande.xml

      ; Write the uninstall keys for Windows
      WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MuseScore_02" "DisplayName" "MuseScore_06"
      WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MuseScore_02" "UninstallString" '"$INSTDIR\uninstall.exe"'
      WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MuseScore_02" "NoModify" 1
      WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MuseScore_02" "NoRepair" 1
      WriteUninstaller "uninstall.exe"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\MuseScore"
  CreateShortCut  "$SMPROGRAMS\MuseScore\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut  "$SMPROGRAMS\MuseScore\MuseScore.lnk" "$INSTDIR\mscore.exe" "" "$INSTDIR\mscore.exe" 0
SectionEnd

;--------------------------------
; Uninstaller

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MuseScore_02"
  DeleteRegKey HKLM SOFTWARE\MuseScore_07

  ; Remove files and uninstaller
  Delete $INSTDIR\mscore.exe
  Delete $INSTDIR\uninstall.exe

  Delete $INSTDIR\templates\instruments.xml
  Delete $INSTDIR\templates\a4piano.msc
  Delete $INSTDIR\templates\pianoa5demo.msc
  Delete $INSTDIR\templates\violina5demo.msc
  Delete $INSTDIR\templates\instruments.xml
  Delete $INSTDIR\templates\instruments_de.xml
  RMDir "$INSTDIR\templates"

  Delete $INSTDIR\wallpaper\paper1.png
  Delete $INSTDIR\wallpaper\paper2.png
  Delete $INSTDIR\wallpaper\paper3.png
  RMDir "$INSTDIR\wallpaper"

  Delete $INSTDIR\demos\adeste.msc
  Delete $INSTDIR\demos\inv1.msc
  Delete $INSTDIR\demos\inv6.msc
  Delete $INSTDIR\demos\inv10.msc
  Delete $INSTDIR\demos\praeludium1.msc
  Delete $INSTDIR\demos\promenade.msc
  Delete $INSTDIR\demos\sonata16.msc
  Delete $INSTDIR\demos\sarabande.xml
  RMDir "$INSTDIR\demos"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MuseScore\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\MuseScore"
  RMDir "$INSTDIR"

SectionEnd

