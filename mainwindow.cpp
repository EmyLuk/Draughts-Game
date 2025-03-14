#include "mainwindow.h"
// #include "model.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtWidgets>
#include <QLabel>
#include <QTableView>
#include <QAbstractTableModel>
#include <QStringList>
#include <QAction>

#define PORT 28765
#define SET_VARIANTA_8x8_CMD "Varianta?nume=8x8"
#define SET_VARIANTA_10x10_CMD "Varianta?nume=10x10"
#define INREGISTREAZA_CLIENT_CMD "Inregistreaza?nume="
#define CLASAMENT_CMD "GetClasament?"
#define MUTARE_CMD "Mutare?"
#define MUTARE_START_ROW_PARAM "startRow"
#define MUTARE_START_COL_PARAM "startCol"
#define MUTARE_END_ROW_PARAM "endRow"
#define MUTARE_END_COL_PARAM "endCol"


#define CLASAMENT_RSP "Clasament"
#define TABLA_RSP "Tabla"
#define IN_ASTEPTARE_RSP "InAsteptare"
#define START_JOC_RSP "StartJoc"
#define INFO_JOC_RSP "InfoJoc"
#define JUCATOR1_RSP_PARAM "jucator1"
#define JUCATOR2_RSP_PARAM "jucator2"
#define CULOARE_PIESE_JUCATOR1_RSP_PARAM "culoare_piese_jucator1"
#define CULOARE_PIESE_JUCATOR2_RSP_PARAM "culoare_piese_jucator2"
#define PIESE_NEGRE_RSP "Negre"
#define PIESE_ALBE_RSP "Albe"
#define PIESE_CAPTURATE_JUCATOR1_RSP_PARAM "piese_capturate_jucator1"
#define PIESE_CAPTURATE_JUCATOR2_RSP_PARAM "piese_capturate_jucator2"
#define JUCATOR_MUTARE_RSP_PARAM "jucator_mutare"
#define STARE_MUTARE_RSP_PARAM "stare_mutare"
#define MUTARE_ACCEPTATA "Acceptata"
#define MUTARE_REFUZATA "Refuzata"
#define MUTARE_VICTORIE "Victorie"
#define TABLA_RSP_PARAM "tabla"

int sockfd;
FILE *logFile;
char numeClient[50];
char jucator1[50];
char jucator1CuloarePiese[5];
char jucator1PieseCapturate[2];
char jucator2[50];
char jucator2CuloarePiese[5];
char jucator2PieseCapturate[2];
char jucatorMutare[50];
char jucatorMutareCuloarePiese[5];
int mutareStartRow = -1;
int mutareStartCol = -1;
int mutareEndRow = -1;
int mutareEndCol = -1;
char stareMutare[10];
char tabla[100];
char variantaJoc[5];

typedef struct {
    char tip_raspuns[50];
    char date[1024];
} Raspuns;

/**
 * Parseaza raspunsuri de forma: tip_raspuns:date
 */
