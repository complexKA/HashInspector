#include "mainwindow.h"
#include "configmanager.h"
#include "totalGlobal.h"

#include <QSettings>
#include <QApplication>
#include <QSharedMemory>




int main( int argc, char *argv[] )
{
    //////////////////////////
    /// Save and set scaling
    /// To do this, the "UIscalingFactor" value must be loaded from the INI file before QApplication is called

    // IMPORTANT: Set the names BEFORE calling QStandardPaths
    QCoreApplication::setApplicationName( "HashInspector" );

    // Determine the path to the INI file
    QString sIniPath = __getSettingsPath();

    // Read values and set scaling
    QSettings xPreSettings( sIniPath, QSettings::IniFormat );
    xPreSettings.beginGroup( "Appearance" );

            double dUIScalingFactor = xPreSettings.value( "UIscalingFactor", 1.0           ).toDouble();
            int    iSingleInstance  = xPreSettings.value( "SingleInstance",  RADIOBUTTON_1 ).toInt();

    xPreSettings.endGroup();

    qputenv( "QT_SCALE_FACTOR", QByteArray::number(dUIScalingFactor) );

    // Prevents rounding errors when scaling factors are not integers
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy( Qt::HighDpiScaleFactorRoundingPolicy::PassThrough );

    // Create an application class
    QApplication a( argc, argv );
    a.setWindowIcon( QIcon(":/images/HashInspector-Logo.png") );        // Set an icon for Cinnamon




    //////////////////////////
    /// Prevent multiple instances

    // Define a unique key for the program
    // It is best to use a GUID or a very specific name

    QSharedMemory sharedMemory( "KA-HashInspector-050689310890" );

    if ( iSingleInstance == RADIOBUTTON_1 )  {

            // Attempt to create the segment (1 byte is sufficient)
            if ( !sharedMemory.create(1) ) {

                // Creation failed -> An instance is already running
                return 0;   // Immediately terminates the second instance

            }

    }



    //////////////////////////
    /// Load settings from the INI file and create the main window

    // Open the main window
    MainWindow w;  w.show();
    ConfigManager::instance();      // Create a CMI instance, load the INI file
    return a.exec();
}
