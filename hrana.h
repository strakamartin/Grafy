#ifndef HRANA_H
#define HRANA_H

class Vrchol;

struct Hrana
{
    Hrana();
    Hrana(int idA, int idB, int vaha);

    Vrchol* a;
    Vrchol* b;

    int mVaha;
    int mIdA;
    int mIdB;
};

#endif // HRANA_H
