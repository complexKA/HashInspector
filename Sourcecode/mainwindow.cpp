#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "configmanager.h"
#include "totalGlobal.h"
#include "worker.h"
#include "appsettings.h"
#include "about.h"

#include <QActionGroup>
#include <QMessageBox>
#include <QFileDialog>
#include <QDirIterator>
#include <QFile>
#include <QCryptographicHash>
#include <QThread>
#include <QClipboard>
#include <QTimer>
#include <QDesktopServices>

#include <qtimer.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/ioprio.h>
#include <sys/resource.h>



// Constructor
MainWindow::MainWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )  {

    ui->setupUi( this );


    // Set an icon for Cinnamon
    setWindowIcon( QIcon(":/images/HashInspector-Logo.png") );

    // Insert logo from the resource
    QPixmap picLogo( ":/images/HashInspector-Logo.png" );
    ui->label_mainLogo->setPixmap( picLogo.scaled(45,45,Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    ui->label_mainLogo->setStyleSheet( "margin: -5px 0 0 1px;" );

    // Do not accept files via drag and drop
    ui->listWidget->setAcceptDrops( true );



    //////////////////////////////
    /// Toolbar

    // Remove the border around the toolbar and the currently active icon
    ui->toolBar->setStyleSheet(

            "QToolBar { border: none; }"
            "QToolButton { margin:1px 5px 0 15px; border:none; padding:0; }"
            "QToolButton:checked { border: none; background-color: transparent; }"
            "QToolButton:pressed { border: none; background-color: transparent; }"
            "QToolButton:active { border: none; background-color: transparent; }"
            "QToolButton#SDLend { margin-bottom:15px; }"

    );

    // Space above
    QWidget* topSpacer = new QWidget();
    topSpacer->setMinimumHeight( 25 );
    ui->toolBar->insertWidget( ui->toolBar->actions().at(0), topSpacer );

    // Enable Settings, Info, and Quit
    connect( ui->actionSettings, &QAction::triggered, this, &MainWindow::onSettingsClicked );
    connect( ui->actionInfo,     &QAction::triggered, this, &MainWindow::onInfoClicked );
    connect( ui->actionQuit,     &QAction::triggered, this, &MainWindow::onQuitClicked );


    //////////////////////////////
    /// ListWidget

    ui->listWidget->setStyleSheet( "QListWidget { border:none; background-color:#000; padding:5px 2px; margin:2px 0 0 0; }" );

    ui->listWidget->setSelectionMode( QAbstractItemView::NoSelection );           // Selektionen sollen nicht möglich sein
    ui->listWidget->setFocusPolicy( Qt::NoFocus );                               // Kein Focus für die Zeilen
    ui->listWidget->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );     // Horizontale Scrollbar ausschalten
    ui->listWidget->setIconSize( QSize(ICONSIZE,ICONSIZE) );                   // Größe der Icons

    connect( ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::__copyTooltipToClipboard );


    //////////////////////////////
    /// Appearance and Behavior

  __setGeomentry();                 // Restore window dimensions
  __setTheme( CMI.iTheme );        // Theme

    setHIstatus1Text( HISTATUS_DEFAULT );
    ui->label_HIstatus2->clear();

    // Turn buttons on or off, enable or disable them
    ui->pb_Open->setEnabled( true );
    ui->pb_Clear->setVisible( false );
    ui->pb_Export->setVisible( false );
    ui->pb_Abort->setVisible( false );

    // A flag indicates that the scaling has changed since the last launch: reset the window
    if ( CMI.bResetWposSize == true )  resetMainWindowPosSize();


    //////////////////////////////
    /// Additional settings

    // Set event filter to intercept the ESC key
    qApp->installEventFilter( this );

}


MainWindow::~MainWindow()
{
    delete ui;
}




// -------------------------------------------------- Overridden functions ----------------------------------------------





// Override closeEvent() to save the window position
void MainWindow::closeEvent( QCloseEvent *event ) {

    CMI.xMainWinGeo = saveGeometry();         // Save the geometry of the current mode
    CMI.save();                              // Initiate the saving of settings via ConfigManager
    QMainWindow::closeEvent( event );       // Open Original Destrukor
}


// EVENTFILTER
// Filters out any tooltips, if necessary
bool MainWindow::eventFilter( QObject *objWatched, QEvent *event ) {

    // Do nothing when the tooltip event occurs
    if ( event->type() == QEvent::ToolTip && !CMI.bShowTooltips )  {

        event->ignore();  return true;

    }

    // Call the default behavior of the base class
    return QMainWindow::eventFilter( objWatched, event );
}


// Event: The Window is displayed
void MainWindow::showEvent( QShowEvent *event ) {

  __setCurrentPriority();         // Set priority
  __setLWandEntryStyle();        // Set the (possibly modified) appearance of the ListWidget and its entries

    // The logo at the top can be hidden if desired
    if ( CMI.bHideLogo == true )  {  ui->widget_LogoArea->hide();
                                     this->setStyleSheet( "#label_HIstatus { padding-top: 20px; }" );
                                  }
    else  {  ui->widget_LogoArea->show();
             this->setStyleSheet( "#label_HIstatus { padding-top: 0; }" );
          }

    QWidget::showEvent( event );     // Call the base class

}




// ------------------------------------------------ Private functions ------------------------------------------------


// Resets the position and size of the main window to their default values
void MainWindow::resetMainWindowPosSize( void )  {

    // Determine the primary display
    QScreen *xPrimaryScreen = QGuiApplication::primaryScreen();

    // Retrieve the dimensions of the main screen (without the taskbar)
    QRect rectScreenGeometry = xPrimaryScreen->availableGeometry();

    // Set the initial size of the window
    this->resize( MAINWIN_MINW, MAINWIN_MINH );

    // Move window
    this->move( rectScreenGeometry.left() + 100, rectScreenGeometry.top() + 100 );
}



// Set the window's dimensions and position if they were saved in lnkModder.ini
// Otherwise, set the default value
void MainWindow::__setGeomentry( void )  {

    // Set window dimensions
    this->setMinimumSize( MAINWIN_MINW, MAINWIN_MINH );
    this->setMaximumSize( MAINWIN_MAXW, MAINWIN_MAXH );

    // Retrieve saved window dimensions based on the mode
    QByteArray geo = CMI.xMainWinGeo;

    // Set window size or default
    if ( !geo.isEmpty() )  restoreGeometry( geo );       // Restore saved dimensions and position
    else  resetMainWindowPosSize();                     // Resets the position and size of the main window to their default values

    // IMPORTANT: After calling setWindowFlags, we must call show()
    // to make the window visible again
    this->show();

}




// Set White Mode or Dark Mode
void MainWindow::__setTheme( int iTheme ) {

    if ( iTheme == RADIOBUTTON_1 )  {

            //////////////////////////////
            /// DARKMODE

            QPalette darkPal;
            darkPal.setColor(QPalette::Window,          QColor(55, 55, 55));
            darkPal.setColor(QPalette::WindowText,      Qt::white);
            darkPal.setColor(QPalette::Base,            QColor(0,0,0));
            darkPal.setColor(QPalette::AlternateBase,   QColor(45, 45, 45));
            darkPal.setColor(QPalette::ToolTipBase,     Qt::white);
            darkPal.setColor(QPalette::ToolTipText,     Qt::white);
            darkPal.setColor(QPalette::Text,            Qt::white);
            darkPal.setColor(QPalette::Button,          QColor(45, 45, 45));
            darkPal.setColor(QPalette::ButtonText,      Qt::white);
            darkPal.setColor(QPalette::BrightText,      Qt::red);
            darkPal.setColor(QPalette::Highlight,       QColor(42, 130, 218));
            darkPal.setColor(QPalette::HighlightedText, Qt::black);

            // The color for normal links (e.g., light blue)
            darkPal.setColor(QPalette::Link, Qt::white);

            // The color for links that have already been visited (e.g., a lighter shade of purple)
            darkPal.setColor(QPalette::LinkVisited, Qt::white);

            // The color of the frame (often Mid or Dark)
            darkPal.setColor(QPalette::Mid, QColor(90, 90, 90));
            darkPal.setColor(QPalette::Light, QColor(160, 160, 160)); // Eine hellere Akzentkante

            // Shadow should remain dark to ensure the right contrast
            darkPal.setColor(QPalette::Shadow, Qt::black);

            // Text color for DISABLED elements (buttons, labels, etc.)
            darkPal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 120));
            darkPal.setColor(QPalette::Disabled, QPalette::Text,       QColor(120, 120, 120));
            darkPal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));

            // Optional: Slightly darken the background of disabled input fields as well
            darkPal.setColor(QPalette::Disabled, QPalette::Base,       QColor(40, 40, 40));

            // Horizontal line
            darkPal.setColor(QPalette::Mid, QColor(67, 67, 67));
            darkPal.setColor(QPalette::Dark, QColor(60, 60, 60));
            darkPal.setColor(QPalette::Light, QColor(67, 67, 67));

            qApp->setPalette( darkPal );

    } else {

            //////////////////////////////
            /// WHITEMODE

            // Get a clean, fresh palette
            QPalette lightPal = QApplication::palette();

            // Explicitly reset the critical roles to "Hell"
            lightPal.setColor(QPalette::Window,          QColor(240, 240, 240));
            lightPal.setColor(QPalette::WindowText,      Qt::black);
            lightPal.setColor(QPalette::Base,            Qt::white);
            lightPal.setColor(QPalette::Text,            Qt::black);
            lightPal.setColor(QPalette::Button,          QColor(240, 240, 240));
            lightPal.setColor(QPalette::ButtonText,      Qt::black);
            lightPal.setColor(QPalette::Link,            Qt::blue);

            // Specifically for DISABLED elements on a light background (dark gray on light gray)
            QColor disabledGray(160, 160, 160);     // A classic "grayed-out"
            lightPal.setColor(QPalette::Disabled, QPalette::WindowText, disabledGray);
            lightPal.setColor(QPalette::Disabled, QPalette::Text,       disabledGray);
            lightPal.setColor(QPalette::Disabled, QPalette::ButtonText, disabledGray);

            // Optional: Slightly darken the background of disabled fields
            lightPal.setColor(QPalette::Disabled, QPalette::Base, QColor(225, 225, 225));

            qApp->setPalette( lightPal );

    }


    // Set tooltips based on the theme
    if ( iTheme == RADIOBUTTON_1 ) {

            // Dark mode: Dark background, light text
            qApp->setStyleSheet(
                "QToolTip { "
                "color: #ffffff; "
                "background-color: #353535; "
                "border: 1px solid #555555; "
                "}"
            );

    } else {

            // Light mode: Light background, dark text
            qApp->setStyleSheet(
                "QToolTip { "
                "color: #000000; "
                "background-color: #f0f0f0; "
                "border: 1px solid #ababab; "
                "}"
            );
    }

}