Raspuns parseaza_raspuns(char* buffer) {
    // fprintf(logFile, "De parsat de la server raspuns %s\n", buffer);
    Raspuns result = {"", ""};

    char *delimitator_raspuns = strstr(buffer, ":");
    if (delimitator_raspuns) {
        int index_delimitator = delimitator_raspuns - buffer;
        strncpy(result.tip_raspuns, buffer, index_delimitator);
        result.tip_raspuns[index_delimitator] = '\0';
        int data_length = strlen(buffer) - index_delimitator - 1;
        strncpy(result.date, delimitator_raspuns + 1, data_length);
        result.date[data_length] = '\0';
    } else {
        strncpy(result.tip_raspuns, buffer, sizeof(result.tip_raspuns) - 1);
        result.tip_raspuns[sizeof(result.tip_raspuns) - 1] = '\0';
        result.date[0] = '\0';  // Nu există date
    }

    fprintf(logFile, "Parsat raspuns server de tip %s si date %s\n",
            result.tip_raspuns, result.date);
    return result;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Creeaza Meniu
    QGroupBox *meniuBox = createMeniu();

    // Creeaza Clasament
    QGroupBox *clasamentBox = createClasament();

    QGroupBox *left = new QGroupBox();
    QVBoxLayout *leftLayout= new QVBoxLayout();
    leftLayout->addWidget(meniuBox);
    leftLayout->addWidget(clasamentBox);
    leftLayout->setStretch(1, 20);
    left->setLayout(leftLayout);
    left->setMinimumWidth(150);

    QGroupBox *right = new QGroupBox();
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(createNume());
    rightLayout->addWidget(createJucatori());
    rightLayout->addWidget(createMutare());
    rightLayout->addWidget(createTabla());
    rightLayout->setStretch(3, 20);
    right->setLayout(rightLayout);
    right->setMinimumWidth(650);

    QGroupBox *top = new QGroupBox();
    QHBoxLayout *top_layout= new QHBoxLayout();
    top_layout->addWidget(left);
    top_layout->addWidget(right);
    top_layout->setStretch(1, 20);
    top->setLayout(top_layout);

    setCentralWidget(top);

    // Configurarea conexiunii pentru socket notifier
    serverPush = new QSocketNotifier(QSocketNotifier::Read, this);
    connect(serverPush, &QSocketNotifier::activated, this, &MainWindow::proceseazaNotificare);
    serverPush->setEnabled(false);

    resize(900,600);
}

QGroupBox* MainWindow::createMeniu()
{
    QGroupBox *meniuBox = new QGroupBox("Variante");
    QVBoxLayout *layout = new QVBoxLayout;

    button_8x8 = new QPushButton("Tabla de 8x8", this);
    button_8x8->setGeometry(QRect(QPoint(100, 100), QSize(200, 50)));
    connect(button_8x8, &QPushButton::released, this, &MainWindow::handle8x8Click);
    layout->addWidget(button_8x8);

    button_10x10 = new QPushButton("Tabla de 10x10", this);
    button_10x10->setGeometry(QRect(QPoint(100,200),QSize(200,100)));
    connect(button_10x10, &QPushButton::released, this, &MainWindow::handle10x10Click);
    layout->addWidget(button_10x10);

    meniuBox->setLayout(layout);
    return meniuBox;
}

QGroupBox* MainWindow::createClasament()
{
    QGroupBox *clasamentBox = new QGroupBox("Clasament");
    QVBoxLayout *clasamentLayout = new QVBoxLayout();

    QTableView* clasamentTable = new QTableView();
    clasamentModel = new QStandardItemModel();
    QStringList horizontalHeader = QStringList() << "Nume" << "Scor";
    clasamentModel->setHorizontalHeaderLabels(horizontalHeader);
    clasamentTable->setModel(clasamentModel);
    clasamentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    clasamentLayout->addWidget(clasamentTable);
    clasamentBox->setLayout(clasamentLayout);
    return clasamentBox;
}

/**
 * Populeaza tabelul clasament folosind date trimise de la server in format:
 * Nume1=Scor1,Nume2=Scor2,Nume3=Scor3,
 * @brief MainWindow::populeazaClasament
 * @param date
 */
void MainWindow::populeazaClasament(char* date) {
    clasamentModel->removeRows(0, clasamentModel->rowCount());
    char *token = strtok(date, ",");
    while (token != NULL) {
        char *delimitator_raspuns = strstr(token, "=");
        if (delimitator_raspuns) {
            int index_delimitator = delimitator_raspuns - token;
            char numeJucator[50];
            strncpy(numeJucator, token, index_delimitator);
            numeJucator[index_delimitator] = '\0';
            QStandardItem *numeJucatorCol = new QStandardItem(numeJucator);
            numeJucatorCol->setTextAlignment(Qt::AlignCenter);
            char scor[10];
            strncpy(scor, delimitator_raspuns+1, strlen(token) - index_delimitator -1);
            scor[strlen(token) - index_delimitator -1] = '\0';
            QStandardItem *scorCol = new QStandardItem(scor);
            scorCol->setTextAlignment(Qt::AlignCenter);
            clasamentModel->appendRow(QList<QStandardItem*>() << numeJucatorCol << scorCol);
        }
        token = strtok(NULL, ",");
    }
}

