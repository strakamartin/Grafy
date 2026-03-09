#include "vrchol.h"

#include <QRandomGenerator>

Vrchol::Vrchol()
    : mVzdalenostOdStartu(0), mJeVzdalenostSpoctena(false), mIndexPredchudce(-1)
{
    mX = QRandomGenerator::global()->bounded(600);
    mY = QRandomGenerator::global()->bounded(600);
}