// Set current priority
void MainWindow::__setCurrentPriority( void )  {

    // CPU (only works if you have root priv.)
    int iNice = 0;
    switch( CMI.iPrioCPU )  {  case COMBOBOX_0  :  iNice = 10;   break;      // Low
                               case COMBOBOX_2  :  iNice = -10;  break;      // High

                               default          :
                               case COMBOBOX_1  :  iNice = 0;    break;      // Normal
                            }

    setpriority( PRIO_PROCESS, 0, iNice );

    // I/O
    int iIOPrioValue = 0;

    switch( CMI.iPrioIO )  {

        case COMBOBOX_0 :   // Class IDLE (3); the level is hardly a factor here (0)
                            iIOPrioValue = IOPRIO_PRIO_VALUE( IOPRIO_CLASS_IDLE, 0 );
                            break;

        default:
        case COMBOBOX_1 :   // BEST-EFFORT Class (2), Level 4 (Standard)
                            iIOPrioValue = IOPRIO_PRIO_VALUE( IOPRIO_CLASS_BE, 4 );
                            break;

    }

    // Pass the combined value now
    syscall( SYS_ioprio_set, IOPRIO_WHO_PROCESS, 0, iIOPrioValue );


    //QMessageBox::about( 0, "Mein Titel", QString("ID: %1 - CPU: %2 - IO: %3").arg(getpid()).arg(iNice).arg(iIOnice) );

}





