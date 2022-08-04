!include Library.nsh

; config installer
OutFile "obse_installer.exe"
InstallDirRegKey HKLM "SOFTWARE\Bethesda Softworks\Oblivion" "Installed Path"
DirText "OBSE Installer" "Oblivion install directory" "" ""
SetCompressor /SOLID /FINAL lzma
Name "OBSE"

; main section
Section
	SetOutPath $INSTDIR
	!insertmacro InstallLib DLL NOTSHARED NOREBOOT_NOTPROTECTED obse_1_1.dll $INSTDIR\obse_1_1.dll $INSTDIR
	!insertmacro InstallLib DLL NOTSHARED NOREBOOT_NOTPROTECTED obse_1_2.dll $INSTDIR\obse_1_2.dll $INSTDIR
	!insertmacro InstallLib DLL NOTSHARED NOREBOOT_NOTPROTECTED obse_1_2_416.dll $INSTDIR\obse_1_2_416.dll $INSTDIR
	!insertmacro InstallLib DLL NOTSHARED NOREBOOT_NOTPROTECTED obse_editor_1_0.dll $INSTDIR\obse_editor_1_0.dll $INSTDIR
	!insertmacro InstallLib DLL NOTSHARED NOREBOOT_NOTPROTECTED obse_editor_1_2.dll $INSTDIR\obse_editor_1_2.dll $INSTDIR
	!insertmacro InstallLib DLL NOTSHARED NOREBOOT_NOTPROTECTED obse_loader.exe $INSTDIR\obse_loader.exe $INSTDIR
SectionEnd

; basic oblivion install directory validation
Function DirectoryPage_Create
	IfFileExists "$INSTDIR\oblivion.exe" done
	MessageBox MB_OK "The installer was unable to determine your Oblivion install directory. Please set the install directory to the folder containing Oblivion.exe."
done:
FunctionEnd

Function DirectoryPage_Leave
	IfFileExists "$INSTDIR\oblivion.exe" done
	MessageBox MB_YESNO "The oblivion EXE file was not found in the install directory. You have probably not selected the correct folder. Continue?" /SD IDYES IDYES done
	Abort
done:
FunctionEnd

; pages
Page directory DirectoryPage_Create "" DirectoryPage_Leave
Page instfiles
