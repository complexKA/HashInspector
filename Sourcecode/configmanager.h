#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "totalGlobal.h"

#include <QSettings>
#include <QDir>



class ConfigManager {

public:

    // Access to the single instance (singleton)
    static ConfigManager& instance() {

        static ConfigManager _instance;
        return _instance;

    }

    // ---- Save values ----
    QByteArray xMainWinGeo, xFileDialogGeo;
    QString    sLastOpenDir;
    QString    sLastExportDir;

    double  dScaleAtLoad     = 1.0;
    bool    bResetWposSize   = false;

    // ---- Counters ----
    int     iStartCounter    = 0;
    int     iInspectCounter  = 0;

    // ---- Settings values ----
    int     iInstance        = RADIOBUTTON_0;       // General section

    bool    bSaveWindowsPS   = true;
    bool    bFDsaveLastDir   = true;
    bool    bFDsavePosSize   = true;

    int     iTheme           = RADIOBUTTON_1;       // Appearance section

    double  dUIscalingFactor = 1.0;
    bool    bShowTooltips    = true;
    bool    bHideLogo        = false;

    int     iLWfontSize = COMBOBOX_4, iLWspacing = COMBOBOX_1;

    QString sAllowedExts;                           // Inspection section

    bool    bHashF_MD5       = false;       bool bHashF_SHA512  = false;
    bool    bHashF_SHA1      = false;       bool bHashF_SHA3256 = false;
    bool    bHashF_SHA256    = false;       bool bHashF_SHA3512 = false;

    int     iHashFormat      = RADIOBUTTON_0;
    int     iOnHashNotFound  = RADIOBUTTON_3;

    bool    bShowCompHash    = false;
    bool    bShowScanPath    = false;
    bool    bIncludeSubf     = true;
    bool    bShowIcons       = true;

    QString sExcludePat;                            // Filter section

    int     iAutoStopMM      = OFF;                 // Auto section
    int     iAutoStopAI      = OFF;
    bool    bResetContents   = false;

    int     iPrioCPU         = COMBOBOX_1;          // Process section
    int     iPrioIO          = COMBOBOX_0;


    // Help function: Trim QString
    QString __max( QString sString, int iMax )  {

            if ( sString.length() > iMax )  sString = sString.left( iMax );
            return sString;

    }


    ////////////////////////////////////////////////
    ///
    ///   Load all values from the INI file
    ///
    ////////////////////////////////////////////////

