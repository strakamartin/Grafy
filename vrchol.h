#ifndef VRCHOL_H
#define VRCHOL_H

#include <QMap>

struct Vrchol
{
    Vrchol();
    QMultiMap<int, Vrchol*> seznamNasledniku;  //vzdalenost k vrcholu a vrchol

    int mVzdalenostOdStartu;
    bool mJeVzdalenostSpoctena;

    int mIndexPredchudce;
    int mX;
    int mY;
};

#endif // VRCHOL_H
