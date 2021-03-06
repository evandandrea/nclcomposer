; nclcomposer.nsi
;
; This script is used to create the NCL Composer package installer for
; Windows.
;  

!include "FileAssociation.nsh"

;--------------------------------
!ifndef VERSION
!define VERSION "0.4.0"
!endif

Name "NCL Composer ${VERSION}"
Caption "NCL Composer Installer"
OutFile "nclcomposer-installer-${VERSION}.exe"
Icon "../src/gui/images/icon.ico"

!define APPNAME "NCL Composer"
!define COMPANYNAME "TeleMidia"

; The default installation directory
InstallDir "$PROGRAMFILES\TeleMidia\NCL Composer"
; InstallDir "C:\Composer"

; License informations
LicenseText "Please review the license terms before installing NCL Composer."
LicenseData "../LICENSE.LGPL"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
; onInit function
Function .onInit
  ReadRegStr $R0 HKLM \
     "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" \
     "UninstallString"
  StrCmp $R0 "" done
	  
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
     "${APPNAME} is already installed. $\n$\nClick `OK` to remove the \
      previous version or `Cancel` to cancel this upgrade." \
      IDOK uninst
      Abort
     
  ;Run the uninstaller
  uninst:
    ClearErrors
    ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
    IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
    no_remove_uninstaller:

  done:

FunctionEnd

;--------------------------------

; Pages
Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------
; Install Types
InstType "Full" 
InstType "Minimal"

;--------------------------------
; The stuffs to install
Section "NCL Composer Core (required)" ; No components page, name is not important
  SectionIn RO
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR\bin
      
  ; Put file there
  File "C:\Composer\bin\*"
  
  SetOutPath $INSTDIR\bin\plugins\platforms
  File "C:\Composer\bin\plugins\platforms\qwindows.dll"

  WriteUninstaller "uninstall.exe"

  ; include Files
  ; SetoutPath $INSTDIR\include
  ; File /r "C:\Composer\include\*"

  ; data Files
  SetOutPath $INSTDIR\share\nclcomposer
  File "C:\Composer\share\nclcomposer\*"

  ; install NCL Language Profile
  SetOutPath $INSTDIR\lib\nclcomposer\plugins
  File "C:\Composer\lib\nclcomposer\plugins\libnclprofile.dll"

  ; translation files
  ; SetOutPath $INSTDIR\extensions
  ; File "C:\Composer\extensions\*.qm"

  ; Associate .cpr files with NCL Composer
  ${registerExtension} $INSTDIR\nclcomposer.exe ".cpr" "NCL Composer project"

  ; Registry information for add/remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
SectionEnd ; end the section

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SectionIn 1
  CreateDirectory "$SMPROGRAMS\NCL Composer"
  CreateShortCut  "$SMPROGRAMS\NCL Composer\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut  "$SMPROGRAMS\NCL Composer\NCL Composer.lnk" "$INSTDIR\nclcomposer.exe" "" "$INSTDIR\nclcomposer.exe" 0      
  CreateShortCut  "$SMPROGRAMS\NCL Composer\ (MakeNSISW).lnk" "$INSTDIR\nclcomposer.nsi" "" "$INSTDIR\nclcomposer.nsi" 0      
SectionEnd

; Ginga default executable
SectionGroup "Install NCL Player"
  Section "Ginga-NCL ITU-T Reference Implementation"
    SectionIn 1

    SetOutPath "$TEMP"

    DetailPrint "Downloading Ginga-NCL Reference Implementation..."

    DetailPrint "Contacting Ginga-ncl.org.br..."
    NSISdl::download /TIMEOUT=15000 "http://composer.telemidia.puc-rio.br/downloads/latest_ginga.php?system=exe" "latest_ginga-win32.exe"

    Pop $R0 ;Get the return value

    StrCmp $R0 "success" OnSuccess

    MessageBox MB_OK "Could not download Ginga-NCL ITU-T Reference Implementation; none of the mirrors appear to be functional."
      Goto done

    OnSuccess:
      DetailPrint "Running Ginga-NCL ITU-T Reference Implementation Setup..."
      ExecWait '"$TEMP\latest_ginga-win32.exe" /qb'
      DetailPrint "Finished Ginga-NCL ITU-T Reference Implementation Setup"

    Delete "$TEMP\latest_ginga-win32.exe"

    done:

  SectionEnd
SectionGroupEnd

; Plugins optional section (can be disabled by the user)
SectionGroup /e "Install Default Plugins"
  Section "Textual View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\libncl_textual_view.dll"
	
	SetOutPath $INSTDIR\bin
	File "C:\Composer\bin\libqscintilla2_telem.dll"
  SectionEnd

  Section "Layout View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\libncl_layout_view.dll"
  SectionEnd

  Section "Properties View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\libproperties_view.dll"
  SectionEnd

  Section "Structural View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\libncl_structural_view.dll"
  SectionEnd

  Section "Outline View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\liboutline_view.dll"
  SectionEnd

  Section "Validator Plugin"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\libvalidator_plugin.dll"
  SectionEnd
  
  Section "Rules View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\libncl_rules_view.dll"
  SectionEnd

  Section "Run View"
    SectionIn 1
    SetOutPath $INSTDIR\lib\nclcomposer\plugins
    File "C:\Composer\lib\nclcomposer\plugins\librun_view.dll"
  SectionEnd

SectionGroupEnd

;--------------------------------
; Uninstaller
UninstallText "This will uninstall NCL Composer. Hit next to continue"
Section "Uninstall"
  Delete "$INSTDIR\*"
  Delete "$INSTDIR\bin\*"
  Delete "$INSTDIR\lib\nclcomposer\plugins\*"
  Delete "$INSTDIR\share\nclcomposer\*"
  RMDir "$INSTDIR\bin"
  RMDir "$INSTDIR\lib"
  RMDir "$INSTDIR\share"

  ;Shortcuts
  Delete "$SMPROGRAMS\NCL Composer\*"
  RMDir "$SMPROGRAMS\NCL Composer"

  ;Remove file association
  ${unregisterExtension} ".cpr" "NCL Composer project"

  ; Always delete uninstall as the last action
  Delete "$INSTDIR\uninstall.exe"

  ; Try to remove the install dir
  RMDir "$INSTDIR"

  ;Remove uninstaller information from the registry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
SectionEnd
