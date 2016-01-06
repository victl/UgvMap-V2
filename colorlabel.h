#ifndef COLORLABEL_H
#define COLORLABEL_H

#include <QLabel>
#include <QMouseEvent>

class ColorLabel : public QLabel
{
    Q_OBJECT
public:
    ColorLabel(QWidget *parent = 0);
//    virtual ~ColorLabel();
protected:
    void mousePressEvent(QMouseEvent *event);
signals:
    void colorChanged(QColor);

public slots:
};

#endif // COLORLABEL_H