// The appearance of the ListWidget and its entries is configured
void MainWindow::__setLWandEntryStyle( void )  {

    ////////////////////////////
    /// Appearance of the ListView element
    /// 1: Background, font colors, and font weights depending on the theme

    // Set dark theme settings
    if ( CMI.iTheme == RADIOBUTTON_1 )  {

            QString sBackground = "background-color:#000";

            ui->listWidget->setStyleSheet( QString("QListWidget { %1 padding:5px 2px; }").arg(sBackground) );

            ui->label_HIstatus1->setStyleSheet( "QLabel { margin:0 2px 1px 0; color:rgb(160,160,160); }" );     // Text color of status 1
            ui->label_HIstatus2->setStyleSheet( "QLabel { margin:0 15px 0 0; color:rgb(160,160,160); }" );     // Text color of status 2

            // Turn off the frame completely
            ui->listWidget->setFrameShape( QFrame::NoFrame );     // Disables the border style
            ui->listWidget->setFrameShadow( QFrame::Plain );     // Set the shadow to "flat"
            ui->listWidget->setLineWidth( 0 );                  // Sets the line weight to zero

            // Farbtauscher
            for( int i = 0; i < ui->listWidget->count(); ++i )  {

                    QListWidgetItem* item = ui->listWidget->item( i );

                    if ( item->foreground().color() == LW_SYSTXT_COLOR_WHITE       )  item->setForeground( LW_SYSTXT_COLOR_DARK       );
                    if ( item->foreground().color() == LW_PREVIEW_COLOR_WHITE      )  item->setForeground( LW_PREVIEW_COLOR_DARK      );
                    if ( item->foreground().color() == LW_HASHFOUND_COLOR_WHITE    )  item->setForeground( LW_HASHFOUND_COLOR_DARK    );

                    if ( item->background().color() == LW_DCBACKGROUND_COLOR_WHITE )  item->setBackground( LW_DCBACKGROUND_COLOR_DARK );

            }

    }

    // Set settings for the white theme
    if ( CMI.iTheme == RADIOBUTTON_0 )  {

            QString sBackground = "background-color:rgb(230,230,230);";

            ui->listWidget->setStyleSheet( QString("QListWidget { %1 padding:5px 2px; }").arg(sBackground) );

            ui->label_HIstatus1->setStyleSheet( "QLabel { margin:0 2px 1px 0; color:rgb(30,30,30); }" );     // Text color of status 1
            ui->label_HIstatus2->setStyleSheet( "QLabel { margin:0 15px 0 0; color:rgb(30,30,30); }" );     // Text color of status 2

            // Rahmen richtig setzen
            ui->listWidget->setFrameShape( QFrame::Box     );
            ui->listWidget->setFrameShadow( QFrame::Raised );
            ui->listWidget->setLineWidth( 1 );

            // Farbtauscher
            for( int i = 0; i < ui->listWidget->count(); ++i )  {

                    QListWidgetItem* item = ui->listWidget->item( i );

                    if ( item->foreground().color() == LW_SYSTXT_COLOR_DARK       )  item->setForeground( LW_SYSTXT_COLOR_WHITE       );
                    if ( item->foreground().color() == LW_PREVIEW_COLOR_DARK      )  item->setForeground( LW_PREVIEW_COLOR_WHITE      );
                    if ( item->foreground().color() == LW_HASHFOUND_COLOR_DARK    )  item->setForeground( LW_HASHFOUND_COLOR_WHITE    );

                    if ( item->background().color() == LW_DCBACKGROUND_COLOR_DARK )  item->setBackground( LW_DCBACKGROUND_COLOR_WHITE );

            }


    }

    ////////////////////////////
    /// Appearance of the ListWidget element
    /// Font size and spacing, Icon

    // Set the font size of the entries
    int iPointSize = CMI.iLWfontSize + 7;
    if ( iPointSize > 16 )  iPointSize = 16;  if ( iPointSize < 7 )  iPointSize = 7;

    QFont xFont = ui->listWidget->font();
    xFont.setPointSize( iPointSize );
    ui->listWidget->setFont( xFont );

    // Set spacing
    int iSpacing = CMI.iLWspacing;
    if( iSpacing < 0 )  iSpacing = 0;  if ( iSpacing > 4 )  iSpacing = 4;
    ui->listWidget->setSpacing( iSpacing );   // Inserts a 1-pixel gap between ALL items

    // Icon size
    if ( CMI.bShowIcons == true )  {

        float fRatio = ICONSIZE / 9.0;
        int iNewSize = qRound( iPointSize * fRatio );
        ui->listWidget->setIconSize( QSize(iNewSize,iNewSize) );

    }

}


