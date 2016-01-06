#include "colorlabel.h"
#include <QColorDialog>

ColorLabel::ColorLabel(QWidget *parent)
    : QLabel(parent)
{
}

//ColorLabel::~ColorLabel()
//{

//}

void ColorLabel::mousePressEvent(QMouseEvent */*event*/)
{
    QColor c = QColorDialog::getColor(palette().color(QPalette::Background));
    if(c.isValid()){
        emit colorChanged(c);
        setStyleSheet(tr("background-color: rgb(%1, %2, %3);").arg(c.red()).arg(c.green()).arg(c.blue()));
    }
}

