#include "mitlicense.h"
#include "ui_mitlicense.h"
#include "totalGlobal.h"


MITlicense::MITlicense( QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::MITlicense )
{
    ui->setupUi( this );


    // Format text
    ui->textbrowser_mit->setStyleSheet( TEXTBROWSER_CSS );

    // Sets the fixed flags: window type, title, system menu, and only the close button
    // Locks the size to the current values (also disables dragging by the corners)
    this->setWindowFlags( Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
    this->setFixedSize( this->width(), this->height() );
}

MITlicense::~MITlicense()
{
    delete ui;
}
