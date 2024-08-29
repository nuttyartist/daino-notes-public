#define Version GetEnv("APP_VERSION")

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{44576435-E098-4581-A6D7-FD7FADC5B063}
AppName=Daino Notes
AppVersion={#Version}
AppPublisher=Awesomeness
AppPublisherURL=https://www.get-notes.com/
AppSupportURL=https://www.get-notes.com/
AppUpdatesURL=https://www.get-notes.com/
UninstallDisplayIcon={app}\Daino Notes.exe
DefaultDirName={autopf64}\Daino Notes
DefaultGroupName=Daino Notes
UninstallDisplayName=Daino Notes
OutputBaseFilename=DainoNotesSetup_{#Version}
Compression=lzma
SolidCompression=yes
; Inno Setup can't seem to calculate the required disk space correctly, and asks for only 3.1 MB.
; Let's use an approximate value to represent the size of the Qt 6 build while installed on disk (100 MiB).
ExtraDiskSpaceRequired=104857600
OutputManifestFile=Setup-Manifest.txt

[Code]
function ParamExists(const Param: string): boolean;
var
  K: integer;
begin
  for K := 1 to ParamCount() do
  begin
    if SameText(ParamStr(K), Param) then
    begin
      Result := true;
      Exit;
    end;
  end;
  Result := false;
end;

// If a different version of Daino Notes is already installed, this function will invoke its uninstaller.
// You can disable this behavior by invoking the installer via command line with /SKIPVERSIONCHECK.
function InitializeSetup(): boolean;
var
  ParamSilent, ParamVerySilent, ParamSuppressMsgBoxes: boolean;
  OurRegKey, Uninstaller, UninstallerParams, InstalledVersion: string;
  RootsToSearchIn: array of integer;
  K, ResultCode: integer;
  Version: TWindowsVersion;
begin
  if not IsWin64() and not (Version.Major >= 10) and not (Version.Minor >= 0) and not (Version.Build >= 17763) then
  begin
    MsgBox('Sorry, this application can only be installed on a 64-bit version of Windows 10 build 17763 and above.', mbError, MB_OK);
    Result := False;  // Quit the installer
  end;
  if ParamExists('/SKIPVERSIONCHECK') then
  begin
    Result := true;
    Exit;
  end;
  ParamSilent := ParamExists('/SILENT');
  ParamVerySilent := ParamExists('/VERYSILENT');
  ParamSuppressMsgBoxes := ParamExists('/SUPPRESSMSGBOXES');
  OurRegKey := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#SetupSetting("AppId")}_is1');
  SetLength(RootsToSearchIn, 1);
  RootsToSearchIn[0] := HKLM32;
  // We have to also search in HKLM64, because some older versions of the installer ran in 64-bit install mode.
  if IsWin64() then
  begin
    SetLength(RootsToSearchIn, 2);
    RootsToSearchIn[1] := HKLM64;
  end;
  for K := 0 to (GetArrayLength(RootsToSearchIn) - 1) do
  begin
    if not RegQueryStringValue(RootsToSearchIn[K], OurRegKey, 'UninstallString', Uninstaller) then
    begin
      continue;
    end;
    if not RegQueryStringValue(RootsToSearchIn[K], OurRegKey, 'DisplayVersion', InstalledVersion) then
    begin
      continue;
    end;
    if SameText('{#SetupSetting("AppVersion")}', InstalledVersion) then
    begin
      continue;
    end;
    // We run the _uninstaller_ with /VERYSILENT by default.
    UninstallerParams := '/VERYSILENT';
    // If _this_ installer was invoked with /SILENT or /SUPPRESSMSGBOXES, we forward those to the uninstaller.
    if ParamSilent then
    begin
      UninstallerParams := UninstallerParams + ' /SILENT';
    end;
    if ParamSuppressMsgBoxes then
    begin
      UninstallerParams := UninstallerParams + ' /SUPPRESSMSGBOXES';
    end;
    if not Exec(RemoveQuotes(Uninstaller), UninstallerParams, '', SW_SHOW, ewWaitUntilTerminated, ResultCode)
       or (ResultCode <> 0) then
    begin
      // Error message boxes can't be suppressed.
      MsgBox('Failed to uninstall version ' + InstalledVersion + '. Reason:'#13#13
             + SysErrorMessage(ResultCode) + '. (code ' + IntToStr(ResultCode) + ')'#13#13
             'Try uninstalling it from the Windows Control Panel.',
             mbCriticalError, MB_OK);
      Result := false;
      Exit;
    end;
  end;
  Result := true;
end;

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
; Qt 6 build (64-bit)
; Minimum supported OS:
; - Windows 10 21H2 (1809 or later)
; Source: https://doc.qt.io/qt-6/supported-platforms.html#windows
Source: "{#SourcePath}\DainoNotes64\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\qml\*"; DestDir: "{app}\qml"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\qmltooling\*"; DestDir: "{app}\qmltooling"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\dxcompiler.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\dxil.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\libcrypto-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\libssl-1_1-x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\msvcp140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\msvcp140_2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Daino Notes.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Core.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Gui.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6LabsFolderListModel.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Network.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Qml.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QmlModels.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QmlWorkerScript.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Quick.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2Basic.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2BasicStyleImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2Fusion.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2FusionStyleImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2Imagine.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2ImagineStyleImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2Impl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2Material.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2MaterialStyleImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2Universal.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2UniversalStyleImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickControls2WindowsStyleImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickDialogs2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickDialogs2QuickImpl.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickDialogs2Utils.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickLayouts.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickShapes.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Sql.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6QuickTemplates2.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Svg.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\Qt6Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763
Source: "{#SourcePath}\DainoNotes64\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64; MinVersion: 10.0.17763

[Icons]
Name: "{group}\Daino Notes"; Filename: "{app}\Daino Notes.exe"
Name: "{group}\{cm:UninstallProgram,Daino Notes}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Daino Notes"; Filename: "{app}\Daino Notes.exe"; Tasks: desktopicon 

[Run]
Filename: "{app}\Daino Notes.exe"; Description: "{cm:LaunchProgram,Daino Notes}"; Flags: nowait postinstall
