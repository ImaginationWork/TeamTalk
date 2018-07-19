; -- ISPPExample1.iss --
;
; This script shows various basic things you can achieve using Inno Setup Preprocessor (ISPP).
; To enable commented #define's, either remove the ';' or use ISCC with the /D switch.

#pragma option -v+
#pragma verboselevel 9

;#define Debug

;#define AppEnterprise

#ifdef AppEnterprise
  #define AppName "TeamTalk Enterprise Edition"
#else
  #define AppName "TeamTalk"
#endif

#define AppVersion GetFileVersion(AddBackslash(SourcePath) + "Release\TeamTalk.exe")

[Setup]
AppName={#AppName}
AppVersion={#AppVersion}
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
UninstallDisplayIcon={app}\TeamTalk.exe
;LicenseFile={#file AddBackslash(SourcePath) + "ISPPExample1License.txt"}
VersionInfoVersion={#AppVersion}
OutputDir={app}\..\..\..\

[Files]         
Source: "Release\*.exe"; DestDir: "{app}\bin\"
Source: "Release\*.dll"; DestDir: "{app}\bin\"
Source: "Release\*.ini"; DestDir: "{app}\bin\"
Source: "data\*"; DestDir: "{app}\data\"; Flags: recursesubdirs
Source: "gui\*"; DestDir: "{app}\gui\"; Flags: recursesubdirs


[Icons]
Name: "{group}\..\{#AppName}"; Filename: "{app}\bin\TeamTalk.exe"

[Run]
Filename: "regsvr32.exe"; WorkingDir: "{app}\bin\"; Parameters: "/s GifSmiley.dll"
