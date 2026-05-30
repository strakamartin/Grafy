#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSpinBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QRandomGenerator>
#include <QFont>
#include <QWheelEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <algorithm>
#include <numeric>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mMaticeSousednosti = nullptr;
    mPocetVrcholu = 0;

    mScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(mScene);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->installEventFilter(this);

    connect(ui->spinBoxPocetVrcholu, &QSpinBox::valueChanged, this, &MainWindow::onPocetVrcholu);
    connect(ui->pushButtonPridejHranu, &QPushButton::clicked, this, &MainWindow::onPridejHranu);

    connect(ui->buttonDijkstra, &QPushButton::clicked, this, &MainWindow::onDijkstra);
    connect(ui->buttonKurskalkuv, &QPushButton::clicked, this, &MainWindow::onKruskalkuv);

    connect(ui->buttonGenerujVrcholy, &QPushButton::clicked, this, &MainWindow::onGenerujVrcholy);
    connect(ui->buttonGenerujHrany, &QPushButton::clicked, this, &MainWindow::onGenerujHrany);
    connect(ui->buttonZmazVse, &QPushButton::clicked, this, &MainWindow::onZmazVse);
    connect(ui->pushButtonSmazHranu, &QPushButton::clicked, this, &MainWindow::onSmazHranu);

    connect(ui->buttonImportVrcholu, &QPushButton::clicked, this, &MainWindow::onImportVrcholu);
    connect(ui->buttonImportHrany, &QPushButton::clicked, this, &MainWindow::onImportHrany);
    connect(ui->buttonExportGrafu, &QPushButton::clicked, this, &MainWindow::onExportGrafu);
}

MainWindow::~MainWindow()
{
    zmazMaticiSousednosti();
    qDeleteAll(mHrany);
    qDeleteAll(mVrcholy);
    delete ui;
}

void MainWindow::onPocetVrcholu()
{
    //zmazema matici jeste se starym poctem vrcholu v promenne mPocetVrcholu
    zmazMaticiSousednosti();
    mPocetVrcholu = ui->spinBoxPocetVrcholu->value();
    vytvorMaticiSousednosti();
    vypisMaticeSousednosti();

    //update combo boxu s vrcholami
    ui->comboPrvniVrchol->clear();
    ui->comboDruhyVrchol->clear();
    ui->comboStartVrchol->clear();
    ui->comboCilVrchol->clear();
    for (int i = 0; i < mPocetVrcholu; ++i) {
        ui->comboPrvniVrchol->addItem(QString::number(i));
        ui->comboDruhyVrchol->addItem(QString::number(i));
        ui->comboStartVrchol->addItem(QString::number(i));
        ui->comboCilVrchol->addItem(QString::number(i));
    }
}


void MainWindow::onPridejHranu()
{
    if (mVrcholy.isEmpty() || mMaticeSousednosti == nullptr)
        return;

    int x = ui->comboPrvniVrchol->currentIndex();
    int y = ui->comboDruhyVrchol->currentIndex();

    if (x == y)
        return;

    // Remove any existing edge between the same pair before re-adding
    removeEdge(x, y);

    // Weight = Euclidean distance between the two vertex positions
    double dx = mVrcholy[x]->mX - mVrcholy[y]->mX;
    double dy = mVrcholy[x]->mY - mVrcholy[y]->mY;
    int vaha = qMax(1, static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy))));

    mMaticeSousednosti[x][y] = vaha;
    mMaticeSousednosti[y][x] = vaha;

    mVrcholy[x]->seznamNasledniku.insert(vaha, mVrcholy[y]);
    mVrcholy[y]->seznamNasledniku.insert(vaha, mVrcholy[x]);

    mHrany.append(new Hrana(x, y, vaha));

    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    vykresliGraf();
    vypisMaticeSousednosti();
}