QGroupBox* MainWindow::createNume() {
    QGroupBox* numeBox = new QGroupBox("Client");
    QHBoxLayout *layout = new QHBoxLayout();

    QLabel *numeLabel = new QLabel("");
    numeLabel->setText("Nume");
    numeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    layout->addWidget(numeLabel);

    numeField = new QLineEdit();
    numeField->setEnabled(false);
    layout->addWidget(numeField);
    inregistreazaButton = new QPushButton("Joaca");
    connect(inregistreazaButton, &QPushButton::released, this, &MainWindow::handleInregistreazaClick);
    layout->addWidget(inregistreazaButton);

    numeBox->setLayout(layout);
    return numeBox;
}

QGroupBox* MainWindow::createJucatori() {
    QGroupBox* jucatoriBox = new QGroupBox("Jucatori");
    QGridLayout *layout = new QGridLayout();

    jucator1Label = new QLabel("");
    jucator1Label->setText("-");
    jucator1Label->setAlignment(Qt::AlignLeft);
    layout->addWidget(jucator1Label, 0, 0, Qt::AlignLeft);
    culoarePieseJucator1Label = new QLabel("");
    culoarePieseJucator1Label->setText("-");
    culoarePieseJucator1Label->setAlignment(Qt::AlignLeft);
    layout->addWidget(culoarePieseJucator1Label, 1, 0, Qt::AlignLeft);
    pieseCapturateJucator1Label = new QLabel("");
    pieseCapturateJucator1Label->setText("-");
    pieseCapturateJucator1Label->setAlignment(Qt::AlignLeft);
    layout->addWidget(pieseCapturateJucator1Label, 2, 0, Qt::AlignLeft);
    QLabel *vsLabel = new QLabel("");
    vsLabel->setText("vs");
    vsLabel->setAlignment(Qt::AlignHCenter);
    layout->addWidget(vsLabel, 0, 1, 3, 1, Qt::AlignCenter);
    layout->setColumnStretch(1, 20);
    jucator2Label = new QLabel("");
    jucator2Label->setText("-");
    jucator2Label->setAlignment(Qt::AlignLeft);
    layout->addWidget(jucator2Label, 0, 2, Qt::AlignRight);
    culoarePieseJucator2Label = new QLabel("");
    culoarePieseJucator2Label->setText("-");
    culoarePieseJucator2Label->setAlignment(Qt::AlignLeft);
    layout->addWidget(culoarePieseJucator2Label, 1, 2, Qt::AlignRight);
    pieseCapturateJucator2Label = new QLabel("");
    pieseCapturateJucator2Label->setText("-");
    pieseCapturateJucator2Label->setAlignment(Qt::AlignLeft);
    layout->addWidget(pieseCapturateJucator2Label, 2, 2, Qt::AlignLeft);

    jucatoriBox->setLayout(layout);
    return jucatoriBox;
}

