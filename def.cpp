#include "def.h"
#include "charpoint.h"
#include "controlpoint.h"

bool amid(const CharPoint *current, const CharPoint *prev, const CharPoint *next)
{
    QPainterPath pth(prev->scenePos());
    pth.cubicTo(prev->outControl->scenePos(), next->inControl->scenePos(), next->scenePos());
    pth.closeSubpath();
    QPainterPathStroker ps;
    ps.setWidth(2);
    QPainterPath range = ps.createStroke(pth);
    if(range.contains(current->scenePos()))
        return true;
    else
        return false;
}
