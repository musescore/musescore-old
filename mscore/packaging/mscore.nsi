;--------------------------------------------------------
;     MuseScore nsis installer script
;     $Id: mscore.nsi,v 1.1 2005/08/21 21:29:11 wschweer Exp $
;--------------------------------------------------------

SetCompressor /SOLID lzma

Name "MuseScore"

OutFile "MuseScore-02.exe"
InstallDir $PROGRAMFILES\MuseScore_02

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\MuseScore_02" "Install_Dir"

;--------------------------------------------------------
Page license
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles
;--------------------------------------------------------

LicenseData "COPYING"
LicenseForceSelection checkbox

Section "mscore02 (required)"
   SectionIn RO

      SetOutPath $INSTDIR
      File mscore\mscore.exe
;      File C:\msys\1.0\mingw\bin\mingwm10.dll
      File C:\qt\4.0.1\bin\mingwm10.dll
      File C:\qt\4.0.1\bin\QtGui4.dll
      File C:\qt\4.0.1\bin\QtCore4.dll
      File C:\qt\4.0.1\bin\QtXml4.dll
      SetOutPath $INSTDIR\templates
      File share\templates\instruments.xml
      File share\templates\a4piano.msc
      File share\templates\pianoa5demo.msc
      File share\templates\violina5demo.msc
      File share\templates\instruments.xml
      SetOutPath $INSTDIR\wallpaper
      File share\wallpaper\paper1.png
      File share\wallpaper\paper2.png
      File share\wallpaper\paper3.png
      SetOutPath $INSTDIR\locale
      File share\locale\mscore_de.qm
      File share\locale\mscore_ru.qm
      SetOutPath $INSTDIR\demos
      File demos\inv1.msc
      File demos\inv6.msc
      File demos\praeludium1.msc
      File demos\promenade.msc
      File demos\sonata16.msc

      ; Special Install of emmentaler font
      SetOutPath $TEMP
      File "fonts\Emmentaler-20.ttf"
      IfFileExists "$WINDIR\Fonts\Emmentaler-20.ttf" EraseTemp 0
            CopyFiles /SILENT "$TEMP\Emmentaler-20.ttf" "$WINDIR\Fonts"
      EraseTemp:
      Delete $TEMP\Emmentaler-20.ttf

      ; Write the installation path into the registry
      WriteRegStr HKLM SOFTWARE\MuseScore_02 "Install_Dir" "$INSTDIR"

      ; Write the uninstall keys for Windows
      WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MuseScore_02" "DisplayName" "MuseScore_02"
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
  DeleteRegKey HKLM SOFTWARE\MuseScore_02

  ; Remove files and uninstaller
  Delete $INSTDIR\mscore.exe
  Delete $INSTDIR\uninstall.exe
  Delete $WINDIR\Fonts\Emmentaler-20.ttf

  Delete $INSTDIR\templates\instruments.xml
  Delete $INSTDIR\templates\a4piano.msc
  Delete $INSTDIR\templates\pianoa5demo.msc
  Delete $INSTDIR\templates\violina5demo.msc
  Delete $INSTDIR\templates\instruments.xml
  RMDir "$INSTDIR\templates"

  Delete $INSTDIR\wallpaper\paper1.png
  Delete $INSTDIR\wallpaper\paper2.png
  Delete $INSTDIR\wallpaper\paper3.png
  RMDir "$INSTDIR\wallpaper"

  Delete $INSTDIR\locale\mscore_de.qm
  Delete $INSTDIR\locale\mscore_ru.qm
  RMDir "$INSTDIR\locale"

  Delete $INSTDIR\demos\inv1.msc
  Delete $INSTDIR\demos\inv6.msc
  Delete $INSTDIR\demos\praeludium1.msc
  Delete $INSTDIR\demos\promenade.msc
  Delete $INSTDIR\demos\sonata16.msc
  RMDir "$INSTDIR\demos"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MuseScore\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\MuseScore"
  RMDir "$INSTDIR"

SectionEnd

