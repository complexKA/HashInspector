#include "appsettings.h"
#include "ui_appsettings.h"
#include "configmanager.h"
#include <unistd.h>



AppSettings::AppSettings( QWidget *parent )
    : QDialog( parent )
    , ui( new Ui::AppSettings )  {

    ui->setupUi( this );


    ////////////////////////////////////////////
    /// Preparing for the dialogue

    // Define a group of radio buttons for Instance
    pInstanceRBgrp = new QButtonGroup( this );
    pInstanceRBgrp->addButton( ui->rb_InstanceMulti,  RADIOBUTTON_0 );
    pInstanceRBgrp->addButton( ui->rb_InstanceSingle, RADIOBUTTON_1 );

    // Define a group of radio buttons for Theme
    pThemeRBgrp = new QButtonGroup( this );
    pThemeRBgrp->addButton( ui->rb_WhiteTheme,        RADIOBUTTON_0 );
    pThemeRBgrp->addButton( ui->rb_DarkTheme,         RADIOBUTTON_1 );

    // Font size and spacing combo boxes
    QStringList slFontSizes = { "7", "8", "9", "10", "11", "12", "13", "14" };
    ui->cmbo_LWFontSize->addItems( slFontSizes );

    QStringList slSpacing = { "0", "1", "2", "3", "4" };
    ui->cmbo_LWspacing->addItems( slSpacing );

    // Combobox Extension-Presets
    QStringList slExtsPresets = { "None", "Video", "Audio", "Office", "Archive" };
    ui->cmbo_ExtsPresets->addItems( slExtsPresets );
    connect( ui->cmbo_ExtsPresets, QOverload<int>::of(&QComboBox::activated), this, &AppSettings::onExtPresetSelected );

    // Define a group of radio buttons "Hash Format"
    pHashFormatRBgrp = new QButtonGroup( this );
    pHashFormatRBgrp->addButton( ui->rb_HFignoreCase, RADIOBUTTON_0 );
    pHashFormatRBgrp->addButton( ui->rb_HFuppercase,  RADIOBUTTON_1 );
    pHashFormatRBgrp->addButton( ui->rb_HFlowercase,  RADIOBUTTON_2 );

    // Define a group of radio buttons "Hash Not Found"
    pOnHNFRBgrp = new QButtonGroup( this );
    pOnHNFRBgrp->addButton( ui->rb_HNFnothing,        RADIOBUTTON_0 );
    pOnHNFRBgrp->addButton( ui->rb_HNFpath,           RADIOBUTTON_1 );
    pOnHNFRBgrp->addButton( ui->rb_HNFpathFilename,   RADIOBUTTON_2 );
    pOnHNFRBgrp->addButton( ui->rb_HNFdoubleClick,    RADIOBUTTON_3 );

    // Comboboxen CPU and IO priority
    QStringList slPrioCPU = { "Low", "Normal", "High" };    ui->cmbo_PrioCPU->addItems( slPrioCPU );
    QStringList slPrioIO  = { "Idle", "Normal" };           ui->cmbo_PrioIO->addItems(  slPrioIO  );



    ////////////////////////////////////////////
    /// Restore controls from the backup

    // General section
    int iSavedIndex_Instance = CMI.iInstance;
    if ( auto *pBtn = pInstanceRBgrp->button(iSavedIndex_Instance) )  pBtn->setChecked( true );

    ui->cb_SaveWindowsPS->setChecked(     CMI.bSaveWindowsPS   );
    ui->cb_FDsaveLastDir->setChecked(     CMI.bFDsaveLastDir   );
    ui->cb_FDsavePosSize->setChecked(     CMI.bFDsavePosSize   );

    // Appearance section
    int iSavedIndex_Theme = CMI.iTheme;
    if ( auto *pBtn = pThemeRBgrp->button(iSavedIndex_Theme) )  pBtn->setChecked( true );

    ui->dsb_UIscalingFactor->setValue(    CMI.dUIscalingFactor );
    ui->cb_ShowTooltips->setChecked(      CMI.bShowTooltips    );
    ui->cb_HideLogo->setChecked(          CMI.bHideLogo        );

    ui->cmbo_LWFontSize->setCurrentIndex( CMI.iLWfontSize      );
    ui->cmbo_LWspacing->setCurrentIndex(  CMI.iLWspacing       );

    // Inspection section
    ui->le_AllowedExts->setText(          CMI.sAllowedExts     );   ui->le_AllowedExts->setMaxLength( 255 );

    ui->cb_HashF_MD5->setChecked(         CMI.bHashF_MD5       );
    ui->cb_HashF_SHA1->setChecked(        CMI.bHashF_SHA1      );
    ui->cb_HashF_SHA256->setChecked(      CMI.bHashF_SHA256    );
    ui->cb_HashF_SHA512->setChecked(      CMI.bHashF_SHA512    );
    ui->cb_HashF_SHA3256->setChecked(     CMI.bHashF_SHA3256   );
    ui->cb_HashF_SHA3512->setChecked(     CMI.bHashF_SHA3512   );

    // ------------- Verification of Hash Functions ------------------
    QList<QCheckBox*> hashBoxes = { ui->cb_HashF_MD5,    ui->cb_HashF_SHA1,    ui->cb_HashF_SHA256,
                                    ui->cb_HashF_SHA512, ui->cb_HashF_SHA3256, ui->cb_HashF_SHA3512
                                  };

    // Call validateHashCheckboxes() every time a checkbox is clicked
    for ( QCheckBox* cb : hashBoxes) {

        connect(cb, &QCheckBox::toggled, this, &AppSettings::__validateHashCheckboxes);

    }

  __validateHashCheckboxes();       // Initial call
    // -------------------------------------

    int iSavedIndex_HashFormat = CMI.iHashFormat;
    if ( auto *pBtn = pHashFormatRBgrp->button(iSavedIndex_HashFormat) )  pBtn->setChecked( true );

    int iSavedIndex_OnHNF = CMI.iOnHashNotFound;
    if ( auto *pBtn = pOnHNFRBgrp->button(iSavedIndex_OnHNF) )  pBtn->setChecked( true );

    ui->cb_ShowCompHash->setChecked(      CMI.bShowCompHash    );
    ui->cb_ShowScanPath->setChecked(      CMI.bShowScanPath    );
    ui->cb_IncludeSubf->setChecked(       CMI.bIncludeSubf     );
    ui->cb_ShowIcons->setChecked(         CMI.bShowIcons       );
    ui->cb_ShowSkipped->setChecked(       CMI.bShowSkipped     );
    ui->cb_ShowTimes->setChecked(         CMI.bShowTimes       );

    // Filter section
    ui->pte_ExcludePat->setPlainText(     CMI.sExcludePat      );
    ui->cb_Filter2active->setChecked(     CMI.bFilter2active   );

    // Auto section
    ui->sb_AutoStopMM->setValue(          CMI.iAutoStopMM      );
    ui->sb_AutoStopAI->setValue(          CMI.iAutoStopAI      );
    ui->cb_ResetContents->setChecked(     CMI.bResetContents   );

    // Priority section
    ui->cmbo_PrioCPU->setCurrentIndex(    CMI.iPrioCPU         );
    ui->cmbo_PrioIO->setCurrentIndex(     CMI.iPrioIO          );

    // CPU priority is only available if the program was launched with root privileges
    if ( geteuid() != 0 )  ui->cmbo_PrioCPU->setEnabled( false );
    else  ui->label_RootPrivRequired->hide();


}

