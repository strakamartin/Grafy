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
    if (mVrcholy.empty() || mMaticeSousednosti == nullptr)
        return;

    int x = ui->comboPrvniVrchol->currentIndex();
    int y = ui->comboDruhyVrchol->currentIndex();

    if (x == y)
        return;

    // Remove any existing edge between the same pair before re-adding
    removeEdge(x, y);

    // Weight = Euclidean distance between the two vertex positions
    double dx = mVrcholy[x].mX - mVrcholy[y].mX;
    double dy = mVrcholy[x].mY - mVrcholy[y].mY;
    int vaha = qMax(1, static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy))));

    mMaticeSousednosti[x][y] = vaha;
    mMaticeSousednosti[y][x] = vaha;

    mVrcholy[x].seznamNasledniku.insert(vaha, &mVrcholy[y]);
    mVrcholy[y].seznamNasledniku.insert(vaha, &mVrcholy[x]);

    mHrany.insert(vaha, Hrana(x, y, vaha));

    kresliScene();
    vypisMaticeSousednosti();
}

// ─── Smaz hranu ──────────────────────────────────────────────────────────────
void MainWindow::onSmazHranu()
{
    if (mVrcholy.empty() || mMaticeSousednosti == nullptr)
        return;

    int x = ui->comboPrvniVrchol->currentIndex();
    int y = ui->comboDruhyVrchol->currentIndex();

    if (!removeEdge(x, y))
        return;

    kresliScene();
    vypisMaticeSousednosti();
}

bool MainWindow::removeEdge(int x, int y)
{
    if (x == y || mMaticeSousednosti == nullptr || mMaticeSousednosti[x][y] == 0)
        return false;

    int vaha = mMaticeSousednosti[x][y];

    mMaticeSousednosti[x][y] = 0;
    mMaticeSousednosti[y][x] = 0;

    mVrcholy[x].seznamNasledniku.remove(vaha, &mVrcholy[y]);
    mVrcholy[y].seznamNasledniku.remove(vaha, &mVrcholy[x]);

    int minIdx = std::min(x, y), maxIdx = std::max(x, y);
    for (auto it = mHrany.begin(); it != mHrany.end(); ++it) {
        const Hrana& h = it.value();
        if (std::min(h.mIndexA, h.mIndexB) == minIdx &&
            std::max(h.mIndexA, h.mIndexB) == maxIdx) {
            mHrany.erase(it);
            break;
        }
    }

    return true;
}

// void MainWindow::onImportGraf()
// {
//     int pocetVrcholu = 4;
//     ui->spinBoxPocetVrcholu->setValue(pocetVrcholu);

//     //init matice
//     mMaticeSousednosti[0][1] = 6;
//     mMaticeSousednosti[1][0] = 6;
//     mMaticeSousednosti[0][2] = 1;
//     mMaticeSousednosti[2][0] = 1;
//     mMaticeSousednosti[0][3] = 5;
//     mMaticeSousednosti[3][0] = 5;
//     mMaticeSousednosti[1][2] = 7;
//     mMaticeSousednosti[2][1] = 7;
//     mMaticeSousednosti[3][2] = 2;
//     mMaticeSousednosti[2][3] = 2;
//     vypisMaticeSousednosti();

//     qDebug() << "dddddddddddddddddddddddddd";

//     //init seznam vrcholu
//     for (int i = 0; i < pocetVrcholu; ++i) {
//         Vrchol tmp;
//         mVrcholy.push_back(tmp);  //pridani na konec
//         qDebug() << tmp.mX << " " << tmp.mY;
//     }

//     //pridani doseznamNaslednikuiku, setriduje podle vahy hrany
//     mVrcholy[0].seznamNasledniku.insert(1, &mVrcholy[2]);
//     mVrcholy[0].seznamNasledniku.insert(5, &mVrcholy[3]);
//     mVrcholy[0].seznamNasledniku.insert(6, &mVrcholy[1]);

