#include "totalGlobal.h"
#include "configmanager.h"

#include <QStandardPaths>
#include <QDir>



// Helper function: Determine the path to the settings file (.ini)
QString __getSettingsPath()  {

    // Determines the path to the INI file, usually: ~/.config/HashInspector/HashInspector.ini
    QString sPath = QStandardPaths::writableLocation( QStandardPaths::AppConfigLocation );

    // IMPORTANT: Create a folder if it does not already exist
    QDir().mkpath( sPath );

    // Return
    return sPath + "/HashInspector.ini";

}



// Helper function: Set the text color for "General Expenses" for the current theme
QColor __getSysTXTcolor()  {

    if ( CMI.iTheme == RADIOBUTTON_1 )  return LW_SYSTXT_COLOR_DARK;
    else  return LW_SYSTXT_COLOR_WHITE;

}

// Helper function: Set the "File Preview" text color for the current theme
QColor __getPreviewColor()  {

    if ( CMI.iTheme == RADIOBUTTON_1 )  return LW_PREVIEW_COLOR_DARK;
    else  return LW_PREVIEW_COLOR_WHITE;

}

// Helper function: Set the text color for "Hash found" for the current theme
QColor __getHashFoundColor()  {

    if ( CMI.iTheme == RADIOBUTTON_1 )  return LW_HASHFOUND_COLOR_DARK;
    else  return LW_HASHFOUND_COLOR_WHITE;

}

// Helper function: Set the background color when double-clicking for the current theme
QColor __getDCbackgroundColor()  {

    if ( CMI.iTheme == RADIOBUTTON_1 )  return LW_DCBACKGROUND_COLOR_DARK;
    else  return LW_DCBACKGROUND_COLOR_WHITE;

}