// ─── Smaz hranu ──────────────────────────────────────────────────────────────
void MainWindow::onSmazHranu()
{
    if (mVrcholy.isEmpty() || mMaticeSousednosti == nullptr)
        return;

    int x = ui->comboPrvniVrchol->currentIndex();
    int y = ui->comboDruhyVrchol->currentIndex();

    if (!removeEdge(x, y))
        return;

    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    vykresliGraf();
    vypisMaticeSousednosti();
}

bool MainWindow::removeEdge(int x, int y)
{
    if (x == y || mMaticeSousednosti == nullptr || mMaticeSousednosti[x][y] == 0)
        return false;

    int vaha = mMaticeSousednosti[x][y];

    mMaticeSousednosti[x][y] = 0;
    mMaticeSousednosti[y][x] = 0;

    mVrcholy[x]->seznamNasledniku.remove(vaha, mVrcholy[y]);
    mVrcholy[y]->seznamNasledniku.remove(vaha, mVrcholy[x]);

    int minIdx = std::min(x, y), maxIdx = std::max(x, y);
    for (int i = 0; i < mHrany.size(); ++i) {
        Hrana* h = mHrany[i];
        if (std::min(h->mIdA, h->mIdB) == minIdx &&
            std::max(h->mIdA, h->mIdB) == maxIdx) {
            mHrany.removeAt(i);
            delete h;
            break;
        }
    }

    return true;
}


// ─── Generuj vrcholy ─────────────────────────────────────────────────────────
void MainWindow::onGenerujVrcholy()
{
    // Clear everything
    mScene->clear();
    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    qDeleteAll(mHrany);
    mHrany.clear();
    qDeleteAll(mVrcholy);
    mVrcholy.clear();
    zmazMaticiSousednosti();

    mPocetVrcholu = ui->spinBoxPocetVrcholu->value();
    if (mPocetVrcholu < 1)
        return;

    vytvorMaticiSousednosti();

    // Update combo boxes
    ui->comboPrvniVrchol->clear();
    ui->comboDruhyVrchol->clear();
    ui->comboStartVrchol->clear();
    ui->comboCilVrchol->clear();

    // Determine drawing area (leave a margin of 30 px on each side)
    const int margin = 30;
    const int W = qMax(ui->graphicsView->width()  - 2 * margin, 200);
    const int H = qMax(ui->graphicsView->height() - 2 * margin, 200);

    for (int i = 0; i < mPocetVrcholu; ++i) {
        Vrchol* v = new Vrchol();
        v->mId = i;
        v->mX  = static_cast<int>(QRandomGenerator::global()->bounded(W)) + margin;
        v->mY  = static_cast<int>(QRandomGenerator::global()->bounded(H)) + margin;
        mVrcholy.append(v);

        ui->comboPrvniVrchol->addItem(QString::number(i));
        ui->comboDruhyVrchol->addItem(QString::number(i));
        ui->comboStartVrchol->addItem(QString::number(i));
        ui->comboCilVrchol->addItem(QString::number(i));
    }

    vykresliGraf();
    vypisMaticeSousednosti();
}


