#ifndef NUM3DIALOG_H
#define NUM3DIALOG_H

#include <QDialog>

namespace Ui {
class Num3Dialog;
}

class Num3Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Num3Dialog(const QString& lbl1, const QString& lbl2, const QString& lbl3, QWidget *parent = 0);
    ~Num3Dialog();

private:
    Ui::Num3Dialog *ui;
};

#endif // NUM3DIALOG_H