AppSettings::~AppSettings()
{
    delete ui;
}



// -------------------------------------- Subfunctions ----------------------------------------------



// Features to restore the last tab when reopening the settings
void AppSettings::setInitialTab( const int iTab ) {  ui->tabWidget->setCurrentIndex( iTab );  }
int AppSettings::getLastTab()                     {  return ui->tabWidget->currentIndex();    }



// This feature ensures that at least one checkbox is selected in the "Hash Functions" section
void AppSettings::__validateHashCheckboxes()  {

    QList<QCheckBox*> hashBoxes = { ui->cb_HashF_MD5,    ui->cb_HashF_SHA1,    ui->cb_HashF_SHA256,
                                    ui->cb_HashF_SHA512, ui->cb_HashF_SHA3256, ui->cb_HashF_SHA3512
                                  };

    QList<QCheckBox*> checkedBoxes;
    for( QCheckBox* cb : hashBoxes )  if ( cb->isChecked() ) checkedBoxes.append( cb );

    bool bDisableLast = ( checkedBoxes.size() == 1 );
    for( QCheckBox* cb : hashBoxes )  {

        if ( bDisableLast && cb->isChecked() )  cb->setEnabled( false );    // Lock the last active checkbox
        else  cb->setEnabled( true );   // Unlock all other checkboxes

    }
}