//     mVrcholy[1].seznamNasledniku.insert(6, &mVrcholy[0]);
//     mVrcholy[1].seznamNasledniku.insert(7, &mVrcholy[2]);

//     mVrcholy[2].seznamNasledniku.insert(1, &mVrcholy[0]);
//     mVrcholy[2].seznamNasledniku.insert(2, &mVrcholy[3]);
//     mVrcholy[2].seznamNasledniku.insert(7 ,&mVrcholy[1]);

//     mVrcholy[3].seznamNasledniku.insert(2, &mVrcholy[2]);
//     mVrcholy[3].seznamNasledniku.insert(5, &mVrcholy[0]);
// }

// ─── Generuj vrcholy ─────────────────────────────────────────────────────────
void MainWindow::onGenerujVrcholy()
{
    // Clear everything
    mScene->clear();
    mVrcholy.clear();
    mHrany.clear();
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

    // Reserve so that pointer addresses remain stable for seznamNasledniku
    mVrcholy.reserve(mPocetVrcholu);

    for (int i = 0; i < mPocetVrcholu; ++i) {
        Vrchol v;
        v.mX = static_cast<int>(QRandomGenerator::global()->bounded(W)) + margin;
        v.mY = static_cast<int>(QRandomGenerator::global()->bounded(H)) + margin;
        mVrcholy.push_back(v);

        ui->comboPrvniVrchol->addItem(QString::number(i));
        ui->comboDruhyVrchol->addItem(QString::number(i));
        ui->comboStartVrchol->addItem(QString::number(i));
        ui->comboCilVrchol->addItem(QString::number(i));
    }

    kresliScene();
    vypisMaticeSousednosti();
}

// ─── Generuj hrany ───────────────────────────────────────────────────────────
void MainWindow::onGenerujHrany()
{
    if (mVrcholy.empty() || mMaticeSousednosti == nullptr || mPocetVrcholu < 2)
        return;

    // Reset adjacency matrix, successor lists and previous Dijkstra state
    for (int i = 0; i < mPocetVrcholu; ++i) {
        for (int j = 0; j < mPocetVrcholu; ++j)
            mMaticeSousednosti[i][j] = 0;
        mVrcholy[i].seznamNasledniku.clear();
        mVrcholy[i].mVzdalenostOdStartu  = INT_MAX;
        mVrcholy[i].mJeVzdalenostSpoctena = false;
        mVrcholy[i].mIndexPredchudce     = -1;
    }
    mHrany.clear();

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
        double dx = mVrcholy[i].mX - mVrcholy[j].mX;
        double dy = mVrcholy[i].mY - mVrcholy[j].mY;
        int vaha = qMax(1, static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy))));

        mMaticeSousednosti[i][j] = vaha;
        mMaticeSousednosti[j][i] = vaha;

        // Update successor lists (pointers are stable because we reserved)
        mVrcholy[i].seznamNasledniku.insert(vaha, &mVrcholy[j]);
        mVrcholy[j].seznamNasledniku.insert(vaha, &mVrcholy[i]);

        mHrany.insert(vaha, Hrana(i, j, vaha));
    }

    kresliScene();
    vypisMaticeSousednosti();
}

// ─── Vymaz vše ───────────────────────────────────────────────────────────────
void MainWindow::onZmazVse()
{
    mScene->clear();
    mVrcholy.clear();
    mHrany.clear();
    zmazMaticiSousednosti();
    mPocetVrcholu = 0;
    mDocastneVrcholy.clear();

    ui->comboPrvniVrchol->clear();
    ui->comboDruhyVrchol->clear();
    ui->comboStartVrchol->clear();
    ui->comboCilVrchol->clear();
    ui->textEditMatice->clear();
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
    mVrcholy.clear();
    mHrany.clear();
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
    mVrcholy.reserve(mPocetVrcholu);
    vytvorMaticiSousednosti();

    for (int i = 0; i < mPocetVrcholu; ++i) {
        Vrchol v;
        v.mX = nacitaneVrcholy[i].x;
        v.mY = nacitaneVrcholy[i].y;
        mVrcholy.push_back(v);

        ui->comboPrvniVrchol->addItem(QString::number(i));
        ui->comboDruhyVrchol->addItem(QString::number(i));
        ui->comboStartVrchol->addItem(QString::number(i));
        ui->comboCilVrchol->addItem(QString::number(i));
    }

    kresliScene();
    vypisMaticeSousednosti();
}

