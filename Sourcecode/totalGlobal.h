#ifndef TOTALGLOBAL_H
#define TOTALGLOBAL_H

#include <QString>
#include <QColor>


static constexpr int OFF                        = 0;

static constexpr int RADIOBUTTON_0              = 0;
static constexpr int RADIOBUTTON_1              = 1;
static constexpr int RADIOBUTTON_2              = 2;
static constexpr int RADIOBUTTON_3              = 3;

static constexpr int COMBOBOX_0                 = 0;
static constexpr int COMBOBOX_1                 = 1;
static constexpr int COMBOBOX_2                 = 2;
static constexpr int COMBOBOX_3                 = 3;
static constexpr int COMBOBOX_4                 = 4;
static constexpr int COMBOBOX_5                 = 5;

static const QString TEXTBROWSER_CSS            = "QTextBrowser { font: 9pt 'Sans Serif'; background-color: transparent; border: none; margin-left: -3px; }";

static const QColor LW_SYSTXT_COLOR_DARK        = QColor(140,140,140);
static const QColor LW_SYSTXT_COLOR_WHITE       = QColor(40,40,40);

static const QColor LW_PREVIEW_COLOR_DARK       = QColor(80,80,80);
static const QColor LW_PREVIEW_COLOR_WHITE      = QColor(160,160,160);

static const QColor LW_HASHFOUND_COLOR_DARK     = QColor(0,255,0);
static const QColor LW_HASHFOUND_COLOR_WHITE    = QColor(0,128,0);

static const QColor LW_DCBACKGROUND_COLOR_DARK  = QColor(40,40,40);
static const QColor LW_DCBACKGROUND_COLOR_WHITE = QColor(220,220,220);

static constexpr int MAINWIN_MINW = 600;    static constexpr int MAINWIN_MAXW = 16777215;
static constexpr int MAINWIN_MINH = 400;    static constexpr int MAINWIN_MAXH = 16777215;

static const QString HISTATUS_DEFAULT           = "Please click <span style='color:rgb(0,149,230);'><b>'Open'</b></span> to inspect a folder or drive";
static const QString HISTATUS_CLIPBOARD         = "<span style='color:rgb(0,149,230);'><b>Copied to clipboard</b></span>";

static int ICONSIZE                             = 15;
static int WORKER_HEADER_LINES                  = 5;

static const QString FILTER_DEFAULTPAT =        "*/.trash-*/\n"
                                                "desktop.ini\n"
                                                "system volume information/\n"
                                                "$recycle.bin/\n"
                                                "recycle?/\n"
                                                "recovery/\n"
                                                "*/thumbs.db\n"
                                                "#recycle/\n"
                                                "*/lost+found/\n"
                                                ".ds_store\n"
                                                "*/.cache/\n"
                                                "$winre_backup_partition.marker";




#define CMI ConfigManager::instance()


extern QString __getSettingsPath();

extern QColor __getSysTXTcolor();
extern QColor __getPreviewColor();
extern QColor __getHashFoundColor();
extern QColor __getDCbackgroundColor();


#endif