// -------------------------------------- Response to the push buttons ----------------------------------------------



// Save settings
void AppSettings::on_pb_Save_clicked()
{
    // General section
    CMI.iInstance        = pInstanceRBgrp->checkedId();

    CMI.bSaveWindowsPS   = ui->cb_SaveWindowsPS->isChecked();
    CMI.bFDsaveLastDir   = ui->cb_FDsaveLastDir->isChecked();
    CMI.bFDsavePosSize   = ui->cb_FDsavePosSize->isChecked();

    // Appearance section
    CMI.iTheme           = pThemeRBgrp->checkedId();
    CMI.dUIscalingFactor = ui->dsb_UIscalingFactor->value();

    CMI.bShowTooltips    = ui->cb_ShowTooltips->isChecked();
    CMI.bHideLogo        = ui->cb_HideLogo->isChecked();

    CMI.iLWfontSize      = ui->cmbo_LWFontSize->currentIndex();
    CMI.iLWspacing       = ui->cmbo_LWspacing->currentIndex();

    // Inspection section
    CMI.sAllowedExts     = ui->le_AllowedExts->text().trimmed();

    CMI.bHashF_MD5       = ui->cb_HashF_MD5->isChecked();
    CMI.bHashF_SHA1      = ui->cb_HashF_SHA1->isChecked();
    CMI.bHashF_SHA256    = ui->cb_HashF_SHA256->isChecked();
    CMI.bHashF_SHA512    = ui->cb_HashF_SHA512->isChecked();
    CMI.bHashF_SHA3256   = ui->cb_HashF_SHA3256->isChecked();
    CMI.bHashF_SHA3512   = ui->cb_HashF_SHA3512->isChecked();

    CMI.iHashFormat      = pHashFormatRBgrp->checkedId();
    CMI.iOnHashNotFound  = pOnHNFRBgrp->checkedId();

    CMI.bShowCompHash    = ui->cb_ShowCompHash->isChecked();
    CMI.bShowScanPath    = ui->cb_ShowScanPath->isChecked();
    CMI.bIncludeSubf     = ui->cb_IncludeSubf->isChecked();
    CMI.bShowIcons       = ui->cb_ShowIcons->isChecked();
    CMI.bShowSkipped     = ui->cb_ShowSkipped->isChecked();
    CMI.bShowTimes       = ui->cb_ShowTimes->isChecked();

    // Filter section
    CMI.sExcludePat      = ui->pte_ExcludePat->toPlainText();
    CMI.bFilter2active   = ui->cb_Filter2active->isChecked();

    // Auto section
    CMI.iAutoStopMM      = ui->sb_AutoStopMM->value();
    CMI.iAutoStopAI      = ui->sb_AutoStopAI->value();
    CMI.bResetContents   = ui->cb_ResetContents->isChecked();

    // Priority section
    CMI.iPrioCPU = ui->cmbo_PrioCPU->currentIndex();
    CMI.iPrioIO  = ui->cmbo_PrioIO->currentIndex();

    // Close the window
    close();
}