// ─── Import hrany z Hrany.txt ────────────────────────────────────────────────
void MainWindow::onImportHrany()
{
    if (mVrcholy.empty() || mMaticeSousednosti == nullptr) {
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

        double dx = mVrcholy[idA].mX - mVrcholy[idB].mX;
        double dy = mVrcholy[idA].mY - mVrcholy[idB].mY;
        int vaha = qMax(1, static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy))));

        mMaticeSousednosti[idA][idB] = vaha;
        mMaticeSousednosti[idB][idA] = vaha;

        mVrcholy[idA].seznamNasledniku.insert(vaha, &mVrcholy[idB]);
        mVrcholy[idB].seznamNasledniku.insert(vaha, &mVrcholy[idA]);

        mHrany.insert(vaha, Hrana(idA, idB, vaha));
    }
    file.close();

    kresliScene();
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
            out << i << " " << mVrcholy[i].mX << " " << mVrcholy[i].mY << "\n";
    }

    // Export edges (each undirected edge written once: smaller index first)
    {
        QFile file("Hrany.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Export grafu", "Nelze zapsat do souboru Hrany.txt");
            return;
        }
        QTextStream out(&file);
        for (auto it = mHrany.cbegin(); it != mHrany.cend(); ++it) {
            const Hrana& h = it.value();
            int a = std::min(h.mIndexA, h.mIndexB);
            int b = std::max(h.mIndexA, h.mIndexB);
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
    ui->textEditMatice->clear();
    QString matice;
    int pocetVrcholu = ui->spinBoxPocetVrcholu->value();
    for (int i = 0; i < pocetVrcholu; ++i) {
        for (int j = 0; j < pocetVrcholu; ++j) {
            matice += QString::number(mMaticeSousednosti[i][j]) + " ";
        }
        matice += "\n";
    }
    ui->textEditMatice->setText(matice);
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
    if (mVrcholy.empty())
        return;

    initDijktra();
    vypocitajVzdalenosti();

    // Reconstruct path from target back to start
    int cilIndex = ui->comboCilVrchol->currentIndex();

    std::set<std::pair<int,int>> pathEdges;
    int cur = cilIndex;
    while (cur != -1 && mVrcholy[cur].mIndexPredchudce != -1) {
        int pred = mVrcholy[cur].mIndexPredchudce;
        pathEdges.insert({std::min(cur, pred), std::max(cur, pred)});
        cur = pred;
    }

    // Show the path in text
    QString cestaText = "Cesta: ";
    QVector<int> pathNodes;
    cur = cilIndex;
    while (cur != -1) {
        pathNodes.prepend(cur);
        cur = mVrcholy[cur].mIndexPredchudce;
    }
    for (int k = 0; k < pathNodes.size(); ++k) {
        if (k > 0) cestaText += " -> ";
        cestaText += QString::number(pathNodes[k]);
    }
    int dist = mVrcholy[cilIndex].mVzdalenostOdStartu;
    cestaText += "\nVzdalenost: " + (dist == INT_MAX ? QString("nedosazitelny") : QString::number(dist));
    ui->textEditMatice->setText(ui->textEditMatice->toPlainText() + "\n" + cestaText);

    kresliScene(pathEdges, Qt::green);
}