void MainWindow::onGenerujHrany()
{
    if (mVrcholy.isEmpty() || mMaticeSousednosti == nullptr || mPocetVrcholu < 2)
        return;

    // Reset adjacency matrix, successor lists and previous Dijkstra state
    for (int i = 0; i < mPocetVrcholu; ++i) {
        for (int j = 0; j < mPocetVrcholu; ++j)
            mMaticeSousednosti[i][j] = 0;
        mVrcholy[i]->seznamNasledniku.clear();
        mVrcholy[i]->mVzdalenostOdStartu  = INT_MAX;
        mVrcholy[i]->mJeVzdalenostSpoctena = false;
        mVrcholy[i]->mIndexPredchudce     = -1;
    }
    qDeleteAll(mHrany);
    mHrany.clear();
    mKostraGrafu.clear();
    mNejkratsiCesta.clear();

    // All possible undirected edges
    QVector<QPair<int,int>> allEdges;
    allEdges.reserve(mPocetVrcholu * (mPocetVrcholu - 1) / 2);
    for (int i = 0; i < mPocetVrcholu; ++i)
        for (int j = i + 1; j < mPocetVrcholu; ++j)
            allEdges.append({i, j});

    // Fisher-Yates shuffle
    for (int i = allEdges.size() - 1; i > 0; --i) {
        int j = static_cast<int>(QRandomGenerator::global()->bounded(static_cast<quint32>(i + 1)));
        std::swap(allEdges[i], allEdges[j]);
    }

    int percentage  = ui->spinHranyPercenta->value();
    int targetEdges = static_cast<int>(allEdges.size() * percentage / 100.0);

    for (int k = 0; k < targetEdges; ++k) {
        int i = allEdges[k].first;
        int j = allEdges[k].second;

        // Weight = rounded Euclidean distance between the two vertices
        double dx = mVrcholy[i]->mX - mVrcholy[j]->mX;
        double dy = mVrcholy[i]->mY - mVrcholy[j]->mY;
        int vaha = qMax(1, static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy))));

        mMaticeSousednosti[i][j] = vaha;
        mMaticeSousednosti[j][i] = vaha;

        mVrcholy[i]->seznamNasledniku.insert(vaha, mVrcholy[j]);
        mVrcholy[j]->seznamNasledniku.insert(vaha, mVrcholy[i]);

        mHrany.append(new Hrana(i, j, vaha));
    }

    vykresliGraf();
    vypisMaticeSousednosti();
}

// ─── Vymaz vše ───────────────────────────────────────────────────────────────
void MainWindow::onZmazVse()
{
    mScene->clear();
    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    qDeleteAll(mHrany);
    mHrany.clear();
    qDeleteAll(mVrcholy);
    mVrcholy.clear();
    zmazMaticiSousednosti();
    mPocetVrcholu = 0;
    mDocastneVrcholy.clear();

    ui->comboPrvniVrchol->clear();
    ui->comboDruhyVrchol->clear();
    ui->comboStartVrchol->clear();
    ui->comboCilVrchol->clear();
    ui->textEditVpravo->clear();
}

// ─── Import vrcholu z Vrcholy.txt ────────────────────────────────────────────
void MainWindow::onImportVrcholu()
{
    QFile file("Vrcholy.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Import vrcholu", "Nelze otevrit soubor Vrcholy.txt");
        return;
    }

    // Parse lines: id x y
    struct VrcholZesouboru { int id, x, y; };
    QVector<VrcholZesouboru> nacitaneVrcholy;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 3)
            continue;
        bool okId, okX, okY;
        int id = parts[0].toInt(&okId);
        int x  = parts[1].toInt(&okX);
        int y  = parts[2].toInt(&okY);
        if (okId && okX && okY)
            nacitaneVrcholy.append({id, x, y});
    }
    file.close();

    if (nacitaneVrcholy.isEmpty())
        return;

    // Sort by ID so that the sequential array index matches usage in Hrany.txt
    std::sort(nacitaneVrcholy.begin(), nacitaneVrcholy.end(),
              [](const VrcholZesouboru& a, const VrcholZesouboru& b){ return a.id < b.id; });

    // Clear current state
    mScene->clear();
    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    qDeleteAll(mHrany);
    mHrany.clear();
    qDeleteAll(mVrcholy);
    mVrcholy.clear();
    zmazMaticiSousednosti();
    mDocastneVrcholy.clear();
    ui->comboPrvniVrchol->clear();
    ui->comboDruhyVrchol->clear();
    ui->comboStartVrchol->clear();
    ui->comboCilVrchol->clear();

    mPocetVrcholu = nacitaneVrcholy.size();
    // Update spinbox without triggering onPocetVrcholu()
    {
        QSignalBlocker blocker(ui->spinBoxPocetVrcholu);
        ui->spinBoxPocetVrcholu->setValue(mPocetVrcholu);
    }
    vytvorMaticiSousednosti();

    for (int i = 0; i < mPocetVrcholu; ++i) {
        Vrchol* v = new Vrchol();
        v->mId = i;
        v->mX  = nacitaneVrcholy[i].x;
        v->mY  = nacitaneVrcholy[i].y;
        mVrcholy.append(v);

        ui->comboPrvniVrchol->addItem(QString::number(i));
        ui->comboDruhyVrchol->addItem(QString::number(i));
        ui->comboStartVrchol->addItem(QString::number(i));
        ui->comboCilVrchol->addItem(QString::number(i));
    }

    vykresliGraf();
    vypisMaticeSousednosti();
}