// Set default values
void AppSettings::on_pb_Default_clicked()
{

    // General section
    pInstanceRBgrp->button(RADIOBUTTON_1)->setChecked( true );

    ui->cb_SaveWindowsPS->setChecked( true );
    ui->cb_FDsaveLastDir->setChecked( true );
    ui->cb_FDsavePosSize->setChecked( true );

    // Appearance section
    pThemeRBgrp->button(RADIOBUTTON_1)->setChecked( true );
    ui->dsb_UIscalingFactor->setValue( 1.0 );

    ui->cb_ShowTooltips->setChecked( true );
    ui->cb_HideLogo->setChecked( false );

    ui->cmbo_LWFontSize->setCurrentIndex( COMBOBOX_4 );
    ui->cmbo_LWspacing->setCurrentIndex( COMBOBOX_1 );

    // Inspection section
    ui->le_AllowedExts->clear();

    ui->cb_HashF_MD5->setChecked( false );
    ui->cb_HashF_SHA1->setChecked( false );
    ui->cb_HashF_SHA256->setChecked( true );
    ui->cb_HashF_SHA512->setChecked( false );
    ui->cb_HashF_SHA3256->setChecked( false );
    ui->cb_HashF_SHA3512->setChecked( false );

    pHashFormatRBgrp->button(RADIOBUTTON_0)->setChecked( true );
    pOnHNFRBgrp->button(RADIOBUTTON_3)->setChecked( true );

    ui->cb_ShowCompHash->setChecked( false );
    ui->cb_ShowScanPath->setChecked( false );
    ui->cb_IncludeSubf->setChecked( true );
    ui->cb_ShowIcons->setChecked( true );
    ui->cb_ShowSkipped->setChecked( false );
    ui->cb_ShowTimes->setChecked( false );

    // Filter section
    ui->pte_ExcludePat->setPlainText( FILTER_DEFAULTPAT );
    ui->cb_Filter2active->setChecked( true );

    // Auto section
    ui->sb_AutoStopMM->setValue( OFF );
    ui->sb_AutoStopAI->setValue( OFF );
    ui->cb_ResetContents->setChecked( false );

    // Priority section
    ui->cmbo_PrioCPU->setCurrentIndex( COMBOBOX_1 );
    ui->cmbo_PrioIO->setCurrentIndex(  COMBOBOX_0 );

}



// -------------------------------------- Respond to other controls ----------------------------------------------



// The user has selected an extension preset in the tab "Inspection 1"
void AppSettings::onExtPresetSelected( const int iIndex )  {

    QString sAllowedExts;

    switch( iIndex)  {  default          :
                        case COMBOBOX_0  :  ui->le_AllowedExts->clear();
                                            ui->le_AllowedExts->setFocus();
                                            return;

                        case COMBOBOX_1  :  sAllowedExts = "mkv;avi;mp4;flv;mov;wmv";    break;
                        case COMBOBOX_2  :  sAllowedExts = "mp3;wav;flac;ogg;m4a;opus";  break;
                        case COMBOBOX_3  :  sAllowedExts = "docx;xlsx;pptx;pdf;odt";     break;
                        case COMBOBOX_4  :  sAllowedExts = "zip:rar;iso;tar;7z;gz;xz";   break;

    }

    ui->le_AllowedExts->setText( sAllowedExts );
    ui->le_AllowedExts->setFocus();

}


// The user clicked the "Clear" button under "Filter"
void AppSettings::on_pb_EPclear_clicked()
{
    ui->pte_ExcludePat->clear();
}


// The user clicked the "Default" button under "Filter"
void AppSettings::on_pb_EPdefault_clicked()
{
    ui->pte_ExcludePat->setPlainText( FILTER_DEFAULTPAT );
}