void MainWindow::populeazaJucatori() {
    jucator1Label->setText(jucator1);
    culoarePieseJucator1Label->setText(jucator1CuloarePiese);
    pieseCapturateJucator1Label->setText(jucator1PieseCapturate);
    jucator2Label->setText(jucator2);
    culoarePieseJucator2Label->setText(jucator2CuloarePiese);
    pieseCapturateJucator2Label->setText(jucator2PieseCapturate);
    if (strcmp(jucatorMutare, jucator1) == 0) {
        jucator1Label->setFont(QFont("Arial", 12, QFont::Weight::Bold));
        culoarePieseJucator1Label->setFont(QFont("Arial", 12, QFont::Weight::Bold));
        pieseCapturateJucator1Label->setFont(QFont("Arial", 12, QFont::Weight::Bold));
        jucator2Label->setFont(QFont("Arial", 12, QFont::Weight::Normal));
        culoarePieseJucator2Label->setFont(QFont("Arial", 12, QFont::Weight::Normal));
        pieseCapturateJucator2Label->setFont(QFont("Arial", 12, QFont::Weight::Normal));
    } else {
        jucator1Label->setFont(QFont("Arial", 12, QFont::Weight::Normal));
        culoarePieseJucator1Label->setFont(QFont("Arial", 12, QFont::Weight::Normal));
        pieseCapturateJucator1Label->setFont(QFont("Arial", 12, QFont::Weight::Normal));
        jucator2Label->setFont(QFont("Arial", 12, QFont::Weight::Bold));
        culoarePieseJucator2Label->setFont(QFont("Arial", 12, QFont::Weight::Bold));
        pieseCapturateJucator2Label->setFont(QFont("Arial", 12, QFont::Weight::Bold));
    }
}

/**
 * Populeaza date despre starea jocului folosind date trimise de la server in format:
 * jucator1=nume1,piese_jucator1=Negre,jucator2=nume2,piese_jucator2=Albe
 * @brief MainWindow::populeazaStareJoc
 * @param date
 */
void MainWindow::populeazaStareJoc(char* date) {
    char *token = strtok(date, ",");
    while (token != NULL) {
        char *delimitator_parametru = strstr(token, "=");
        if (delimitator_parametru) {
            int index_delimitator = delimitator_parametru - token;
            char numeParametru[256];
            strncpy(numeParametru, token, index_delimitator);
            numeParametru[index_delimitator] = '\0';
            char valoareParametru[256];
            strncpy(valoareParametru, delimitator_parametru+1, strlen(token) - index_delimitator -1);
            valoareParametru[strlen(token) - index_delimitator -1] = '\0';
            if (strcmp(JUCATOR1_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucator1, valoareParametru);
            } else if (strcmp(CULOARE_PIESE_JUCATOR1_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucator1CuloarePiese, valoareParametru);
            } else if (strcmp(PIESE_CAPTURATE_JUCATOR1_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucator1PieseCapturate, valoareParametru);
            } else if (strcmp(JUCATOR2_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucator2, valoareParametru);
            } else if (strcmp(CULOARE_PIESE_JUCATOR2_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucator2CuloarePiese, valoareParametru);
            } else if (strcmp(PIESE_CAPTURATE_JUCATOR2_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucator2PieseCapturate, valoareParametru);
            } else if (strcmp(JUCATOR_MUTARE_RSP_PARAM, numeParametru) == 0) {
                strcpy(jucatorMutare, valoareParametru);
                if (strcmp(jucatorMutare, jucator1) == 0) {
                    strcpy(jucatorMutareCuloarePiese, jucator1CuloarePiese);
                } else {
                    strcpy(jucatorMutareCuloarePiese, jucator2CuloarePiese);
                }
            } else if (strcmp(STARE_MUTARE_RSP_PARAM, numeParametru) == 0) {
                strcpy(stareMutare, valoareParametru);
            } else if(strcmp(TABLA_RSP_PARAM, numeParametru) == 0) {
                strcpy(tabla, valoareParametru);
            }
        }
        token = strtok(NULL, ",");
    }
    fprintf(logFile, "Stare joc pentru client %s: jucator1 = %s, jucator2 = %s", numeClient, jucator1, jucator2);
    populeazaJucatori();
    populeazaMutare();
    populeazaTabla(tabla);
}

QTableView* MainWindow::createTabla()
{
    tablaTable = new QTableView();
    tablaModel = new QStandardItemModel();
    tablaTable->setModel(tablaModel);
    tablaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tablaTable->resizeColumnsToContents();
    tablaTable->setEnabled(false);
    return tablaTable;
}

