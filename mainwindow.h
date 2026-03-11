#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <vector>   //neco jako dynamicke pole, jenom chytrejsi
#include <map>  //sklada sa u dvojice hodnot {klic, hodnota}
#include <set>
#include <QMap>
#include <climits>
#include <cmath>

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
    void onDijkstra();
    void onKruskalkuv();

    void onGenerujVrcholy();
    void onGenerujHrany();
    void onZmazVse();
    void onSmazHranu();

    void onImportVrcholu();
    void onImportHrany();
    void onExportGrafu();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::MainWindow *ui;

    int mPocetVrcholu;
    int** mMaticeSousednosti;  //dvorozmerne pole
    std::vector<Vrchol> mVrcholy;
    QMultiMap<int, Hrana> mHrany;

    //v mape je klic a hodnota, klicem bude vzdalenost od pocatecneho vrcholu
    //hodnota bude index daneho vrcholu
    //map anebo multimap nam zabezpeci automaticke trideni od nejmensi vzdalenosti po nejvacsi
    std::multimap<int, int> mDocastneVrcholy;//map nestaci, protoze vice uzlu muze mit rovnakou vzdalenost, chceme aby v mape byli vsichni

    QGraphicsScene* mScene;

    void vytvorMaticiSousednosti();
    void zmazMaticiSousednosti();

    void vypisMaticeSousednosti();

    void initDijktra();
    void vypocitajVzdalenosti();
    void updateSousedu(int indexNejblizsihoVrcholu);
    void vypisVzdalenosti();

    void kresliScene(const std::set<std::pair<int,int>>& zvyrazneneHrany = {},
                     QColor zvyraznenaBarva = Qt::green);

    // Removes the undirected edge (x,y) from all data structures; does nothing
    // if x == y or the edge does not exist. Returns true if an edge was removed.
    bool removeEdge(int x, int y);
};
#endif // MAINWINDOW_H
