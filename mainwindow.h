#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSocketNotifier>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QTableView>
#include <QModelIndex>

#define JOC_8x8 64
#define JOC_10x10 100

#define CLASAMENT_COL_COUNT 2
#define CLASAMENT_ROW_COUNT 10

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    QGroupBox* createMeniu();
    QGroupBox* createClasament();
    QGroupBox* createNume();
    QGroupBox* createJucatori();
    QTableView* createTabla();
    QGroupBox* createMutare();
    void populeazaClasament(char* date);
    void populeazaTabla(char* date);
    void populeazaStareJoc(char* date);
    void populeazaJucatori();
    void populeazaMutare();
    void handle8x8Click();
    void handle10x10Click();
    void handleInregistreazaClick();
    void handleResetMutareClick();
    void handleTrimiteMutareClick();
    void handleTablaClick(const QModelIndex& index);
    void proceseazaNotificare();
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

private:

    QLineEdit *numeField;
    QPushButton *inregistreazaButton;
    QLabel *jucator1Label;
    QLabel *culoarePieseJucator1Label;
    QLabel *pieseCapturateJucator1Label;
    QLabel *jucator2Label;
    QLabel *culoarePieseJucator2Label;
    QLabel *pieseCapturateJucator2Label;
    QPushButton *button_8x8;
    QPushButton *button_10x10;
    QStandardItemModel *clasamentModel;
    QStandardItemModel *tablaModel;
    QTableView *tablaTable;
    QSocketNotifier *serverPush;
    QModelIndex* startMutare;
    QModelIndex* endMutare;
    QLabel *mutareMesajLabel;
    QLabel *mutareStartLabel;
    QLabel *mutareToLabel;
    QLabel *mutareEndLabel;
    QPushButton *mutareResetButton;
    QPushButton *mutareTrimiteButton;
};
#endif // MAINWINDOW_H