void MainWindow::populeazaTabla(char* date) {
    int dimensiune = sqrt(strlen(date));
    fprintf(logFile, "Populeaza tabla dimensiune %d cu %s\n", dimensiune, date);
    tablaModel->clear();
    for (int i = 0; i < dimensiune; ++i) {
        tablaModel->setHorizontalHeaderItem(i, new QStandardItem(QString(QChar('A' + i))));
        tablaModel->setVerticalHeaderItem(i, new QStandardItem(QString::number(i + 1)));
    }

    int index = 0;
    for (int i = 0; i < dimensiune; ++i) {
        for (int j = 0; j < dimensiune; ++j) {
            QStandardItem* item = new QStandardItem();
            if (date[index] == 'N' || date[index] == 'A') {
                item->setText(QString(date[index]));
                item->setTextAlignment(Qt::AlignCenter);
                item->setForeground(Qt::white); // Culoare piesă
            }

            if ((i + j) % 2 == 0) {
                item->setBackground(Qt::white);
            } else {
                item->setBackground(Qt::black);
            }

            tablaModel->setItem(i, j, item);
            index++;
        }
    }
    fprintf(logFile, "S-a populat tabla dimensiune %d cu %s\n", dimensiune, date);
}

QGroupBox* MainWindow::createMutare() {
    QGroupBox* mutareBox = new QGroupBox("Stare");
    QHBoxLayout *layout = new QHBoxLayout();
    mutareMesajLabel = new QLabel();
    layout->addWidget(mutareMesajLabel);
    mutareStartLabel = new QLabel();
    layout->addWidget(mutareStartLabel, Qt::AlignRight);
    mutareToLabel = new QLabel();
    mutareToLabel->setText("->");
    mutareToLabel->setVisible(false);
    layout->addWidget(mutareToLabel);
    mutareEndLabel = new QLabel();
    layout->addWidget(mutareEndLabel, Qt::AlignLeft);
    mutareResetButton = new QPushButton("Reset");
    mutareResetButton->setVisible(false);
    connect(mutareResetButton, &QPushButton::clicked, this, &MainWindow::handleResetMutareClick);
    layout->addWidget(mutareResetButton);
    mutareTrimiteButton = new QPushButton("Trimite");
    mutareTrimiteButton->setVisible(false);
    connect(mutareTrimiteButton, &QPushButton::clicked, this, &MainWindow::handleTrimiteMutareClick);
    layout->addWidget(mutareTrimiteButton);
    layout->setStretch(3, 20);

    mutareBox->setLayout(layout);
    return mutareBox;
}

void MainWindow::populeazaMutare() {
    if (strcmp(MUTARE_VICTORIE,stareMutare) == 0) { // Jocul s-a terminat
        mutareMesajLabel->setText(QString("%1 a castigat jocul!").arg(jucatorMutare));
        tablaTable->setEnabled(false); // Dezactivam tabla
        mutareTrimiteButton->setEnabled(false); // Dezactivam butoanele
        mutareTrimiteButton->setVisible(false);
        mutareResetButton->setEnabled(false);
        mutareResetButton->setVisible(false);
        return;
    }

    if (strcmp(jucatorMutare, numeClient) == 0) {
        if (strcmp(MUTARE_REFUZATA, stareMutare) == 0) { // Mutarea este refuzata
            mutareMesajLabel->setText("Mutare refuzată. Încercați din nou!");
            mutareTrimiteButton->setEnabled(false);
            mutareTrimiteButton->setVisible(true);
            mutareResetButton->setEnabled(true);
            mutareResetButton->setVisible(true);
            tablaTable->setEnabled(true);
            connect(tablaTable, SIGNAL(clicked(const QModelIndex &)), this, SLOT(handleTablaClick(const QModelIndex &)));
        } else { // Mutarea este acceptată sau urmeaza să fie realizată
            mutareMesajLabel->setText("Selectează mutarea");
            mutareToLabel->setVisible(true);
            mutareResetButton->setEnabled(false);
            mutareResetButton->setVisible(true);
            mutareTrimiteButton->setEnabled(false);
            mutareTrimiteButton->setVisible(true);
            tablaTable->setEnabled(true);
            connect(tablaTable, SIGNAL(clicked(const QModelIndex &)), this, SLOT(handleTablaClick(const QModelIndex &)));
        }
    } else {
        if (strcmp(jucator1, numeClient) == 0 || strcmp(jucator2, numeClient) == 0) {
            mutareMesajLabel->setText("Așteapta mutarea adversarului");
            disconnect(tablaTable, SIGNAL(clicked(const QModelIndex &)), this, SLOT(handleTablaClick(const QModelIndex &)));
            tablaTable->setEnabled(false);
        } else {
            mutareMesajLabel->setText(QString("%1 muta acum").arg(jucatorMutare));
        }
        mutareToLabel->setVisible(false);
        mutareResetButton->setVisible(false);
        mutareTrimiteButton->setVisible(false);
    }
}

