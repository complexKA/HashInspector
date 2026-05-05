#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "totalGlobal.h"

#include <QMainWindow>
#include <QEvent>
#include <qevent.h>
#include <QListWidgetItem>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = nullptr );
    ~MainWindow();

    void resetMainWindowPosSize();

protected:
    void contextMenuEvent( QContextMenuEvent *xEvent ) override {
        // Right-clicking on the toolbar allows you to disable the tools.
        // We simply ignore the event -> no menu appears
        xEvent->ignore();
    }


    void closeEvent( QCloseEvent * ) override;
    bool eventFilter( QObject *, QEvent * ) override;
    void showEvent( QShowEvent * ) override;

    void onInfoClicked();
    void onQuitClicked();
    void onSettingsClicked();

private slots:
    void on_pb_Open_clicked();
    void on_pb_Clear_clicked();
    void on_pb_Export_clicked();

    void on_listWidget_itemDoubleClicked( QListWidgetItem * );

private:
    Ui::MainWindow *ui;

    int              iLastSettingsTab = 0;
    QListWidgetItem *xCurrentItem     = nullptr;

    void    __setGeomentry();
    void    __setTheme( int );
    void    __setCurrentPriority();
    void    __setLWandEntryStyle();
    void    __copyTooltipToClipboard( QListWidgetItem * );
    void    __removeGreen();
    void    __removeRed();

    QString __clipTextLeft( const QString &, int );
    void    __updateLabelDisplay();
    void    resizeEvent( QResizeEvent * ) override;
    void    setHIstatus1Text( const QString &);

    void    __onMessageLogged( const QString, const QColor, const bool bIcon = false );
    void    __onFileInProgress( const QString );
    void    __onFileHashProgress( const int );
    void    __onCounterUpdate( const int, const int, const int );
    void    __onHashResultReady( const bool, const int, const int, const int, const QString, const QString );
    void    __onFinished( QThread *, const int, const int );
    void    doTheWork( const QString );

};
#endif // MAINWINDOW_H