void MainWindow::onKruskalkuv()
{
    if (mVrcholy.empty() || mHrany.empty())
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

    // mHrany is a QMultiMap sorted by weight (ascending) – perfect for Kruskal
    std::set<std::pair<int,int>> mstEdges;
    for (auto it = mHrany.cbegin(); it != mHrany.cend(); ++it) {
        const Hrana& h = it.value();
        if (unite(h.mIndexA, h.mIndexB)) {
            mstEdges.insert({std::min(h.mIndexA, h.mIndexB),
                              std::max(h.mIndexA, h.mIndexB)});
        }
    }

    kresliScene(mstEdges, Qt::blue);
}

void MainWindow::initDijktra()
{
    int pocetVrcholu = ui->spinBoxPocetVrcholu->value();
    if (pocetVrcholu < 1)
        return;

    mDocastneVrcholy.clear();

    //vytvoreni vrcholu a vlozeni do vektora
    //vrcholy budu mit vzdalenost nekonecno(INT_MAX) na start vrcholu
    for (std::size_t i = 0; i < mVrcholy.size(); ++i) {
        mVrcholy[i].mVzdalenostOdStartu   = INT_MAX;
        mVrcholy[i].mJeVzdalenostSpoctena = false;
        mVrcholy[i].mIndexPredchudce      = -1;
    }
    //jako start vrcholu nastavime mu vzdalenost na 0
    int startIndexVrchol = ui->comboStartVrchol->currentIndex();
    mVrcholy[startIndexVrchol].mVzdalenostOdStartu = 0;
    //do mapy vrcholu na spracovani pridame prvni vrchol
    mDocastneVrcholy.insert({0,startIndexVrchol});  //odtud zacne nas algoritmus prepocitavat vzdalenost k sousedum
}

//hlavna metoda algoritmu
void MainWindow::vypocitajVzdalenosti()
{
    while(!mDocastneVrcholy.empty()) {
        //map.begin() je ukazatel na prvni prvek mapy
        int indexNejblizsihoVrcholu = mDocastneVrcholy.begin()->second;
        if (mVrcholy[indexNejblizsihoVrcholu].mJeVzdalenostSpoctena) {
            //vrchol je jiz spocten, nepotrebujeme pocitat znovu, jenom ho odstranime z mapy
            mDocastneVrcholy.erase(mDocastneVrcholy.begin());
            continue;
        }
        //spocteme sousedy
        updateSousedu(indexNejblizsihoVrcholu);

        //odstranime jiz vrchol s trvalou vzdalenosti z mapy
        mVrcholy[indexNejblizsihoVrcholu].mJeVzdalenostSpoctena = true;
        mDocastneVrcholy.erase(mDocastneVrcholy.begin());
    }
}

void MainWindow::updateSousedu(int indexNejblizsihoVrcholu)
{
    //spocteme sousedy
    for (std::size_t i = 0; i < mVrcholy.size(); ++i) {//jeden radek matice
        //pokud mame sousedijici vrchol a neni spocteny.
        if (mMaticeSousednosti[indexNejblizsihoVrcholu][i] != 0 &&
            !mVrcholy[i].mJeVzdalenostSpoctena)
        {
            //soused: index i
            //predsely vrchol: index indexNejmensihoVrcholu
            //spocteme novou potencionalny vzdalenost
            int novaVzdalenost =  mVrcholy[indexNejblizsihoVrcholu].mVzdalenostOdStartu +
                                 mMaticeSousednosti[indexNejblizsihoVrcholu][i];
            //pokud je mensi, tak udelame update vzdalenosti
            if(novaVzdalenost < mVrcholy[i].mVzdalenostOdStartu) {
                mVrcholy[i].mVzdalenostOdStartu = novaVzdalenost;
                mVrcholy[i].mIndexPredchudce = indexNejblizsihoVrcholu;//udelame tez update predchudce pro pozdejsi vypsani cesty
            }
            //vlozime novy spocteny vrchol(souseda) do mapy
            mDocastneVrcholy.insert({mVrcholy[i].mVzdalenostOdStartu,i});
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
            indexCesty = mVrcholy[indexCesty].mIndexPredchudce;
            if (indexCesty != -1)
                cestaKuStartu += "->";
        }

        //vypis jednoho vrcholu
        vypis += "Vrchol "+ QString::number(i) +
                 ": vzdalenost " + QString::number(mVrcholy[i].mVzdalenostOdStartu) + ", " +
                cestaKuStartu + "\n";
    }
    ui->textEditMatice->setText(ui->textEditMatice->toPlainText() + "\n" + vypis);
}

