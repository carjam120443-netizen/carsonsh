#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif

[Setup]
AppId={{A9F8B6E2-62E8-4E3D-91D2-CA25005A0001}
AppName=CarsonSH
AppVersion={#AppVersion}
AppPublisher=Carson
AppPublisherURL=https://github.com/carjam120443-netizen/carsonsh
AppSupportURL=https://github.com/carjam120443-netizen/carsonsh/issues
DefaultDirName={localappdata}\Programs\CarsonSH
DefaultGroupName=CarsonSH
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=..\..\dist\installer\carsonsh.ico
UninstallDisplayIcon={app}\carsonsh.ico
OutputDir=..\..\dist
OutputBaseFilename=CarsonSH-Setup-x64
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=no

[Tasks]
Name: "startmenuicon"; Description: "Create a Start Menu shortcut"; GroupDescription: "Shortcuts:"; Flags: checkedonce
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Dirs]
Name: "{%USERPROFILE}\.config\carsonsh"; Flags: uninsneveruninstall
Name: "{%USERPROFILE}\.config\carsonsh\themes"; Flags: uninsneveruninstall
Name: "{%USERPROFILE}\.config\carsonsh\plugins"; Flags: uninsneveruninstall

[Files]
Source: "..\..\dist\installer\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "config\config"; DestDir: "{%USERPROFILE}\.config\carsonsh"; DestName: "config"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\..\framework\themes\*.conf"; DestDir: "{%USERPROFILE}\.config\carsonsh\themes"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\..\framework\plugins\*.conf"; DestDir: "{%USERPROFILE}\.config\carsonsh\plugins"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "..\..\others\examples\COMMAND-CHEATSHEET.md"; DestDir: "{app}\docs"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\CarsonSH"; Filename: "{app}\carsonsh.cmd"; WorkingDir: "{%USERPROFILE}"
Name: "{autodesktop}\CarsonSH"; Filename: "{app}\carsonsh.cmd"; WorkingDir: "{%USERPROFILE}"; Tasks: desktopicon

[Run]
Filename: "{app}\carsonsh.cmd"; Description: "Launch CarsonSH"; Flags: postinstall nowait skipifsilent