// Click on a red entry: copy the tooltip text to the clipboard
void MainWindow::__copyTooltipToClipboard( QListWidgetItem *item )  {

    if ( ui->label_HIstatus1->text() == HISTATUS_CLIPBOARD )  return;

    if ( item && !item->toolTip().isEmpty() )  {

        QGuiApplication::clipboard()->setText( item->toolTip() );


        ///////////////////
        /// Feedback in the status bar at the top right

        // Save the current "actual" text before displaying the message
        // We retrieve it from 'FullText', since that's where the text appears without the "Copied" message
        QString sBackupText = ui->label_HIstatus1->property("FullText").toString();

        // Display the message (setHIstatus1Text handles everything)
        setHIstatus1Text( HISTATUS_CLIPBOARD );

        // Restore the previous state after 1.5 seconds
        QTimer::singleShot( 1500, this, [this, sBackupText]()  {

            // We'll just revert to the old text
            setHIstatus1Text( sBackupText );

        });
    }
}



// All green entries in the ListWidget should be deleted
void MainWindow::__removeGreen( void )  {

    for( int i = ui->listWidget->count() - 1; i >= 0; --i )  {

            QListWidgetItem *xItem = ui->listWidget->item(i);
            if ( xItem->foreground().color() == LW_HASHFOUND_COLOR_DARK || xItem->foreground().color() == LW_HASHFOUND_COLOR_WHITE)  {

                delete ui->listWidget->takeItem( i );

            }
    }

}



// All red entries in the ListWidget should be deleted
void MainWindow::__removeRed( void )  {

    for( int i = ui->listWidget->count() - 1; i >= 0; --i )  {

            QListWidgetItem *xItem = ui->listWidget->item(i);
            if ( xItem->foreground().color() == Qt::red)  delete ui->listWidget->takeItem( i );

    }

}




// ------------------------------------------------ Specifically for Status 1 ------------------------------------------------