// ─── Scene drawing ───────────────────────────────────────────────────────────

void MainWindow::kresliScene(const std::set<std::pair<int,int>>& zvyrazneneHrany,
                              QColor zvyraznenaBarva)
{
    mScene->clear();

    QFont smallFont;
    smallFont.setPointSize(8);

    const QColor normalColor(180, 180, 180);   // light gray — less prominent
    const QPen   normalPen(normalColor, 1);

    // ── Pass 1: non-highlighted edges (drawn first → lowest z-order) ──────────
    for (auto it = mHrany.cbegin(); it != mHrany.cend(); ++it) {
        const Hrana& h = it.value();
        std::pair<int,int> key{std::min(h.mIndexA, h.mIndexB),
                               std::max(h.mIndexA, h.mIndexB)};
        if (zvyrazneneHrany.count(key) > 0)
            continue;  // drawn in pass 2

        const Vrchol& va = mVrcholy[static_cast<std::size_t>(h.mIndexA)];
        const Vrchol& vb = mVrcholy[static_cast<std::size_t>(h.mIndexB)];
        QPointF p1(va.mX, va.mY), p2(vb.mX, vb.mY);

        mScene->addLine(QLineF(p1, p2), normalPen);

        QGraphicsTextItem* wLabel = mScene->addText(QString::number(h.mVaha));
        wLabel->setFont(smallFont);
        wLabel->setPos((p1 + p2) / 2.0);
        wLabel->setDefaultTextColor(normalColor);
    }

    // ── Pass 2: highlighted edges (drawn on top of normal edges) ─────────────
    QPen highlightPen(zvyraznenaBarva, 3);
    for (auto it = mHrany.cbegin(); it != mHrany.cend(); ++it) {
        const Hrana& h = it.value();
        std::pair<int,int> key{std::min(h.mIndexA, h.mIndexB),
                               std::max(h.mIndexA, h.mIndexB)};
        if (zvyrazneneHrany.count(key) == 0)
            continue;  // already drawn in pass 1

        const Vrchol& va = mVrcholy[static_cast<std::size_t>(h.mIndexA)];
        const Vrchol& vb = mVrcholy[static_cast<std::size_t>(h.mIndexB)];
        QPointF p1(va.mX, va.mY), p2(vb.mX, vb.mY);

        mScene->addLine(QLineF(p1, p2), highlightPen);

        QGraphicsTextItem* wLabel = mScene->addText(QString::number(h.mVaha));
        wLabel->setFont(smallFont);
        wLabel->setPos((p1 + p2) / 2.0);
        wLabel->setDefaultTextColor(zvyraznenaBarva);
    }

    // ── Pass 3: vertices (always on top) ─────────────────────────────────────
    const int r = 10;
    for (int i = 0; i < static_cast<int>(mVrcholy.size()); ++i) {
        const Vrchol& v = mVrcholy[static_cast<std::size_t>(i)];

        mScene->addEllipse(v.mX - r, v.mY - r, 2 * r, 2 * r,
                           QPen(Qt::black), QBrush(Qt::red));

        QString lbl = QString::number(i) +
                      " [" + QString::number(v.mX) +
                      "," + QString::number(v.mY) + "]";
        QGraphicsTextItem* txt = mScene->addText(lbl);
        txt->setFont(smallFont);
        txt->setPos(v.mX - r, v.mY - 2 * r - 16);
        txt->setDefaultTextColor(Qt::darkBlue);
    }
}

