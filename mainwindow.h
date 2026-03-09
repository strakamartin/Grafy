#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>   //neco jako dynamicke pole, jenom chytrejsi
#include <map>  //sklada sa u dvojice hodnot {klic, hodnota}
#include <QMap>

#include "vrchol.h"
#include "hrana.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onPocetVrcholu();
    void onPridejHranu();
    void onImportGraf();
    void onDijkstra();
    void onKruskalkuv();

private:
    Ui::MainWindow *ui;

    int mPocetVrcholu;
    int** mMaticeSousednosti;  //dvorozmerne pole
    std::vector<Vrchol> mVrcholy;  //vektor je take vylepsene dynamicke pole, kde se nemusite starat o alokaciu a dealokaciu
    QMultiMap<int, Hrana> mHrany;

    //v mape je klic a hodnota, klicem bude vzdalenost od pocatecneho vrcholu
    //hodnota bude index daneho vrcholu
    //map anebo multimap nam zabezpeci automaticke trideni od nejmensi vzdalenosti po nejvacsi
    std::multimap<int, int> mDocastneVrcholy;//map nestaci, protoze vice uzlu muze mit rovnakou vzdalenost, chceme aby v mape byli vsichni

    void vytvorMaticiSousednosti();
    void zmazMaticiSousednosti();

    void vypisMaticeSousednosti();

    void initDijktra();
    void vypocitajVzdalenosti();
    void updateSousedu(int indexNejblizsihoVrcholu);
    void vypisVzdalenosti();

};
#endif // MAINWINDOW_H