void MainWindow::handle8x8Click() {
    // Trimite varianta aleasa la server
    strcpy(variantaJoc, "8x8");
    char sendBuffer[256];
    strcpy(sendBuffer, SET_VARIANTA_8x8_CMD);
    send(sockfd, sendBuffer, strlen(sendBuffer), 0);
    fprintf(logFile, "Am trimis la server comanda %s\n", sendBuffer);
    // Primeste tabla de la server
    char recvBuffer[1024];
    int recvSize = recv(sockfd, recvBuffer , sizeof(recvBuffer) , 0);
    if (recvSize > 0) {
        recvBuffer[recvSize] = '\0';
        Raspuns raspuns = parseaza_raspuns(recvBuffer);
        if (strcmp(TABLA_RSP, raspuns.tip_raspuns) == 0) {
            populeazaTabla(raspuns.date);
        } else {
            fprintf(logFile, "Nu am primit tabla de la server. Raspuns primit: %s\n", recvBuffer);
        }
    }
    // Primeste clasament de la server
    recvSize = recv(sockfd, recvBuffer , sizeof(recvBuffer) , 0);
    if (recvSize > 0) {
        recvBuffer[recvSize] = '\0';
        Raspuns raspuns = parseaza_raspuns(recvBuffer);
        if (strcmp(CLASAMENT_RSP, raspuns.tip_raspuns) == 0) {
            populeazaClasament(raspuns.date);
        } else {
            fprintf(logFile, "Nu am primit clasament de la server. Raspuns primit: %s\n", recvBuffer);
        }
    }

    numeField->setEnabled(true);
    inregistreazaButton->setEnabled(true);
}

void MainWindow::handle10x10Click() {
    strcpy(variantaJoc,"10x10");
    // Trimite varianta aleasa la server
    char sendBuffer[256];
    strcpy(sendBuffer, SET_VARIANTA_10x10_CMD);
    send(sockfd, sendBuffer, strlen(sendBuffer), 0);
    fprintf(logFile,"Am trimis la server comanda %s\n", sendBuffer);

    // Primeste tabla de la server
    char recvBuffer[1024];
    int recvSize = recv(sockfd, recvBuffer , sizeof(recvBuffer) , 0);
    if (recvSize > 0) {
        recvBuffer[recvSize] = '\0';
        Raspuns raspuns = parseaza_raspuns(recvBuffer);
        if (strcmp(TABLA_RSP, raspuns.tip_raspuns) == 0) {
            populeazaTabla(raspuns.date);
        } else {
            fprintf(logFile, "Nu am primit tabla de la server. Raspuns primit: %s\n", recvBuffer);
        }
    }
    // Primeste clasament de la server
    recvSize = recv(sockfd, recvBuffer , sizeof(recvBuffer) , 0);
    if (recvSize > 0) {
        recvBuffer[recvSize] = '\0';
        Raspuns raspuns = parseaza_raspuns(recvBuffer);
        if (strcmp(CLASAMENT_RSP, raspuns.tip_raspuns) == 0) {
            populeazaClasament(raspuns.date);
        } else {
            fprintf(logFile, "Nu am primit clasament de la server. Raspuns primit: %s\n", recvBuffer);
        }
    }
    numeField->setEnabled(true);
    inregistreazaButton->setEnabled(true);
}

