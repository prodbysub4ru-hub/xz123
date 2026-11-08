#define MyAppName "AnimeTune"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "AnimeTune"

#ifndef PluginSource
  #error "PluginSource was not supplied by the build workflow."
#endif

[Setup]
AppId={{C8F3E52A-5E6D-4B12-9C65-8A2D0E6D0A11}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commoncf}\VST3\AnimeTune.vst3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
OutputDir=..\dist
OutputBaseFilename=AnimeTune-Installer
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
Uninstallable=yes
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
CloseApplications=no

[Files]
Source: "{#PluginSource}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall AnimeTune"; Filename: "{uninstallexe}"

[Code]
function InitializeSetup(): Boolean;
begin
  Result := IsWin64;
  if not Result then
    MsgBox('AnimeTune requires 64-bit Windows.', mbError, MB_OK);
end;
