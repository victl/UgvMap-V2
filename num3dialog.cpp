#include "num3dialog.h"
#include "ui_num3dialog.h"

Num3Dialog::Num3Dialog(const QString& lbl1, const QString& lbl2, const QString& lbl3, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Num3Dialog)
{
    ui->setupUi(this);
    ui->label1->setText(lbl1);
    ui->label2->setText(lbl2);
    ui->label3->setText(lbl3);
}

Num3Dialog::~Num3Dialog()
{
    delete ui;
}