// Trim the text on the left until it fits
QString MainWindow::__clipTextLeft( const QString &sFullText, int iMaxWidth )  {

    // Remove HTML for measurement
    QTextDocument doc;
    doc.setHtml( sFullText );
    QString sPlain = doc.toPlainText();

    QFontMetrics fm( ui->label_HIstatus1->font() );

    // If it's already fine, don't do anything
    if ( fm.horizontalAdvance(sPlain) <= iMaxWidth ) return sFullText;

    // We'll strip out the plain text, but we need to preserve the HTML structure (colors, etc.).
    // For simplicity's sake, we'll continue working with the plain text here
    QString prefix = "...";
    while( sPlain.length() > 0 && (fm.horizontalAdvance(prefix + sPlain) > iMaxWidth) )  sPlain.remove( 0, 1 );

    return prefix + sPlain;
}


// Redraw the text in HIstatus1, trimming it on the left if necessary
void MainWindow::__updateLabelDisplay()  {

    // Ganzen Text ggf. merken
    if ( !ui->label_HIstatus1->property("FullText").isValid() )  {

           ui->label_HIstatus1->setProperty( "FullText", ui->label_HIstatus1->text() );

    }

    // Calculate string width
    QString sFullText = ui->label_HIstatus1->property( "FullText" ).toString();
    int iVerfuegbareBreite = ui->label_HIstatus1->width() - 10;     // 10px reserve

    // We challenge the logic behind the cuts
    QString sDisplay = __clipTextLeft( sFullText, iVerfuegbareBreite );

    if ( ui->label_HIstatus1->text() != sDisplay ) ui->label_HIstatus1->setText( sDisplay );

}


// The width of the window changes
void MainWindow::resizeEvent( QResizeEvent *event )  {

  __updateLabelDisplay();
    QMainWindow::resizeEvent( event );

}


// New feature to set the text in HIstatus1
void MainWindow::setHIstatus1Text( const QString &sText )  {

    ui->label_HIstatus1->setProperty( "FullText", sText );
    ui->label_HIstatus1->setProperty( "CachedWidth", QVariant() );    // Clear the cache
  __updateLabelDisplay();

}







// ------------------------------------------------ New features ------------------------------------------------



////////////////////////////////////////////////
///
///   DO THE WORK
///
////////////////////////////////////////////////

// Slot for standard log messages
void MainWindow::__onMessageLogged( const QString sMessage, const QColor xColor, const bool bIcon )  {

    QListWidgetItem *xItem = new QListWidgetItem( sMessage );
    xItem->setForeground( xColor );
    ui->listWidget->addItem( xItem );
    ui->listWidget->scrollToBottom();

    // Set icon
    if ( bIcon == true && CMI.bShowIcons == true )  xItem->setIcon( QIcon(":/images/Info-Disabled.png") );

}

// Slot: File is being edited
void MainWindow::__onFileInProgress( const QString sFileName )  {

    xCurrentItem = new QListWidgetItem( sFileName );
    xCurrentItem->setForeground( __getPreviewColor() );
    ui->listWidget->addItem( xCurrentItem );
    ui->listWidget->scrollToBottom();

}

// Progress indicator during hash calculation
void MainWindow::__onFileHashProgress( const int iPercent )  {

    ui->label_HIstatus2->setText( QString("Computing Hash... %1%").arg(iPercent) );

}

// Slot: Update the counters in the top-right corner
void MainWindow::__onCounterUpdate( const int iSuccessCount, const int iErrorCount, const int iSkippedCount )  {

    setHIstatus1Text( QString("%1 — <span style='color:rgb(0,149,230);'><b>Ok: %2&nbsp;&nbsp;&nbsp;&nbsp;Fail: %3&nbsp;&nbsp;&nbsp;&nbsp;Skip: %4</b></span>")
                              .arg(CMI.sLastOpenDir)
                              .arg(iSuccessCount)
                              .arg(iErrorCount)
                              .arg(iSkippedCount)
                    );

}