void MainWindow::handleInregistreazaClick() {
    strcpy(numeClient, numeField->text().toUtf8().data());

    numeField->setEnabled(false);
    inregistreazaButton->setEnabled(false);

    if (strlen(numeClient) > 0) {
        char sendBuffer[256] = {0};
        snprintf(sendBuffer, sizeof(sendBuffer), "%s%s", INREGISTREAZA_CLIENT_CMD, numeClient);
        fprintf(logFile, "Am trimis la server comanda: %s\n", sendBuffer);

        if (send(sockfd, sendBuffer, strlen(sendBuffer), 0) < 0) {
            fprintf(logFile, "Eroare la trimiterea numelui la server\n");
            return;
        }

        char recvBuffer[2048] = {0};
        int recvSize = recv(sockfd, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (recvSize > 0) {
            recvBuffer[recvSize] = '\0';
            fprintf(logFile, "Răspuns primit de la server: %s\n", recvBuffer);

            Raspuns raspuns = parseaza_raspuns(recvBuffer);

            if (strcmp(IN_ASTEPTARE_RSP, raspuns.tip_raspuns) == 0) {
                mutareMesajLabel->setText("Așteaptă începutul unui nou joc.");
                serverPush->setEnabled(true);
            } else if (strcmp(INFO_JOC_RSP, raspuns.tip_raspuns) == 0) {
                populeazaStareJoc(raspuns.date);
                serverPush->setEnabled(true);
            } else if (strcmp("EroareNumeDuplicat", raspuns.tip_raspuns) == 0) {
                mutareMesajLabel->setText("Numele este deja utilizat. Alege alt nume.");
                numeField->setEnabled(true);
                inregistreazaButton->setEnabled(true);
            } else if (strcmp("EroareMaximClienti", raspuns.tip_raspuns) == 0) {
                mutareMesajLabel->setText("Numărul maxim de clienți a fost atins.");
                numeField->setEnabled(true);
                inregistreazaButton->setEnabled(true);
            } else {
                fprintf(logFile, "Răspuns neașteptat: %s\n", recvBuffer);
            }
        } else if (recvSize == 0) {
            fprintf(logFile, "Serverul a închis conexiunea\n");
        } else {
            fprintf(logFile, "Eroare la recepționarea datelor de la server\n");
        }
    } else {
        numeField->setEnabled(true);
        inregistreazaButton->setEnabled(true);
    }
}

void MainWindow::handleTablaClick(const QModelIndex& index) {
    if (mutareStartRow < 0) {
        if (QString(jucatorMutareCuloarePiese[0]) == tablaModel->data(index).toString()) {
            mutareStartRow = index.row();
            mutareStartCol = index.column();
            mutareStartLabel->setText(
                QString("%1%2")
                    .arg(tablaModel->horizontalHeaderItem(index.column())->text())
                    .arg(tablaModel->verticalHeaderItem(index.row())->text())
                );
            mutareResetButton->setEnabled(true);
        }
    } else if (mutareEndRow < 0) {
        if (QString("") == tablaModel->data(index).toString()) {
            mutareEndRow = index.row();
            mutareEndCol = index.column();
            mutareEndLabel->setText(
                QString("%1%2")
                    .arg(tablaModel->horizontalHeaderItem(index.column())->text())
                    .arg(tablaModel->verticalHeaderItem(index.row())->text())
                );
            mutareTrimiteButton->setEnabled(true);
        }
    }
}

void MainWindow::handleResetMutareClick() {
    mutareResetButton->setEnabled(false);
    mutareTrimiteButton->setEnabled(false);
    mutareStartLabel->setText("");
    mutareEndLabel->setText("");
    mutareStartRow = -1;
    mutareStartCol = -1;
    mutareEndRow = -1;
    mutareEndCol = -1;
}

void MainWindow::handleTrimiteMutareClick() {
    char sendBuffer[512] = {0};
    snprintf(sendBuffer, sizeof(sendBuffer), "%s%s=%d,%s=%d,%s=%d,%s=%d", MUTARE_CMD,
             MUTARE_START_ROW_PARAM, mutareStartRow,
             MUTARE_START_COL_PARAM, mutareStartCol,
             MUTARE_END_ROW_PARAM, mutareEndRow,
             MUTARE_END_COL_PARAM, mutareEndCol
             );
    fprintf(logFile, "Am trimis la server comanda: %s\n", sendBuffer);
    if (send(sockfd, sendBuffer, strlen(sendBuffer), 0) < 0) {
        fprintf(logFile, "Eroare la trimiterea mutarii la server\n");
        return;
    }
    mutareResetButton->setVisible(false);
    mutareTrimiteButton->setVisible(false);
    mutareStartLabel->setText("");
    mutareEndLabel->setText("");
    mutareStartRow = -1;
    mutareStartCol = -1;
    mutareEndRow = -1;
    mutareEndCol = -1;
}

void MainWindow::proceseazaNotificare() {
    char recvBuffer[2048];
    int recvSize = recv(sockfd, recvBuffer, sizeof(recvBuffer), 0);
    if (recvSize > 0) {
        recvBuffer[recvSize] = '\0';
        Raspuns raspuns = parseaza_raspuns(recvBuffer);

        // Dacă primim răspuns de tip InfoJoc
        if (strcmp(INFO_JOC_RSP, raspuns.tip_raspuns) == 0) {
            fprintf(logFile, "Populeaza stare joc pentru client %s\n", numeClient);
            populeazaStareJoc(raspuns.date);
            if (strcmp(MUTARE_VICTORIE, stareMutare) == 0) {
                fprintf(logFile, "Trimit Clasament: stare_mutare=%s\n", raspuns.date);

                char sendBuffer[256];
                strcpy(sendBuffer, CLASAMENT_CMD);
                strcat(sendBuffer, "varianta=");
                strcat(sendBuffer, variantaJoc);

                // Trimitem comanda către server
                send(sockfd, sendBuffer, strlen(sendBuffer), 0);
            }
        } else if (strcmp(IN_ASTEPTARE_RSP, raspuns.tip_raspuns) == 0) {
            mutareMesajLabel->setText("Așteaptă începutul unui nou joc.");
            mutareToLabel->setVisible(false);
            mutareResetButton->setVisible(false);
            mutareTrimiteButton->setVisible(false);
        }
        else if (strcmp(CLASAMENT_RSP, raspuns.tip_raspuns) == 0) {
            populeazaClasament(raspuns.date);
        }
    }
}

int conectare_server() {
    logFile = fopen("client_log.txt","w");
    setbuf(logFile,NULL);

    struct sockaddr_in servaddr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(logFile,"Eroare socket respins\n");
        exit(EXIT_FAILURE);
    }
    fprintf(logFile, "Conectare client cu socket fd=%d\n", sockfd);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
        fprintf(logFile,"Eroare Adresa invalida!\n");
        exit(EXIT_FAILURE);
    }
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        fprintf(logFile,"Eroare la conexiune\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int deconectare_server() {
    fprintf(logFile,"Deconectare client cu socket fd=%d\n", sockfd);
    close(sockfd);
    fclose(logFile);
    return 0;
}

void MainWindow::showEvent(QShowEvent *event) {
    event->accept();
    conectare_server();
    serverPush->setSocket(sockfd);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
    deconectare_server();
}