// ─── Import hrany z Hrany.txt ────────────────────────────────────────────────
void MainWindow::onImportHrany()
{
    if (mVrcholy.isEmpty() || mMaticeSousednosti == nullptr) {
        QMessageBox::warning(this, "Import hrany", "Nejprve importujte nebo vygenerujte vrcholy.");
        return;
    }

    QFile file("Hrany.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Import hrany", "Nelze otevrit soubor Hrany.txt");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;
        bool okA, okB;
        int idA = parts[0].toInt(&okA);
        int idB = parts[1].toInt(&okB);
        if (!okA || !okB)
            continue;
        if (idA < 0 || idA >= mPocetVrcholu || idB < 0 || idB >= mPocetVrcholu)
            continue;
        if (idA == idB)
            continue;

        // Skip if edge already exists
        if (mMaticeSousednosti[idA][idB] != 0)
            continue;

        double dx = mVrcholy[idA]->mX - mVrcholy[idB]->mX;
        double dy = mVrcholy[idA]->mY - mVrcholy[idB]->mY;
        int vaha = qMax(1, static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy))));

        mMaticeSousednosti[idA][idB] = vaha;
        mMaticeSousednosti[idB][idA] = vaha;

        mVrcholy[idA]->seznamNasledniku.insert(vaha, mVrcholy[idB]);
        mVrcholy[idB]->seznamNasledniku.insert(vaha, mVrcholy[idA]);

        mHrany.append(new Hrana(idA, idB, vaha));
    }
    file.close();

    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    vykresliGraf();
    vypisMaticeSousednosti();
}

// ─── Export grafu do Vrcholy.txt a Hrany.txt ─────────────────────────────────
void MainWindow::onExportGrafu()
{
    // Export vertices
    {
        QFile file("Vrcholy.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Export grafu", "Nelze zapsat do souboru Vrcholy.txt");
            return;
        }
        QTextStream out(&file);
        for (int i = 0; i < mPocetVrcholu; ++i)
            out << mVrcholy[i]->mId << " " << mVrcholy[i]->mX << " " << mVrcholy[i]->mY << "\n";
    }

    // Export edges (each undirected edge written once: smaller index first)
    {
        QFile file("Hrany.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Export grafu", "Nelze zapsat do souboru Hrany.txt");
            return;
        }
        QTextStream out(&file);
        for (const Hrana* h : mHrany) {
            int a = std::min(h->mIdA, h->mIdB);
            int b = std::max(h->mIdA, h->mIdB);
            out << a << " " << b << "\n";
        }
    }

    QMessageBox::information(this, "Export grafu", "Graf byl exportovan do Vrcholy.txt a Hrany.txt");
}

