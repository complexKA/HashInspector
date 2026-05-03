#ifndef MITLICENSE_H
#define MITLICENSE_H

#include <QDialog>


namespace Ui {
class MITlicense;
}

class MITlicense : public QDialog
{
    Q_OBJECT

public:
    explicit MITlicense( QWidget *parent = nullptr );
    ~MITlicense();

private:
    Ui::MITlicense *ui;

};

#endif // MITLICENSE_H
