#include "about.h"
#include "ui_about.h"
#include <QMessageBox>
#include <QTextFrame>

#include "mitlicense.h"
#include "totalGlobal.h"


About::About( QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::About )  {

    ui->setupUi( this );


    // Insert logo from the resource
    QPixmap picLogo( ":/images/HashInspector-Logo.png" );
    ui->label_logo->setPixmap( picLogo.scaled(45,45,Qt::KeepAspectRatio, Qt::SmoothTransformation) );

    // Leave a little space to the right of the scrollbar (tip)
    QTextFrameFormat xFormat = ui->textbrowser_about1->document()->rootFrame()->frameFormat();
    xFormat.setRightMargin( 15 );
    ui->textbrowser_about1->document()->rootFrame()->setFrameFormat( xFormat );

    // Format the "About" text in all tabs
    ui->textbrowser_about1->setStyleSheet( TEXTBROWSER_CSS );
    ui->textbrowser_about2->setStyleSheet( TEXTBROWSER_CSS );

    // Make sure the first tab opens first
    ui->tabWidget->setCurrentIndex( 0 );

    // Links should open in a new browser window
    ui->textbrowser_about1->setOpenExternalLinks( true );
    ui->textbrowser_about2->setOpenExternalLinks( true );

    // Sets the fixed flags: window type, title, system menu, and only the close button
    // Locks the size to the current values (also disables dragging by the corners)
    this->setWindowFlags( Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
    this->setFixedSize( this->width(), this->height() );

}

About::~About()
{
    delete ui;
}


void About::on_pb_aboutQt_clicked()
{
    QMessageBox::aboutQt( this, "Über Qt" );
}


void About::on_pushButton_clicked()
{
    MITlicense winMITlicence( this );
    winMITlicence.setModal( true );
    winMITlicence.exec();
}