// Slot: The hash has been calculated and found in the filename
void MainWindow::__onHashResultReady( const bool bSuccess,
                                      const int iSuccessCount, const int iErrorCount, const int iSkippedCount,
                                      const QString sAbsoluteFilePath,
                                      const QString sHashes
                                    )
{   ui->label_HIstatus2->clear();

    if ( xCurrentItem)  {

        QString sFileName = xCurrentItem->text();

        // Set the color
        xCurrentItem->setForeground( bSuccess ? __getHashFountColor() : Qt::red );

        // If successful, prefix with [OK]; if an error occurs, display the path in the tooltip
        if ( bSuccess )  xCurrentItem->setText( "[OK] " + xCurrentItem->text() );
        else  {  // Optionally, include the path or the path and filename in the tooltip
                 switch( CMI.iOnHashNotFound )
                        {
                        case RADIOBUTTON_0  :
                        case RADIOBUTTON_3  :  break;

                        default             :
                        case RADIOBUTTON_1  :  xCurrentItem->setToolTip( sAbsoluteFilePath );                    break;
                        case RADIOBUTTON_2  :  xCurrentItem->setToolTip( sAbsoluteFilePath + '/' + sFileName );  break;
                        }

                 xCurrentItem->setData( Qt::UserRole,sAbsoluteFilePath );     // Save the absolute path regardless of the tooltip

                 // If "Show calculated hashes" was selected
                 if ( CMI.bShowCompHash == true )  {

                        QStringList lHashList = sHashes.split( ';', Qt::SkipEmptyParts );
                        for( const QString &sSingleHash : lHashList)  {

                                QListWidgetItem *xItem = new QListWidgetItem( QString("Computed %1").arg(sSingleHash) );
                                xItem->setForeground( Qt::red );
                                ui->listWidget->addItem( xItem );

                                QString sHashOnly = sSingleHash.section( ": ", 1 );
                                xItem->setToolTip( sHashOnly );       // Hash as a tooltip

                        }

                }

                // Set icon
                if ( CMI.bShowIcons == true )  xCurrentItem->setIcon( QIcon(":/images/RedArrow.png") );

              }

        // Display status
        setHIstatus1Text( QString("%1 — <span style='color:rgb(0,149,230);'><b>Ok: %2&nbsp;&nbsp;&nbsp;&nbsp;Fail: %3&nbsp;&nbsp;&nbsp;&nbsp;Skip: %4</b></span>")
                                  .arg(CMI.sLastOpenDir)
                                  .arg(iSuccessCount)
                                  .arg(iErrorCount)
                                  .arg(iSkippedCount)
                        );

    }

}

// Slot: The inspection is complete
void MainWindow::__onFinished( QThread *thread, const int iSuccessCount, const int iErrorCount )  {

    // Turn buttons on or off, enable or disable them
    ui->pb_Open->setEnabled( true );
    ui->pb_Clear->setVisible( true );
    ui->pb_Export->setVisible( true );
    ui->pb_Abort->setVisible( false );

    ui->actionSettings->setEnabled( true );

    // Status indicator
    setHIstatus1Text( QString("%1 — <span style='color:rgb(0,149,230);'><b>done</b></span>").arg(CMI.sLastOpenDir) );
    ui->label_HIstatus2->clear();

    // End the thread
    thread->quit();

    // Spacer
    QListWidgetItem *xSpacer = new QListWidgetItem( ui->listWidget );
    xSpacer->setFlags( Qt::NoItemFlags );
    xSpacer->setSizeHint( QSize(0, 7) );

    // Create a container for the buttons
    QListWidgetItem *xButtonItem = new QListWidgetItem( ui->listWidget );
    QWidget *xContainer = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(xContainer);

    // Create the buttons
    QPushButton *xButtonG = new QPushButton( "Remove &green entries" );
    QPushButton *xButtonR = new QPushButton( "Remove &red entries" );

    xButtonG->setFixedWidth( 150 );
    xButtonR->setFixedWidth( 140 );

    // Place both buttons in the SAME layout
    layout->addWidget( xButtonG );
    layout->addWidget( xButtonR );
    layout->addStretch();       // Slide both buttons to the left

    // Optimize spacing
    layout->setContentsMargins( 5, 2, 5, 4 );
    layout->setSpacing( 5 );    // Distance between the two buttons

    // Assign the container to the item
    xContainer->setLayout( layout );
    xButtonItem->setSizeHint( xContainer->sizeHint() );
    ui->listWidget->setItemWidget( xButtonItem, xContainer );

    ui->listWidget->scrollToBottom();

    // Take counters into account
    if ( iSuccessCount == 0 )  xButtonG->setEnabled( false );
    if ( iErrorCount == 0  )   xButtonR->setEnabled( false );

    // Signal-Connections
    connect( xButtonG, &QPushButton::clicked, [this, xButtonG]()  {

      __removeGreen();
        xButtonG->setEnabled( false );

    });

    connect( xButtonR, &QPushButton::clicked, [this, xButtonR]()  {

      __removeRed();
        xButtonR->setEnabled( false );

    });

}

