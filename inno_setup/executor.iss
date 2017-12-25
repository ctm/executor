

[Setup]
AppName=Executor
AppVerName=Executor version 2.1pr16
DefaultDirName={pf}\Executor
DisableProgramGroupPage=yes
; ^ since no icons will be created in "{group}", we don't need the wizard
;   to ask for a group name.
UninstallDisplayIcon={app}\Executor.exe

[Types]
Name: "full"; Description: "Full installation"
Name: "compact"; Description: "Compact installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "Executor"; Description: "Executor and required files"; Types: full compact custom; Flags: fixed

Name: "Freeware"; Description: "Free Programs"; Types: full
Name: "Freeware\Risk"; Description: "Risk! - Board Game"; Types: full

Name: "Demoware"; Description: "Limited Demo Programs"; Types: full
Name: "Demoware\StuffIt_Expander"; Description: "StuffIt Expander - Archive Expander"; Types: full
Name: "Demoware\MacBreadboard"; Description: "MacBreadboard - Circuit Exploration"; Types: full
Name: "Demoware\Ultimate_Solitaire"; Description: "Eric's Ultimate Solitaire - Solitaire"; Types: full
Name: "Demoware\lemmings"; Description: "Oh No! More Lemmings - Help Lemmings Get Home"; Types: full

Name: "Shareware"; Description: "Programs you must pay for if you use"; Types: full
Name: "Shareware\Tex_Edit"; Description: "Tex-Edit - Text Editor"; Types: full
Name: "Shareware\Speedometer"; Description: "Speedometer - Benchmark Tool"; Types: full

[Dirs]
Name: "{app}\Apps\System Folder\Extensions"
Name: "{app}\Apps\System Folder\Preferences"

[Files]
Source: "Executor.exe"; DestDir: "{app}"; Components: Executor
Source: "Readme.txt"; DestDir: "{app}"; Components: Executor; Flags: isreadme
Source: "exemove.exe"; DestDir: "{app}"; Components: Executor
Source: "makehfv.exe"; DestDir: "{app}"; Components: Executor
Source: "printdef.ini"; DestDir: "{app}"; Components: Executor
Source: "printers.ini"; DestDir: "{app}"; Components: Executor
Source: "tips.txt"; DestDir: "{app}"; Components: Executor
Source: "dirMap-le"; DestDir: "{app}"; Components: Executor
Source: "README-SDL.txt"; DestDir: "{app}"; Components: Executor
Source: "SDL.dll"; DestDir: "{app}"; Components: Executor
Source: "cdenable.sys"; DestDir: "{sys}"; Components: Executor
Source: "Apps\System Folder\%windows.rsrc"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%mac.rsrc"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\Browser"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\windows.rsrc"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%Browser"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%Extensions"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\godata.sav"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%godata.sav"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\ParamRAM"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%ParamRAM"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%Preferences"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\Printer"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%Printer"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\System"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%System"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\system.ard"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\%system.ard"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "Apps\System Folder\mac.rsrc"; DestDir: "{app}\Apps\System Folder"; Components: Executor
Source: "configur\50217268.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4c415a48.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\47534361.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\444f4c4c.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\43475246.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4c505243.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\47756e53.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\44576174.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\43504354.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\32362e32.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\483444c6.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\44616e47.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4352534c.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\3842494d.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\454a3035.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4354494d.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\3f3f3f3f.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\57494c44.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\55505550.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\43555341.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\41443344.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\424f424f.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\6f7a6d35.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\556c7433.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\53635246.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\414f7163.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\5354775a.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\72647020.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\57435345.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\5368537a.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\52415a5a.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\41525435.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\45474144.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\72706db5.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\574f4c46.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\53704a4b.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\5249534b.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\50474150.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\41527c46.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\494e5455.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\73506433.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\57504332.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\53706563.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\524a4253.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\504c5073.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d427264.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4154726e.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\57696c64.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\a78ea8a0.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\5843454c.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\5370696e.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\524c4d5a.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\504e4331.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d4b444e.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\48525630.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4164426b.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\41324d68.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\a7bfc2a2.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\58505233.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\54424235.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\52565253.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\50505456.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d4d4343.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\48525631.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\45535041.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\41706569.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\6b616a72.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\ac7e5ea0.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\63417244.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\54424236.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\526a3031.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\50566d74.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d4d5042.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\48734c61.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\455544c6.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\43574c44.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\484c5832.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\f5536b69.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\674f4c46.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\544b4e4f.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\53414e54.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\50674c67.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d504e54.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\496d6167.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\46424420.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\43595153.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\42454132.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\434c4144.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\6c6f6733.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\54745264.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\53495421.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\506fc450.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d535744.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4a4b5445.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\466c6974.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\43764d4d.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\424f4c4f.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d917263.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\54775231.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\53495478.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\50736f64.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d61656c.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4a524735.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\46756e47.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4444534b.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\426163d5.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\534d4c53.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\51444c58.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d687a75.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4a70616b.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\47436f6e.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\44454c49.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\42646c6d.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\522a6368.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4d6f6c54.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\4b705353.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\474f474f.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\44494132.ecf"; DestDir: "{app}\configur"; Components: Executor
Source: "configur\426e4871.ecf"; DestDir: "{app}\configur"; Components: Executor

