#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <QColor>
#include <QPen>


class MapObject
{
public:
    explicit MapObject();

    MapObject *setId(unsigned int id){
        _id = id;
        return this;
    }

    unsigned int id() const{
        return _id;
    }

    MapObject *setLineColor(const QColor &color){
        _lineColor = color;
        return this;
    }

    QColor lineColor() const{
        return _lineColor;
    }

    MapObject *setFillColor(const QColor &color){
        _fillColor = color;
        return this;
    }

    QColor fillColor() const{
        return _fillColor;
    }

    MapObject *setLineWidth(double width){
        _lineWidth = width;
        return this;
    }

    double lineWidth() const{
        return _lineWidth;
    }

    virtual QPen getPen() const{
        QPen pen(getBrush(), _lineWidth);
        pen.setColor(_lineColor);
        return pen;
    }

    virtual QBrush getBrush() const{
        return QBrush(_fillColor);
    }

    QString getName() const
    {
        return _name;
    }

    MapObject *setName(const QString &name)
    {
        _name = name;
        return this;
    }

    virtual unsigned int reArrangeIds(){return 0;}


protected /*variables*/:
    unsigned int _id;
    QColor _lineColor;
    QColor _fillColor;
    double _lineWidth;
    QString _name;
};

#endif // MAPOBJECT_H