// Primary Job Function
void MainWindow::doTheWork( const QString sPath )  {

    // Turn buttons on or off, enable or disable them
    ui->pb_Open->setEnabled( false );
    ui->pb_Clear->setVisible( false );
    ui->pb_Export->setVisible( false );
    ui->pb_Abort->setVisible( true );

    ui->actionSettings->setEnabled( false );

    // Set space if necessary
    if ( ui->listWidget->count() > 0 )  ui->listWidget->addItem( "" );

    // Prepare
    QThread    *thread = new QThread;
    HashWorker *worker = new HashWorker( sPath, CMI.sAllowedExts );
    worker->moveToThread( thread );

    // Connections
    connect( thread, &QThread::started,     worker, &HashWorker::process );
    connect( worker, &HashWorker::finished, thread, &QThread::quit );
    connect( worker, &HashWorker::finished, worker, &HashWorker::deleteLater );
    connect( thread, &QThread::finished,    thread, &QThread::deleteLater );

    // Communication: Worker -> GUI
    connect( worker, &HashWorker::messageLogged,    this, &MainWindow::__onMessageLogged    );
    connect( worker, &HashWorker::fileInProgress,   this, &MainWindow::__onFileInProgress   );
    connect( worker, &HashWorker::fileHashProgress, this, &MainWindow::__onFileHashProgress );
    connect( worker, &HashWorker::counterUpdate,    this, &MainWindow::__onCounterUpdate    );
    connect( worker, &HashWorker::hashResultReady,  this, &MainWindow::__onHashResultReady  );

    // Connect the Abort button directly to the worker
    connect( ui->pb_Abort, &QPushButton::clicked, worker, &HashWorker::__abort, Qt::DirectConnection );

    // Establish a connection to reactivate the Open pushbutton
    // We connect the finished signal of the THREAD to a slot or lambda
    connect( worker, &HashWorker::finished, this, [this, thread, worker](const int iSuccessCount, const int iErrorCount)  {

        // Disconnect ALL connections from the Abort button so that, on the next startup,
        // there are no "ghost connections" to old workers.
        ui->pb_Abort->disconnect( worker );

      __onFinished( thread, iSuccessCount, iErrorCount );

    });

    // Let's get started
    thread->start();

}



// ---------------------------------------------- Click on items in the toolbar ---------------------------------



// The "About" page is opened
void MainWindow::onInfoClicked() {

    ui->actionInfo->setChecked( true );     // Info-Symbol on

            About winAbout;
            winAbout.setModal( true );
            winAbout.exec();

    ui->actionInfo->setChecked( false );     // Info-Symbol off

}


// Quit is called
void MainWindow::onQuitClicked() {

    QApplication::quit();

}



// Settings (gear icon) is opened
void MainWindow::onSettingsClicked() {

    ui->actionSettings->setChecked( true );     // Gear icon on

            int iThemeOld = CMI.iTheme;

            // Open the Settings dialog
            AppSettings winAppSettings;
            winAppSettings.setModal( true );
            winAppSettings.setInitialTab( iLastSettingsTab );
            winAppSettings.exec();
            iLastSettingsTab = winAppSettings.getLastTab();

            // You may need to reset the theme
            if ( CMI.iTheme != iThemeOld )  __setTheme( CMI.iTheme );

            // Refresh listings
            QShowEvent eventSE;
            QCoreApplication::sendEvent( this, &eventSE );


    ui->actionSettings->setChecked( false );     // Gear icon off

}



// ---------------------------------------------- Click on buttons ---------------------------------------------------




// Open a directory or drive for inspection
void MainWindow::on_pb_Open_clicked()
{
    QString sSelectedDir = "";


    ///////////////////////////
    /// Folder selection 1
    /// Use the original dialog; do not save the size and position

    if ( CMI.bFDsavePosSize == false )
    sSelectedDir = QFileDialog::getExistingDirectory(  this,
                                                       "Select folder or drive",
                                                       CMI.sLastOpenDir,
                                                       QFileDialog::DontResolveSymlinks
                                                    );

    ///////////////////////////
    /// Directory selection 2
    /// Use modified directory selection dialog; size and position are saved

    if ( CMI.bFDsavePosSize == true )  {

        // Create a dialog instance
        QFileDialog xDialog( this, "Select folder or drive", CMI.sLastOpenDir );

        // IMPORTANT: Switch the mode to directories
        xDialog.setFileMode( QFileDialog::Directory );
        xDialog.setOption( QFileDialog::DontResolveSymlinks, true );

        // Important for resizing in Linux Mint
        xDialog.setOption( QFileDialog::DontUseNativeDialog );

        // Restore geometry
        if ( !CMI.xFileDialogGeo.isEmpty() )  xDialog.restoreGeometry( CMI.xFileDialogGeo );
        else  xDialog.resize( 800, 600 );

        if ( xDialog.exec() == QDialog::Accepted )  {

                sSelectedDir = xDialog.selectedFiles().first();     // Retrieve the selected path
                CMI.xFileDialogGeo = xDialog.saveGeometry();       // Save geometry in ConfigManager

        }

    }

    if ( sSelectedDir.isEmpty() ) return;

    // Bookmark this directory
    CMI.sLastOpenDir = QFileInfo( sSelectedDir ).absoluteFilePath();


    ///////////////////////////
    /// Do the work

    if ( CMI.bResetContents == true )  ui->listWidget->clear();
    setHIstatus1Text( QString("%1 — <span style='color:rgb(0,149,230);'><b>Starting</b></span>").arg(CMI.sLastOpenDir) );

    doTheWork( CMI.sLastOpenDir );

}



