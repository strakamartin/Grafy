#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mMaticeSousednosti = nullptr;
    mPocetVrcholu = 0;

    connect(ui->buttonImportGraf, &QPushButton::clicked, this, &MainWindow::onImportGraf);
    connect(ui->spinBoxPocetVrcholu, &QSpinBox::valueChanged, this, &MainWindow::onPocetVrcholu);
    connect(ui->pushButtonPridejHranu, &QPushButton::clicked, this, &MainWindow::onPridejHranu);

    connect(ui->buttonDijkstra, &QPushButton::clicked, this, &MainWindow::onDijkstra);

    connect(ui->buttonKurskalkuv, &QPushButton::clicked, this, &MainWindow::onKruskalkuv);
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
    for (int i = 0; i < mPocetVrcholu; ++i) {
        ui->comboPrvniVrchol->addItem(QString::number(i));
        ui->comboDruhyVrchol->addItem(QString::number(i));
        ui->comboStartVrchol->addItem(QString::number(i));
    }
}


void MainWindow::onPridejHranu()
{
    int x = ui->comboPrvniVrchol->currentIndex();
    int y = ui->comboDruhyVrchol->currentIndex();
    mMaticeSousednosti[x][y] = ui->spinBoxVahaHrany->value();
    mMaticeSousednosti[y][x] = ui->spinBoxVahaHrany->value();
    vypisMaticeSousednosti();

    //todo update nasledniku
}

void MainWindow::onImportGraf()
{
    int pocetVrcholu = 4;
    ui->spinBoxPocetVrcholu->setValue(pocetVrcholu);

    //init matice
    mMaticeSousednosti[0][1] = 6;
    mMaticeSousednosti[1][0] = 6;
    mMaticeSousednosti[0][2] = 1;
    mMaticeSousednosti[2][0] = 1;
    mMaticeSousednosti[0][3] = 5;
    mMaticeSousednosti[3][0] = 5;
    mMaticeSousednosti[1][2] = 7;
    mMaticeSousednosti[2][1] = 7;
    mMaticeSousednosti[3][2] = 2;
    mMaticeSousednosti[2][3] = 2;
    vypisMaticeSousednosti();

    qDebug() << "dddddddddddddddddddddddddd";

    //init seznam vrcholu
    for (int i = 0; i < pocetVrcholu; ++i) {
        Vrchol tmp;
        mVrcholy.push_back(tmp);  //pridani na konec
        qDebug() << tmp.mX << " " << tmp.mY;
    }

    //pridani doseznamNaslednikuiku, setriduje podle vahy hrany
    mVrcholy[0].seznamNasledniku.insert(1, &mVrcholy[2]);
    mVrcholy[0].seznamNasledniku.insert(5, &mVrcholy[3]);
    mVrcholy[0].seznamNasledniku.insert(6, &mVrcholy[1]);

    mVrcholy[1].seznamNasledniku.insert(6, &mVrcholy[0]);
    mVrcholy[1].seznamNasledniku.insert(7, &mVrcholy[2]);

    mVrcholy[2].seznamNasledniku.insert(1, &mVrcholy[0]);
    mVrcholy[2].seznamNasledniku.insert(2, &mVrcholy[3]);
    mVrcholy[2].seznamNasledniku.insert(7 ,&mVrcholy[1]);

    mVrcholy[3].seznamNasledniku.insert(2, &mVrcholy[2]);
    mVrcholy[3].seznamNasledniku.insert(5, &mVrcholy[0]);
}


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
    initDijktra();
    vypocitajVzdalenosti();
    vypisVzdalenosti();
}

void MainWindow::onKruskalkuv()
{

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
        mVrcholy[i].mVzdalenostOdStartu = INT_MAX;
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

