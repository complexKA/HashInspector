#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QButtonGroup>
#include <QDialog>


namespace Ui {
class AppSettings;
}

class AppSettings : public QDialog
{
    Q_OBJECT

public:
    explicit AppSettings( QWidget *parent = nullptr );
    ~AppSettings();

    void setInitialTab( int );
    int  getLastTab();

private slots:
    void on_pb_Save_clicked();
    void on_pb_Default_clicked();

    void on_pb_EPclear_clicked();
    void on_pb_EPdefault_clicked();

    void __validateHashCheckboxes();

private:
    Ui::AppSettings *ui;

    QButtonGroup *pInstanceRBgrp;
    QButtonGroup *pThemeRBgrp;
    QButtonGroup *pHashFormatRBgrp;
    QButtonGroup *pOnHNFRBgrp;

    void onExtPresetSelected( int );

};

#endif // APPSETTINGS_H
