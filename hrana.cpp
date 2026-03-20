#include "hrana.h"

Hrana::Hrana() : a(nullptr), b(nullptr), mVaha(0), mIdA(-1), mIdB(-1) {}

Hrana::Hrana(int idA, int idB, int vaha)
    : a(nullptr), b(nullptr), mVaha(vaha), mIdA(idA), mIdB(idB) {}