// ─── Zoom via Ctrl+scroll ─────────────────────────────────────────────────────
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->graphicsView && event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        if (wheelEvent->modifiers() & Qt::ControlModifier) {
            const double scaleFactor = 1.15;
            if (wheelEvent->angleDelta().y() > 0)
                ui->graphicsView->scale(scaleFactor, scaleFactor);
            else
                ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
            return true; // event consumed
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ─── Matrix helpers ──────────────────────────────────────────────────────────

void MainWindow::vytvorMaticiSousednosti()
{
    // Alokace
    int pocetVrcholu = ui->spinBoxPocetVrcholu->value();
    mMaticeSousednosti = new int*[pocetVrcholu];
    for(int i = 0; i < pocetVrcholu; ++i) {
        mMaticeSousednosti[i] = new int[pocetVrcholu];
        for (int j = 0; j < pocetVrcholu; ++j) {
            mMaticeSousednosti[i][j] = 0;
        }
    }
    // Použití: pole[2][5] = 10;
}
void MainWindow::zmazMaticiSousednosti()
{
    if (mMaticeSousednosti == nullptr)
        return;

    // Deallokace dvorozmerniho pole(uvolnění)
    for(int i = 0; i < mPocetVrcholu; ++i) {
        delete[] mMaticeSousednosti[i]; // Smazat řádky
    }
    delete[] mMaticeSousednosti; // Smazat pole ukazatelů
    mMaticeSousednosti = nullptr;
}

void MainWindow::vypisMaticeSousednosti()
{
    ui->textEditVpravo->clear();
    QString matice;
    QString hrany;
    int pocetVrcholu = ui->spinBoxPocetVrcholu->value();
    for (int i = 0; i < pocetVrcholu; ++i) {
        QString hranyVrcholu;
        for (int j = 0; j < pocetVrcholu; ++j) {
            QString vaha = QString::number(mMaticeSousednosti[i][j]);

            matice += vaha + " ";
            if (mMaticeSousednosti[i][j] > 0)
                hranyVrcholu += QString::number(i) + "-" + QString::number(j) + ": " + vaha + "\n";
        }
        matice += "\n";
        if (!hranyVrcholu.isEmpty())
            hrany += hranyVrcholu + "\n";
    }
    ui->textEditVpravo->setText(hrany);
    ui->textEditVlevo->setText("Matice sousednosti:\n" + matice);
}

// ─── Dijkstra ────────────────────────────────────────────────────────────────

void MainWindow::onDijkstra()
{
    /*
 * DIJKSTRUUV ALGORITMUS - POSTUP:
 * ------------------------------
 * 1. INICIALIZACE
 *    - Vytvor pole 'vzdalenost' (dist) a nastav vse na nekonecno (INT_MAX).
 *    - Vzdalenost pocatecniho uzlu nastav na 0.
 *    - Vloz pocatecni uzel do mapy .
 *
 * 2. HLAVNI CYKLUS (dokud neni mapa prazdna)
 *    - Vyber uzel 'u' s nejmensi vzdalenosti z mapy.
 *    - Pokud je tato vzdalenost vetsi nez uz ulozena, uzel preskoc (optimalizace).
 *
 * 3. RELAXACE HRAN
 *    - Pro kazdeho souseda 'v' uzlu 'u':
 *      - Nova_vzdalenost = vzdalenost[u] + vaha_hrany(u, v)
 *      - POKUD (Nova_vzdalenost < vzdalenost[v]):
 *          - vzdalenost[v] = Nova_vzdalenost
 *          - Nastav 'u' jako predchudce 'v' (pro rekonstrukci cesty)
 *          - Vloz 'v' do mapy
 *
 * 4. KONEC
 *    - Po vycerpani mapy obsahuje pole 'dist' nejkratsi vzdalenosti.
 */
    if (mVrcholy.isEmpty())
        return;

    initDijktra();
    vypocitajVzdalenosti();

    // Reconstruct path from target back to start → fill mNejkratsiCesta
    int cilIndex = ui->comboCilVrchol->currentIndex();

    mNejkratsiCesta.clear();
    mKostraGrafu.clear();
    int cur = cilIndex;
    while (cur != -1) {
        mNejkratsiCesta.prepend(mVrcholy[cur]);
        cur = mVrcholy[cur]->mIndexPredchudce;
    }

    // Show the path in text
    QString cestaText = "Cesta: ";
    for (int k = 0; k < mNejkratsiCesta.size(); ++k) {
        if (k > 0) cestaText += " -> ";
        cestaText += QString::number(mNejkratsiCesta[k]->mId);
    }
    int dist = mVrcholy[cilIndex]->mVzdalenostOdStartu;
    cestaText += "\nVzdalenost: " + (dist == INT_MAX ? QString("nedosazitelny") : QString::number(dist));
    ui->textEditVpravo->setText(ui->textEditVpravo->toPlainText() + "\n" + cestaText);

    vykresliGraf();
}

void MainWindow::onKruskalkuv()
{
    if (mVrcholy.isEmpty() || mHrany.isEmpty())
        return;

    // Union-Find
    std::vector<int> parent(static_cast<std::size_t>(mPocetVrcholu));
    std::vector<int> rankUF(static_cast<std::size_t>(mPocetVrcholu), 0);
    std::iota(parent.begin(), parent.end(), 0);

    auto find = [&](int x) -> int {
        // Iterative path compression
        while (parent[static_cast<std::size_t>(x)] != x) {
            // Path halving
            int next = parent[static_cast<std::size_t>(x)];
            parent[static_cast<std::size_t>(x)] = parent[static_cast<std::size_t>(next)];
            x = next;
        }
        return x;
    };

    auto unite = [&](int x, int y) -> bool {
        int px = find(x), py = find(y);
        if (px == py) return false;
        if (rankUF[static_cast<std::size_t>(px)] < rankUF[static_cast<std::size_t>(py)])
            std::swap(px, py);
        parent[static_cast<std::size_t>(py)] = px;
        if (rankUF[static_cast<std::size_t>(px)] == rankUF[static_cast<std::size_t>(py)])
            ++rankUF[static_cast<std::size_t>(px)];
        return true;
    };

    // Sort edges by weight ascending (Kruskal requires sorted order).
    // Copy so that the original mHrany order is preserved for other operations.
    QList<Hrana*> sortedHrany = mHrany;
    std::sort(sortedHrany.begin(), sortedHrany.end(),
              [](const Hrana* a, const Hrana* b){ return a->mVaha < b->mVaha; });

    // Run Kruskal – fill mKostraGrafu with MST edge endpoint pairs
    mKostraGrafu.clear();
    mNejkratsiCesta.clear();
    for (Hrana* h : sortedHrany) {
        if (unite(h->mIdA, h->mIdB)) {
            mKostraGrafu.append(mVrcholy[h->mIdA]);
            mKostraGrafu.append(mVrcholy[h->mIdB]);
        }
    }

    vykresliGraf();
}

void MainWindow::initDijktra()
{
    int pocetVrcholu = ui->spinBoxPocetVrcholu->value();
    if (pocetVrcholu < 1)
        return;

    mDocastneVrcholy.clear();

    //vytvoreni vrcholu a vlozeni do vektora
    //vrcholy budu mit vzdalenost nekonecno(INT_MAX) na start vrcholu
    for (int i = 0; i < mVrcholy.size(); ++i) {
        mVrcholy[i]->mVzdalenostOdStartu   = INT_MAX;
        mVrcholy[i]->mJeVzdalenostSpoctena = false;
        mVrcholy[i]->mIndexPredchudce      = -1;
    }
    //jako start vrcholu nastavime mu vzdalenost na 0
    int startIndexVrchol = ui->comboStartVrchol->currentIndex();
    mVrcholy[startIndexVrchol]->mVzdalenostOdStartu = 0;
    //do mapy vrcholu na spracovani pridame prvni vrchol
    mDocastneVrcholy.insert({0,startIndexVrchol});  //odtud zacne nas algoritmus prepocitavat vzdalenost k sousedum
}

//hlavna metoda algoritmu
void MainWindow::vypocitajVzdalenosti()
{
    while(!mDocastneVrcholy.empty()) {
        //map.begin() je ukazatel na prvni prvek mapy
        int indexNejblizsihoVrcholu = mDocastneVrcholy.begin()->second;
        if (mVrcholy[indexNejblizsihoVrcholu]->mJeVzdalenostSpoctena) {
            //vrchol je jiz spocten, nepotrebujeme pocitat znovu, jenom ho odstranime z mapy
            mDocastneVrcholy.erase(mDocastneVrcholy.begin());
            continue;
        }
        //spocteme sousedy
        updateSousedu(indexNejblizsihoVrcholu);

        //odstranime jiz vrchol s trvalou vzdalenosti z mapy
        mVrcholy[indexNejblizsihoVrcholu]->mJeVzdalenostSpoctena = true;
        mDocastneVrcholy.erase(mDocastneVrcholy.begin());
    }
}

void MainWindow::updateSousedu(int indexNejblizsihoVrcholu)
{
    //spocteme sousedy
    for (int i = 0; i < mVrcholy.size(); ++i) {//jeden radek matice
        //pokud mame sousedijici vrchol a neni spocteny.
        if (mMaticeSousednosti[indexNejblizsihoVrcholu][i] != 0 &&
            !mVrcholy[i]->mJeVzdalenostSpoctena)
        {
            //soused: index i
            //predsely vrchol: index indexNejmensihoVrcholu
            //spocteme novou potencionalny vzdalenost
            int novaVzdalenost =  mVrcholy[indexNejblizsihoVrcholu]->mVzdalenostOdStartu +
                                 mMaticeSousednosti[indexNejblizsihoVrcholu][i];
            //pokud je mensi, tak udelame update vzdalenosti
            if(novaVzdalenost < mVrcholy[i]->mVzdalenostOdStartu) {
                mVrcholy[i]->mVzdalenostOdStartu = novaVzdalenost;
                mVrcholy[i]->mIndexPredchudce = indexNejblizsihoVrcholu;//udelame tez update predchudce pro pozdejsi vypsani cesty
            }
            //vlozime novy spocteny vrchol(souseda) do mapy
            mDocastneVrcholy.insert({mVrcholy[i]->mVzdalenostOdStartu,i});
        }
    }
}

void MainWindow::vypisVzdalenosti()
{
    QString vypis;
    for (int i = 0; i < mPocetVrcholu; ++i) {
        //vybudovani cesty k startovemu vrcholu pro jeden vrchol
        QString cestaKuStartu = "Cesta ku startu: ";
        int indexCesty = i;
        while(indexCesty != -1) { //-1 ma jiz jenom start
            cestaKuStartu += QString::number(indexCesty);
            indexCesty = mVrcholy[indexCesty]->mIndexPredchudce;
            if (indexCesty != -1)
                cestaKuStartu += "->";
        }

        //vypis jednoho vrcholu
        vypis += "Vrchol "+ QString::number(i) +
                 ": vzdalenost " + QString::number(mVrcholy[i]->mVzdalenostOdStartu) + ", " +
                cestaKuStartu + "\n";
    }
    ui->textEditVpravo->setText(ui->textEditVpravo->toPlainText() + "\n" + vypis);
}

// ─── Scene drawing ───────────────────────────────────────────────────────────

void MainWindow::vykresliGraf()
{
    mScene->clear();

    QFont smallFont;
    smallFont.setPointSize(8);

    const QColor normalColor(180, 180, 180);
    const QPen   normalPen(normalColor, 1);

    // Build lookup sets for highlighted edges (O(1) membership test)
    // mNejkratsiCesta: vertices in path order; consecutive pairs are path edges
    auto edgeKey = [](int a, int b) -> QPair<int,int> {
        return {std::min(a, b), std::max(a, b)};
    };

    QSet<QPair<int,int>> pathEdges;
    for (int i = 0; i + 1 < mNejkratsiCesta.size(); ++i) {
        pathEdges.insert(edgeKey(mNejkratsiCesta[i]->mId, mNejkratsiCesta[i + 1]->mId));
    }

    // mKostraGrafu: flat list of vertex pairs [va0, vb0, va1, vb1, ...]; each pair is one MST edge
    QSet<QPair<int,int>> mstEdges;
    for (int i = 0; i + 1 < mKostraGrafu.size(); i += 2) {
        mstEdges.insert(edgeKey(mKostraGrafu[i]->mId, mKostraGrafu[i + 1]->mId));
    }

    // ── Pass 1: normal edges ──────────────────────────────────────────────────
    for (const Hrana* h : mHrany) {
        QPair<int,int> key = edgeKey(h->mIdA, h->mIdB);
        if (pathEdges.contains(key) || mstEdges.contains(key))
            continue;

        const Vrchol* va = mVrcholy[h->mIdA];
        const Vrchol* vb = mVrcholy[h->mIdB];
        QPointF p1(va->mX, va->mY), p2(vb->mX, vb->mY);

        mScene->addLine(QLineF(p1, p2), normalPen);

        QString vahaPopis = QString::number(h->mIdA) + "-" + QString::number(h->mIdB) + ":" +
                            QString::number(h->mVaha);

        QGraphicsTextItem* wLabel = mScene->addText(vahaPopis);
        wLabel->setFont(smallFont);
        wLabel->setPos((p1 + p2) / 2.0);
        wLabel->setDefaultTextColor(QColor(0,0,0));
    }

    // ── Pass 2: shortest-path edges (green) ──────────────────────────────────
    const QPen pathPen(Qt::green, 3);
    for (const Hrana* h : mHrany) {
        QPair<int,int> key = edgeKey(h->mIdA, h->mIdB);
        if (!pathEdges.contains(key))
            continue;

        const Vrchol* va = mVrcholy[h->mIdA];
        const Vrchol* vb = mVrcholy[h->mIdB];
        QPointF p1(va->mX, va->mY), p2(vb->mX, vb->mY);

        mScene->addLine(QLineF(p1, p2), pathPen);

        QGraphicsTextItem* wLabel = mScene->addText(QString::number(h->mVaha));
        wLabel->setFont(smallFont);
        wLabel->setPos((p1 + p2) / 2.0);
        wLabel->setDefaultTextColor(Qt::green);
    }

    // ── Pass 3: MST edges (blue) ──────────────────────────────────────────────
    const QPen mstPen(Qt::blue, 3);
    for (const Hrana* h : mHrany) {
        QPair<int,int> key = edgeKey(h->mIdA, h->mIdB);
        if (!mstEdges.contains(key))
            continue;

        const Vrchol* va = mVrcholy[h->mIdA];
        const Vrchol* vb = mVrcholy[h->mIdB];
        QPointF p1(va->mX, va->mY), p2(vb->mX, vb->mY);

        mScene->addLine(QLineF(p1, p2), mstPen);

        QGraphicsTextItem* wLabel = mScene->addText(QString::number(h->mVaha));
        wLabel->setFont(smallFont);
        wLabel->setPos((p1 + p2) / 2.0);
        wLabel->setDefaultTextColor(Qt::blue);
    }

    // ── Pass 4: vertices (always on top) ─────────────────────────────────────
    const int r = 10;
    for (const Vrchol* v : mVrcholy) {
        mScene->addEllipse(v->mX - r, v->mY - r, 2 * r, 2 * r,
                           QPen(Qt::black), QBrush(Qt::red));

        QString lbl = QString::number(v->mId) +
                      " [" + QString::number(v->mX) +
                      "," + QString::number(v->mY) + "]";
        QGraphicsTextItem* txt = mScene->addText(lbl);
        txt->setFont(smallFont);
        txt->setPos(v->mX - r, v->mY - 2 * r - 16);
        txt->setDefaultTextColor(Qt::darkBlue);
    }
}

