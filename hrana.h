#ifndef HRANA_H
#define HRANA_H

class Vrchol;

struct Hrana
{
    Hrana();
    Hrana(int indexA, int indexB, int vaha);

    Vrchol* a;
    Vrchol* b;

    int mVaha;
    int mIndexA;
    int mIndexB;
};

#endif // HRANA_H