// "Clear" was clicked
void MainWindow::on_pb_Clear_clicked()
{

    // Check whether the CTRL key is being held down
    if ( QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier) )  __removeGreen();

    else  {     // Otherwise, clear the entire contents of the ListWidget
                ui->listWidget->clear();
                setHIstatus1Text( HISTATUS_DEFAULT );
                ui->label_HIstatus2->clear();

                // Turn buttons on or off, enable or disable them
                ui->pb_Open->setEnabled( true );
                ui->pb_Clear->setVisible( false );
                ui->pb_Export->setVisible( false );
                ui->pb_Abort->setVisible( false );

    }

}



void MainWindow::on_pb_Export_clicked()
{
    QString sSelectedFile = "";

    // Simple dialog (without saving size/position)
    if ( CMI.bFDsavePosSize == false )
    sSelectedFile = QFileDialog::getSaveFileName(  this,
                                                   "Save list content to file",
                                                   CMI.sLastExportDir,
                                                   "Text files (*.txt);;All files (*.*)"
                                                );

    // Custom dialog (saving geometry and position)
    else  {  QFileDialog xDialog( this, "Save list content to file", CMI.sLastExportDir );

             xDialog.setAcceptMode( QFileDialog::AcceptSave );
             xDialog.setFileMode( QFileDialog::AnyFile );
             xDialog.setNameFilters( {"Text files (*.txt)", "All files (*.*)"} );

             // Important for resizing in Linux Mint
             xDialog.setOption( QFileDialog::DontUseNativeDialog );

             if ( !CMI.xFileDialogGeo.isEmpty() )  xDialog.restoreGeometry( CMI.xFileDialogGeo );
             else  xDialog.resize( 800, 600 );

             if ( xDialog.exec() == QDialog::Accepted )  {

                 sSelectedFile = xDialog.selectedFiles().first();
                 CMI.xFileDialogGeo = xDialog.saveGeometry();

             }
    }

    if ( sSelectedFile.isEmpty() ) return;

    // Open the file in write mode (text mode ensures proper line breaks)
    QFile file( sSelectedFile );

    if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )  {

        QTextStream out( &file );
        int         iWrittenLN = 0;

        QLocale     usLocale( QLocale::English, QLocale::UnitedStates );
        QString     xTimestamp = usLocale.toString( QDateTime::currentDateTime(), "MMMM dd, yyyy - HH:mm:ss" );

        // Write the header
        out << "Program: HashInspector https://github.com/complexKA/HashInspector\n";
        out << "Date/Time: " << xTimestamp << "\n";
        out << "------------------------------------------------\n\n";

        // Iterate through all items in the ListWidget
        for( iWrittenLN = 0; iWrittenLN < ui->listWidget->count(); ++iWrittenLN)  {

            QListWidgetItem* xItem = ui->listWidget->item( iWrittenLN );
            if ( xItem ) out << xItem->text() << "\n";        // Type text + line break

        }

        file.close(); // Datei schließen

        QMessageBox::about( 0, "Success", QString("%1 lines written to<br>%2").arg(iWrittenLN + WORKER_HEADER_LINES).arg(sSelectedFile) );

        // Save the last directory for the next time
        CMI.sLastExportDir = QFileInfo( sSelectedFile ).absolutePath();

    }  else  {  // Error handling if the file could not be created
                QMessageBox::critical( this, "Error", "Could not save file: " + file.errorString() );
             }

}



// -------------------------------------------- Click on items in the ListWidget ------------------------------------



// Double-click an item. If it is red, the system will attempt to open the currently selected File Explorer
// to display the corresponding directory
void MainWindow::on_listWidget_itemDoubleClicked( QListWidgetItem *xItem )  {

    if ( !xItem )  return;

    switch( CMI.iOnHashNotFound )  {

            case RADIOBUTTON_0  :
            case RADIOBUTTON_1  :
            case RADIOBUTTON_2  :  break;       // No action

            case RADIOBUTTON_3  :  // Retrieve the (always saved) absolute path
                                   QString sPath = xItem->data( Qt::UserRole ).toString();
                                   if ( sPath.isEmpty() )  return;

                                   // Launch the current file explorer (e.g., Nemo)
                                   QDesktopServices::openUrl( QUrl::fromLocalFile(sPath) );

                                   // Reset all other items
                                   for( int i = 0; i < ui->listWidget->count(); ++i )   ui->listWidget->item(i)->setBackground( Qt::transparent );

                                   // Select the current item
                                   xItem->setBackground( __getDCbackgroundColor() );

                                   break;
    }

}