    void load()  {

        QSettings s( __getSettingsPath(), QSettings::IniFormat );

        // Forces Qt to reload the INI file from the hard drive
        s.sync();

        // Window geometry and last opened directory
        s.beginGroup( "Geometry" );

            xMainWinGeo      = s.value( "MainWinGeo"       ).toByteArray();
            xFileDialogGeo   = s.value( "FileDialog"       ).toByteArray();

        s.endGroup();

        s.beginGroup( "Last" );

            sLastOpenDir     = s.value( "LastOpenDir",     QDir::homePath() ).toString();    sLastOpenDir   = __max( sLastOpenDir  , 255 );
            sLastExportDir   = s.value( "LastExportDir",   QDir::homePath() ).toString();    sLastExportDir = __max( sLastExportDir, 255 );

        s.endGroup();

        s.beginGroup( "Counter" );

            iStartCounter    = s.value( "StartCounter",    0 ).toInt();  iStartCounter++;
            iInspectCounter  = s.value( "InspectCounter",  0 ).toInt();

        s.endGroup();

        // General section
        //s.beginGroup( "General" );

            iInstance        = s.value( "SingleInstance",  RADIOBUTTON_1 ).toInt();
            bSaveWindowsPS   = s.value( "SaveWindowsPS",   true          ).toBool();
            bFDsaveLastDir   = s.value( "FDsaveLastDir",   true          ).toBool();     if ( bFDsaveLastDir == false )  sLastOpenDir = QDir::homePath();
            bFDsavePosSize   = s.value( "FDsavePosSize",   true          ).toBool();
            bResetWposSize   = s.value( "ResetWposSize",   false         ).toBool();

        //s.endGroup();

        // Appearance section
        s.beginGroup( "Appearance" );

            iTheme           = s.value( "Theme",           RADIOBUTTON_1 ).toInt();
            dUIscalingFactor = s.value( "UIscalingFactor", 1.0           ).toDouble();

            bShowTooltips    = s.value( "ShowTooltips",    true          ).toBool();
            bHideLogo        = s.value( "HideLogo",        false         ).toBool();

            iLWfontSize      = s.value( "LWfontSize",      COMBOBOX_4    ).toInt();
            iLWspacing       = s.value( "LWspacing",       COMBOBOX_1    ).toInt();

        s.endGroup();

        // Inspection section
        s.beginGroup( "Inspection" );

            sAllowedExts     = s.value( "AllowedExts",     ""            ).toString();

            bHashF_MD5       = s.value( "HashF_MD5",       false         ).toBool();
            bHashF_SHA1      = s.value( "HashF_SHA1",      false         ).toBool();
            bHashF_SHA256    = s.value( "HashF_SHA256",    true          ).toBool();
            bHashF_SHA512    = s.value( "HashF_SHA512",    false         ).toBool();
            bHashF_SHA3256   = s.value( "HashF_SHA3256",   false         ).toBool();
            bHashF_SHA3512   = s.value( "HashF_SHA3512",   false         ).toBool();

            iHashFormat      = s.value( "HashFormat",      RADIOBUTTON_0 ).toInt();
            iOnHashNotFound  = s.value( "OnHashNotFound",  RADIOBUTTON_3 ).toInt();

            bShowCompHash    = s.value( "ShowCompHash",    false         ).toBool();
            bShowScanPath    = s.value( "ShowScanPath",    false         ).toBool();
            bIncludeSubf     = s.value( "IncludeSubf",     true          ).toBool();
            bShowIcons       = s.value( "ShowIcons",       true          ).toBool();

        s.endGroup();

        // Filter section
        s.beginGroup( "Filter" );

            sExcludePat      = s.value( "ExcludePatterns", FILTER_DEFAULTPAT ).toString();      sExcludePat = __max( sExcludePat, 1024 );

        s.endGroup();

        // Auto section
        s.beginGroup( "Auto" );

            iAutoStopMM      = s.value( "AutoStopMM",      OFF           ).toInt();
            iAutoStopAI      = s.value( "AutoStopAI",      OFF           ).toInt();
            bResetContents   = s.value( "ResetContents",   false         ).toBool();

        s.endGroup();

        // Priority section
        s.beginGroup( "Priority" );

            iPrioCPU         = s.value( "CPU",             COMBOBOX_1    ).toInt();
            iPrioIO          = s.value( "IO",              COMBOBOX_0    ).toInt();

        s.endGroup();

        // Sonstiges
        dScaleAtLoad = dUIscalingFactor;        // ScalingFactor zum Start merken

    }



    ////////////////////////////////////////////////
    ///
    ///   Save all values to the INI file
    ///
    ////////////////////////////////////////////////