Source: "Apps\Shareware\Tex-Edit\Revision History"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\%Revision History"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\Tex-Edit"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\%Tex-Edit"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\Tex-Edit Prefs"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\%Tex-Edit Prefs"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\Tex-Edit Reference"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\%Tex-Edit Reference"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\Welcome to Tex-Edit!"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\Tex-Edit\%Welcome to Tex-Edit!"; DestDir: "{app}\Apps\Shareware\Tex-Edit"; Components: Shareware\Tex_Edit
Source: "Apps\Shareware\%Tex-Edit"; DestDir: "{app}\Apps\Shareware"; Components: Shareware\Tex_Edit

Source: "Apps\Shareware\About Shareware"; DestDir: "{app}\Apps\Shareware"; Components: Shareware
Source: "Apps\Shareware\%About Shareware"; DestDir: "{app}\Apps\Shareware"; Components: Shareware

Source: "Apps\Shareware\%speedometer3.23 Folder"; DestDir: "{app}\Apps\Shareware"; Components: Shareware\Speedometer
Source: "Apps\Shareware\speedometer3.23 Folder\Speedo 3.21%D1%3E3.23 Doc"; DestDir: "{app}\Apps\Shareware\speedometer3.23 Folder"; Components: Shareware\Speedometer
Source: "Apps\Shareware\speedometer3.23 Folder\%Speedo 3.21%D1%3E3.23 Doc"; DestDir: "{app}\Apps\Shareware\speedometer3.23 Folder"; Components: Shareware\Speedometer
Source: "Apps\Shareware\speedometer3.23 Folder\Speedometer 3.23"; DestDir: "{app}\Apps\Shareware\speedometer3.23 Folder"; Components: Shareware\Speedometer
Source: "Apps\Shareware\speedometer3.23 Folder\%Speedometer 3.23"; DestDir: "{app}\Apps\Shareware\speedometer3.23 Folder"; Components: Shareware\Speedometer

Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%What's new in StuffIt Expander%AA"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\Aladdin Order Form"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%Aladdin Order Form"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\Aladdin Product Overview"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%Aladdin Product Overview"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\Expander Reg. Form"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%Expander Reg. Form"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\Icon%0D"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%Icon%0D"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\License Agreement"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%License Agreement"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\StuffIt Expander%AA Read Me"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%StuffIt Expander%AA Read Me"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\What was installed (and where)"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\%What was installed (and where)"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!\What's new in StuffIt Expander%AA"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Read Us First!"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\Icon%0D"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\%Icon%0D"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\%Read Us First!"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\StuffIt Expander%AA"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder\%StuffIt Expander%AA"; DestDir: "{app}\Apps\Demoware\StuffIt Expander%AA 4.0.1 Folder"; Components: Demoware\StuffIt_Expander
Source: "Apps\Demoware\%StuffIt Expander%AA 4.0.1 Folder"; DestDir: "{app}\Apps\Demoware"; Components: Demoware\StuffIt_Expander

Source: "Apps\Demoware\MacBreadboard%C4\4-bit Counter"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\%4-bit Counter"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\D Flip%2FFlop Demo"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\%D Flip%2FFlop Demo"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\Decade Counter"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\%Decade Counter"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\MacBreadboard DEMO 1.1"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\MacBreadboard%C4\%MacBreadboard DEMO 1.1"; DestDir: "{app}\Apps\Demoware\MacBreadboard%C4"; Components: Demoware\MacBreadboard
Source: "Apps\Demoware\%MacBreadboard%C4"; DestDir: "{app}\Apps\Demoware"; Components: Demoware\MacBreadboard

Source: "Apps\Demoware\About Demoware"; DestDir: "{app}\Apps\Demoware"; Components: Demoware
Source: "Apps\Demoware\%About Demoware"; DestDir: "{app}\Apps\Demoware"; Components: Demoware

Source: "Apps\Demoware\Ultimate Solitaire Demo"; DestDir: "{app}\Apps\Demoware"; Components: Demoware\Ultimate_Solitaire
Source: "Apps\Demoware\%Ultimate Solitaire Demo"; DestDir: "{app}\Apps\Demoware"; Components: Demoware\Ultimate_Solitaire

Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%BW Graphics"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%Graphics"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%Levels"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%Music"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%Oh No! More Lemmings Demo"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%Read Me First (MS Word Format)"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\%Read Me First%7F"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\BW Graphics"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\Graphics"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\Levels"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\Music"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\Oh No! More Lemmings Demo"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\Read Me First (MS Word Format)"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings
Source: "Apps\Demoware\Oh No!  More Lemmings Demo\Read Me First%7F"; DestDir: "{app}\Apps\Demoware\Oh No!  More Lemmings Demo"; Components: Demoware\lemmings

Source: "Apps\Freeware\About Freeware"; DestDir: "{app}\Apps\Freeware"; Components: Freeware
Source: "Apps\Freeware\%About Freeware"; DestDir: "{app}\Apps\Freeware"; Components: Freeware
Source: "Apps\Freeware\Risk!"; DestDir: "{app}\Apps\Freeware"; Components: Freeware\Risk
Source: "Apps\Freeware\%Risk!"; DestDir: "{app}\Apps\Freeware"; Components: Freeware\Risk


[Icons]
Name: "{commonprograms}\Executor"; Filename: "{app}\Executor.exe"
Name: "{commonprograms}\Executor Full-Screen"; Filename: "{app}\Executor.exe"; Parameters: "-fullscreen"
Name: "{userdesktop}\Executor"; Filename: "{app}\Executor.exe"
Name: "{userdesktop}\Executor Full-Screen"; Filename: "{app}\Executor.exe"; Parameters: "-fullscreen"
