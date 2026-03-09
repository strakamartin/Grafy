#include "hrana.h"

Hrana::Hrana() : a(nullptr), b(nullptr), mVaha(0), mIndexA(-1), mIndexB(-1) {}

Hrana::Hrana(int indexA, int indexB, int vaha)
    : a(nullptr), b(nullptr), mVaha(vaha), mIndexA(indexA), mIndexB(indexB) {}