    void save()  {

        QSettings s( __getSettingsPath(), QSettings::IniFormat );
        s.clear();

        // Synchronize first (loads changes from other instances into memory)
        s.sync();

        // Window geometry and last opened directory
        if ( bSaveWindowsPS )  {

        s.beginGroup( "Geometry" );

            s.setValue( "MainWinGeo",      xMainWinGeo      );        // Window dimensions
            s.setValue( "FileDialog",      xFileDialogGeo   );

        s.endGroup();

        }

        // Last paths, etc.
        s.beginGroup( "Last" );

            s.setValue( "LastOpenDir",     sLastOpenDir     );        // The last directory opened
            s.setValue( "LastExportDir",   sLastExportDir   );        // Last directory at Export

        s.endGroup();

        // Counter
        s.beginGroup( "Counter" );

            s.setValue( "StartCounter",    iStartCounter    );
            s.setValue( "InspectCounter",  iInspectCounter  );

        s.endGroup();

        // General section
        //s.beginGroup( "General" );

            s.setValue( "SingleInstance",  iInstance        );        // Radio buttons
            s.setValue( "SaveWindowsPS",   bSaveWindowsPS   );        // Checkbox Main window position and size
            s.setValue( "FDsaveLastDir",   bFDsaveLastDir   );        // Checkbox Last directory
            s.setValue( "FDsavePosSize",   bFDsavePosSize   );        // Checkbox Save File Dialog position and size

            if ( dScaleAtLoad != dUIscalingFactor )  s.setValue( "ResetWposSize", true );
            else  s.setValue( "ResetWposSize", false );

        //s.endGroup();

        // Appearance section
        s.beginGroup( "Appearance" );

            s.setValue( "Theme",           iTheme           );        // Radio buttons
            s.setValue( "UIscalingFactor", dUIscalingFactor );        // UI Scaling Factor

            s.setValue( "ShowTooltips",    bShowTooltips    );        // Checkbox Show tooltips
            s.setValue( "HideLogo",        bHideLogo        );        // Checkbox Hide Logo

            s.setValue( "LWfontSize",      iLWfontSize      );        // Combobox Font size
            s.setValue( "LWspacing",       iLWspacing       );        // Combobox spacing

        s.endGroup();

        // Inspection section
        s.beginGroup( "Inspection" );

            s.setValue( "AllowedExts",     sAllowedExts     );        // Line Edit Allowed Extensions

            s.setValue( "HashF_MD5",       bHashF_MD5       );        // Checkbox MD5
            s.setValue( "HashF_SHA1",      bHashF_SHA1      );        // Checkbox SHA1
            s.setValue( "HashF_SHA256",    bHashF_SHA256    );        // Checkbox SHA256
            s.setValue( "HashF_SHA512",    bHashF_SHA512    );        // Checkbox SHA512
            s.setValue( "HashF_SHA3256",   bHashF_SHA3256   );        // Checkbox SHA3-256
            s.setValue( "HashF_SHA3512",   bHashF_SHA3512   );        // Checkbox SHA3-512

            s.setValue( "HashFormat",      iHashFormat      );        // Radiobuttons Hash Format
            s.setValue( "OnHashNotFound",  iOnHashNotFound  );        // Radiobuttons On Hash Not Found

            s.setValue( "ShowCompHash",    bShowCompHash    );        // Checkbox Show computed hash on mismatch
            s.setValue( "ShowScanPath",    bShowScanPath    );        // Checkbox Show scanning path
            s.setValue( "IncludeSubf",     bIncludeSubf     );        // Checkbox Include Subfolders
            s.setValue( "ShowIcons",       bShowIcons       );        // Checkbox Show icons

        s.endGroup();

        // Filter section
        s.beginGroup( "Filter" );

            s.setValue( "ExcludePatterns", sExcludePat      );

        s.endGroup();

        // Auto section
        s.beginGroup( "Auto" );

            s.setValue( "AutoStopMM",      iAutoStopMM      );        // Spinbox Auto-stop hash mismatch
            s.setValue( "AutoStopAI",      iAutoStopAI      );        // Spinbox Auto-stop after inspection
            s.setValue( "ResetContents",   bResetContents   );        // Checkbox Reset Contents

        s.endGroup();

        // Priority section
        s.beginGroup( "Priority" );

            s.setValue( "CPU",             iPrioCPU         );        // Combobox CPU Priority
            s.setValue( "IO",              iPrioIO          );        // Combobox I/O Priority

        s.endGroup();

        // Force sync to ensure that the file is written immediately
        s.sync();

    }

private:
    ConfigManager() { load(); }     // Private constructor

    // Prevent copying
    ConfigManager( const ConfigManager& ) = delete;
    void operator=( const ConfigManager& ) = delete;
};

#endif

