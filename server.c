#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <asm-generic/socket.h>

#define PORT 28765
#define MAX_CONECTATI 10
#define MAX_CLASAMENT 100
#define JOC_8x8 8
#define JOC_10x10 10

#define SET_VARIANTA_CMD "Varianta"
#define VARIANTA_PARAM_8x8 "8x8"
#define VARIANTA_PARAM_10x10 "10x10"

#define INREGISTREAZA_CMD "Inregistreaza"
#define NUME_CMD_PARAM "nume"

#define MUTARE_CMD "Mutare"
#define MUTARE_START_ROW_PARAM "startRow"
#define MUTARE_START_COL_PARAM "startCol"
#define MUTARE_END_ROW_PARAM "endRow"
#define MUTARE_END_COL_PARAM "endCol"
#define EROARE_MUTARE_RSP "EroareMutare"
#define CONFIRMARE_MUTARE_RSP "ConfirmareMutare"
#define CLASAMENT_CMD "GetClasament"
#define CLASAMENT_RSP "Clasament:"
#define TABLA_RSP "Tabla:"
#define IN_ASTEPTARE_RSP "InAsteptare"
#define INFO_JOC_RSP "InfoJoc:"
#define JUCATOR1_RSP_PARAM "jucator1"
#define JUCATOR2_RSP_PARAM "jucator2"
#define CULOARE_PIESE_JUCATOR1_RSP_PARAM "culoare_piese_jucator1"
#define CULOARE_PIESE_JUCATOR2_RSP_PARAM "culoare_piese_jucator2"
#define PIESE_NEGRE "Negre"
#define PIESE_ALBE "Albe"
#define PIESE_CAPTURATE_JUCATOR1_RSP_PARAM "piese_capturate_jucator1"
#define PIESE_CAPTURATE_JUCATOR2_RSP_PARAM "piese_capturate_jucator2"
#define JUCATOR_MUTARE_RSP_PARAM "jucator_mutare"
#define STARE_MUTARE_RSP_PARAM "stare_mutare"
#define MUTARE_ACCEPTATA "Acceptata"
#define MUTARE_REFUZATA "Refuzata"
#define MUTARE_VICTORIE "Victorie"
#define TABLA_RSP_PARAM "tabla"

typedef struct {
    char nume_client[50];
    int scor;
} ScorJucator;

typedef struct {
    char nume_client[50];
    int socket_client;
} ClientConectat;

typedef struct {
	ClientConectat clienti_conectati[MAX_CONECTATI];
	int clienti_conectati_count;
	ClientConectat jucator1;
	char jucator1_culoare_piese[5];
	int jucator1_piese_capturate;
	ClientConectat jucator2;
	char jucator2_culoare_piese[5];
	int jucator2_piese_capturate;
	ClientConectat* jucator_mutare;
	char stare_mutare[10];
	char tabla_de_joc[100];
	ScorJucator clasament[MAX_CLASAMENT];
	int clasament_count;
	int cod_varianta;
} Dame;

Dame dame_8x8 = {
	.clienti_conectati = { },
	.clienti_conectati_count = 0,
	.jucator1 = { .nume_client = { '\0' }, .socket_client = -1 },
	.jucator1_culoare_piese = { '\0' },
	.jucator1_piese_capturate = 0,
	.jucator2 = { .nume_client = { '\0' }, .socket_client = -1 },
	.jucator2_culoare_piese = { '\0' },
	.jucator2_piese_capturate = 0,
	.stare_mutare = { '\0' },
	.tabla_de_joc = {
			 '.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
			 'N', '.', 'N', '.', 'N', '.', 'N', '.' ,
			 '.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
			 '.', '.', '.', '.', '.', '.', '.', '.' ,
			 '.', '.', '.', '.', '.', '.', '.', '.' ,
			 'A', '.', 'A', '.', 'A', '.', 'A', '.' ,
			 '.', 'A', '.', 'A', '.', 'A', '.', 'A' ,
			 'A', '.', 'A', '.', 'A', '.', 'A', '.'
		},
	.clasament = {
			{ .nume_client = "Marius", .scor = 25 },
			{ .nume_client = "George", .scor = 23 },
			{ .nume_client = "Andreea", .scor = 22 },
			{ .nume_client = "Adrian", .scor = 18 }
	},
	.clasament_count = 4,
	.cod_varianta = JOC_8x8
};

Dame dame_10x10 = {
	.clienti_conectati = { },
	.clienti_conectati_count = 0,
	.jucator1 = { .nume_client = { '\0' }, .socket_client = -1 },
	.jucator1_culoare_piese = { '\0' },
	.jucator1_piese_capturate = 0,
	.jucator2 = { .nume_client = { '\0' }, .socket_client = -1 },
	.jucator2_culoare_piese = { '\0' },
	.jucator2_piese_capturate = 0,
	.stare_mutare = { '\0' },
	.tabla_de_joc = {
			 '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
			 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.' ,
			 '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
			 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.' ,
			 '.', '.', '.', '.', '.', '.', '.', '.', '.', '.' ,
			 '.', '.', '.', '.', '.', '.', '.', '.', '.', '.' ,
			 '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A',
			 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.',
			 '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A',
			 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.'
	},
	.clasament = {
			{ .nume_client = "Andreea", .scor = 18 },
			{ .nume_client = "Victor", .scor = 16 },
			{ .nume_client = "Marius", .scor = 15 }
	},
	.clasament_count = 3,
	.cod_varianta = JOC_10x10
};

typedef struct {
	char nume[50];
	char valoare[50];
} ParametruComanda;

typedef struct {
    char nume[50];
    ParametruComanda parametri[10];
    int parametri_count;
} Comanda;

typedef struct {
	int startRow;
	int startCol;
	int endRow;
	int endCol;
} Mutare;

FILE *log_file;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Parseaza comenzi de forma: comanda?nume_parametru=valoare_parametru
 */
Comanda parseaza_comanda(char* buffer) {
	fprintf(log_file, "De parsat de la client comanda %s\n", buffer);
	Comanda result;
	char *delimitator_comanda = strstr(buffer, "?");
	int index_delimitator = delimitator_comanda - buffer;
	strncpy(result.nume, buffer, index_delimitator);
	result.nume[index_delimitator] = '\0';
	int params_length = strlen(buffer) - index_delimitator - 1;
	char params[1024];
	strncpy(params, delimitator_comanda + 1, params_length);
	params[params_length] = '\0';
	if (delimitator_comanda) {
		int index = 0;
		char *token = strtok(params, ",");
		while (token != NULL) {
			char *delimitator_param = strstr(token, "=");
		    if (delimitator_param) {
		        int index_delimitator = delimitator_param - token;
		        strncpy(result.parametri[index].nume, token, index_delimitator);
		        result.parametri[index].nume[index_delimitator] = '\0';
		        strncpy(result.parametri[index].valoare, delimitator_param+1, strlen(token) - index_delimitator -1);
		        result.parametri[index].valoare[strlen(token) - index_delimitator -1] = '\0';
		    }
		    token = strtok(NULL, ",");
		    index++;
		}
		result.parametri_count = index;
	}
	fprintf(log_file, "Am parsat de la client comanda %s cu %d parametri:\n", result.nume, result.parametri_count);
	for (int i = 0; i < result.parametri_count; i++) {
		fprintf(log_file, "%s=%s\n", result.parametri[i].nume, result.parametri[i].valoare);
	}
	return result;
}

// Adaugam tabla la raspuns
void tipareste_tabla(char* dest, Dame* dame) {
	int tabla_size = 0;
	if (dame->cod_varianta == JOC_8x8) {
		tabla_size = 64;
	} else if (dame->cod_varianta == JOC_10x10) {
		tabla_size = 100;
	}
	strncat(dest, dame->tabla_de_joc, tabla_size);
	strcat(dest, "\0");
}

// Adaugam clasament la raspuns in forma: nume1=scor1,nume2=scor2,
void tipareste_clasament(char* dest, ScorJucator clasament[MAX_CLASAMENT]) {
	for (int i = 0; i < MAX_CLASAMENT; i++) {
		if (strcmp(clasament[i].nume_client, "\0") != 0) {
			strcat(dest, clasament[i].nume_client);
			strcat(dest, "=");
			char s[5];
			sprintf(s, "%d", clasament[i].scor);
			strcat(dest, s);
			strcat(dest, ",");
		} else {
			break;
		}
	}
	strcat(dest, "\0");
}

// Adaugam stare joc la raspuns in forma: jucator1=nume1,piese_jucator1=Negre,jucator2=nume2,piese_jucator2=Albe,mutare=nume1
void tipareste_stare_joc(char* dest, Dame* dame) {
	strcat(dest, JUCATOR1_RSP_PARAM);
	strcat(dest, "=");
	strcat(dest, dame->jucator1.nume_client);
	strcat(dest, ",");
	strcat(dest, CULOARE_PIESE_JUCATOR1_RSP_PARAM);
	strcat(dest, "=");
	strcat(dest, dame->jucator1_culoare_piese);
	strcat(dest, ",");
	strcat(dest, PIESE_CAPTURATE_JUCATOR1_RSP_PARAM);
	strcat(dest, "=");
	char jucator1PieseCapturate[3];
	sprintf(jucator1PieseCapturate, "%d", dame->jucator1_piese_capturate);
	strcat(dest, jucator1PieseCapturate);
	strcat(dest, ",");
	strcat(dest, JUCATOR2_RSP_PARAM);
	strcat(dest, "=");
	strcat(dest, dame->jucator2.nume_client);
	strcat(dest, ",");
	strcat(dest, CULOARE_PIESE_JUCATOR2_RSP_PARAM);
	strcat(dest, "=");
	strcat(dest, dame->jucator2_culoare_piese);
	strcat(dest, ",");
	strcat(dest, PIESE_CAPTURATE_JUCATOR2_RSP_PARAM);
	strcat(dest, "=");
	char jucator2PieseCapturate[3];
	sprintf(jucator2PieseCapturate, "%d", dame->jucator2_piese_capturate);
	strcat(dest, jucator2PieseCapturate);
	strcat(dest, ",");
	strcat(dest, JUCATOR_MUTARE_RSP_PARAM);
	strcat(dest, "=");
	strcat(dest, dame->jucator_mutare->nume_client);
	strcat(dest, ",");
	strcat(dest, STARE_MUTARE_RSP_PARAM);
	strcat(dest, "=");
	strcat(dest, dame->stare_mutare);
	strcat(dest, ",");
	strcat(dest, TABLA_RSP_PARAM);
	strcat(dest, "=");
	int tabla_size = 0;
	if (dame->cod_varianta == JOC_8x8) {
		tabla_size = 64;
	} else if (dame->cod_varianta == JOC_10x10) {
		tabla_size = 100;
	}
	strncat(dest, dame->tabla_de_joc, tabla_size);
	strcat(dest, "\0");
}

int adauga_client_conectat(Dame* dame, const char *nume, int socket) {
    // Verificam daca MAX clienti conectati
    if (dame->clienti_conectati_count == MAX_CONECTATI) {
    	fprintf(log_file, "S-a atins numarul maxim de clienti conectati la 8x8: %d\n", MAX_CONECTATI);
    	return -2;
    }

    // Verificam daca numele este deja utilizat
    for (int i = 0; i < MAX_CONECTATI; i++) {
        if (strcmp(dame->clienti_conectati[i].nume_client, nume) == 0) {
            return -1;
        }
    }
    // Setam datele la primul index liber din vector
    for (int i = 0; i < MAX_CONECTATI; i++) {
    	if (dame->clienti_conectati[i].nume_client[0] == '\0') {
    		strcpy(dame->clienti_conectati[i].nume_client, nume);
    		dame->clienti_conectati[i].socket_client = socket;
    		break;
    	}
    }
    dame->clienti_conectati_count++;
    fprintf(log_file, "Numar clienti conectati la %d: %d\n", dame->cod_varianta, dame->clienti_conectati_count);
    return 0;
}

int sterge_client_conectat(Dame* dame, int socket) {
	if (dame == NULL) {
		fprintf(log_file, "Clientul %d nu a fost conectat la nici o varianta de joc\n", socket);
		return 0;
	}
    // Cautam socketul intre clienti conectati la varianta 8x8
    for (int i = 0; i < MAX_CONECTATI; i++) {
        if (dame->clienti_conectati[i].socket_client == socket) {
        	strcpy(dame->clienti_conectati[i].nume_client, "\0");
        	dame->clienti_conectati[i].socket_client = -1;
        	dame->clienti_conectati_count--;
        }
    }
    fprintf(log_file, "Numar clienti conectati la %d: %d\n", dame->cod_varianta, dame->clienti_conectati_count);
    return 0;
}

int anuleaza_joc(Dame* dame) {
	strcpy(dame->jucator1.nume_client, "\0");
	dame->jucator1.socket_client = -1;
	strcpy(dame->jucator1_culoare_piese, "\0");
	dame->jucator1_piese_capturate = 0;
	strcpy(dame->jucator2.nume_client, "\0");
	dame->jucator2.socket_client = -1;
	strcpy(dame->jucator2_culoare_piese, "\0");
	dame->jucator2_piese_capturate = 0;
	dame->jucator_mutare = NULL;
	strcpy(dame->stare_mutare, "\0");
	if (JOC_8x8 == dame->cod_varianta) {
		char tabla[64] = {
				'.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
				'N', '.', 'N', '.', 'N', '.', 'N', '.' ,
				'.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
				'.', '.', '.', '.', '.', '.', '.', '.' ,
				'.', '.', '.', '.', '.', '.', '.', '.' ,
				'A', '.', 'A', '.', 'A', '.', 'A', '.' ,
				'.', 'A', '.', 'A', '.', 'A', '.', 'A' ,
				'A', '.', 'A', '.', 'A', '.', 'A', '.'
		};
		memcpy(dame->tabla_de_joc, tabla, 64);
	} else {
		char tabla[100] = {
				 '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
				 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.' ,
				 '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N' ,
				 'N', '.', 'N', '.', 'N', '.', 'N', '.', 'N', '.' ,
				 '.', '.', '.', '.', '.', '.', '.', '.', '.', '.' ,
				 '.', '.', '.', '.', '.', '.', '.', '.', '.', '.' ,
				 '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A',
			 	 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.',
			 	 '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A',
			 	 'A', '.', 'A', '.', 'A', '.', 'A', '.', 'A', '.'
		};
		memcpy(dame->tabla_de_joc, tabla, 100);
	}
}

int alege_jucatori(Dame* dame) {
	if (dame->clienti_conectati_count >= 2 && dame->jucator1.nume_client[0] == '\0') {
		int i = 0;
		for (; i < MAX_CONECTATI; i++) {
			if (dame->clienti_conectati[i].nume_client[0] != '\0') {
				strcpy(dame->jucator1.nume_client, dame->clienti_conectati[i].nume_client);
				dame->jucator1.socket_client = dame->clienti_conectati[i].socket_client;
				strcpy(dame->jucator1_culoare_piese, PIESE_NEGRE);
				break;
			}
		}
		for (int j = i; j < MAX_CONECTATI; j++) {
			if (dame->clienti_conectati[j].nume_client[0] != '\0') {
				strcpy(dame->jucator2.nume_client, dame->clienti_conectati[j].nume_client);
				dame->jucator2.socket_client = dame->clienti_conectati[j].socket_client;
				strcpy(dame->jucator2_culoare_piese, PIESE_ALBE);
			}
		}
		dame->jucator_mutare = &dame->jucator1;
	}
	return 0;
}

int is_jucator(Dame* dame, int socket) {
	return socket >= 0 && (dame->jucator1.socket_client == socket || dame->jucator2.socket_client == socket);
}

void actualizeaza_clasament(Dame* dame, const char* castigator) {
    for (int i = 0; i < dame->clasament_count; i++) {
        if (strcmp(dame->clasament[i].nume_client, castigator) == 0) {
            dame->clasament[i].scor += 5; 
            return;
        }
    }

    if (dame->clasament_count < MAX_CLASAMENT) {
        strcpy(dame->clasament[dame->clasament_count].nume_client, castigator);
        dame->clasament[dame->clasament_count].scor = 5;
        dame->clasament_count++;
    } else {
        fprintf(stderr, "Clasamentul este plin, nu putem adauga castigatorul.\n");
    }
}


int is_mutare_valida(Dame* dame, Mutare mutare) {
    int rowSize = (dame->cod_varianta == JOC_8x8) ? 8 : 10;
    int pozitieStart = mutare.startRow * rowSize + mutare.startCol;
    int pozitieEnd = mutare.endRow * rowSize + mutare.endCol;

    // Validam limitele pozitiilor
    if (mutare.startRow < 0 || mutare.startRow >= rowSize ||
        mutare.startCol < 0 || mutare.startCol >= rowSize ||
        mutare.endRow < 0 || mutare.endRow >= rowSize ||
        mutare.endCol < 0 || mutare.endCol >= rowSize) {
        return 0; // Pozitii invalide
    }

    char piesa = dame->tabla_de_joc[pozitieStart];

    // Verificam daca piesa apartine jucatorului care muta
    if (dame->jucator_mutare == &dame->jucator1 && piesa != 'N') { 
        return 0; 
    }
    if (dame->jucator_mutare == &dame->jucator2 && piesa != 'A') { 
        return 0; 
    }

    
    if (dame->tabla_de_joc[pozitieEnd] != '.') {
        return 0; 
    }
    if (piesa == 'N' && mutare.endRow <= mutare.startRow) {
        return 0; 
    }
    if (piesa == 'A' && mutare.endRow >= mutare.startRow) {
        return 0; 
    }

    if (abs(mutare.startRow - mutare.endRow) == 1 &&
        abs(mutare.startCol - mutare.endCol) == 1) {
        return 1;
    }
    if (abs(mutare.startRow - mutare.endRow) == 2 &&
        abs(mutare.startCol - mutare.endCol) == 2) {
    
        int capturaRow = (mutare.startRow + mutare.endRow) / 2;
        int capturaCol = (mutare.startCol + mutare.endCol) / 2;
        int pozitieCaptura = capturaRow * rowSize + capturaCol;
        char piesaCapturata = dame->tabla_de_joc[pozitieCaptura];
        if (dame->jucator_mutare == &dame->jucator1 && piesaCapturata == 'A') {
            return 1; 
        }
        if (dame->jucator_mutare == &dame->jucator2 && piesaCapturata == 'N') {
            return 1; 
        }
    }

    return 0; 
}

int muta_jucatori_la_coada(Dame* dame){
	
	ClientConectat copie_conectati[MAX_CONECTATI];
	int count=0;
	for( int i =0; i<MAX_CONECTATI;i++){
		if( dame->clienti_conectati[i].socket_client>0 && !is_jucator(dame, dame->clienti_conectati[i].socket_client)){
			strcpy(copie_conectati[count].nume_client,dame->clienti_conectati[i].nume_client);
    		copie_conectati[count].socket_client = dame->clienti_conectati[i].socket_client;
			count++;
		}
	}
	strcpy(copie_conectati[count].nume_client,dame->jucator1.nume_client);
	copie_conectati[count].socket_client = dame->jucator1.socket_client;
	count++;
	strcpy(copie_conectati[count].nume_client,dame->jucator2.nume_client);
    copie_conectati[count].socket_client = dame->jucator2.socket_client;
	count++;

	for( int i=0;i<count;i++){
		strcpy(dame->clienti_conectati[i].nume_client, copie_conectati[i].nume_client);
		dame->clienti_conectati[i].socket_client= copie_conectati[i].socket_client;
	}

	for(int i= count; i<MAX_CONECTATI;i++){
		strcpy(dame->clienti_conectati[i].nume_client, "\0");
        dame->clienti_conectati[i].socket_client = -1;
	}
	return 0;
}

void actualizeaza_tabla(Dame* dame, Mutare mutare) {
    if (is_mutare_valida(dame, mutare)) {
        int rowSize = (dame->cod_varianta == JOC_8x8) ? 8 : 10;

        // Calculam pozitia de start si extragem piesa
        int pozitieStart = mutare.startRow * rowSize + mutare.startCol;
        char piesa = dame->tabla_de_joc[pozitieStart];
        dame->tabla_de_joc[pozitieStart] = '.';

        // Calculam pozitia de stop si mutam piesa
        int pozitieEnd = mutare.endRow * rowSize + mutare.endCol;
        dame->tabla_de_joc[pozitieEnd] = piesa;

        // Verificam daca mutarea include o captura 
        if (abs(mutare.startRow - mutare.endRow) == 2 && abs(mutare.startCol - mutare.endCol) == 2) {
            // Calculam pozitia piesei capturate
            int capturaRow = (mutare.startRow + mutare.endRow) / 2;
            int capturaCol = (mutare.startCol + mutare.endCol) / 2;
            int pozitieCaptura = capturaRow * rowSize + capturaCol;

            // Eliminam piesa capturata
            dame->tabla_de_joc[pozitieCaptura] = '.';

            // Actualizam scorul jucatorului curent
            if (dame->jucator_mutare == &dame->jucator1) {
                dame->jucator1_piese_capturate++;
                if (dame->jucator1_piese_capturate >= 5) {
                    strcpy(dame->stare_mutare, MUTARE_VICTORIE);
					actualizeaza_clasament(dame, dame->jucator1.nume_client);
                    return; 
                }
            } else if (dame->jucator_mutare == &dame->jucator2) {
                dame->jucator2_piese_capturate++;
                if (dame->jucator2_piese_capturate >= 5) {
                    strcpy(dame->stare_mutare, MUTARE_VICTORIE);
					actualizeaza_clasament(dame, dame->jucator2.nume_client);
                    return; 
                }
            }
        }

        // Schimbam randul jucatorului care muta
        if (dame->jucator_mutare == &dame->jucator1) {
            dame->jucator_mutare = &dame->jucator2;
        } else {
            dame->jucator_mutare = &dame->jucator1;
        }

        // Actualizam starea mutarii
        strcpy(dame->stare_mutare, MUTARE_ACCEPTATA);
    } else {
        // Mutarea este invalida, pastram jucatorul curent
        strcpy(dame->stare_mutare, MUTARE_REFUZATA);
        dame->jucator_mutare = dame->jucator_mutare;
    }
}
void gestiuneClientConectat(Dame* varianta_dame, int socket_desc) {
	if (varianta_dame->clienti_conectati_count == 1) {
       	// Pentru primul client conectat trimit doar mesaj ca e pus in asteptare
       	char raspuns[50];
       	strcpy(raspuns, IN_ASTEPTARE_RSP);
       	send(socket_desc, raspuns, sizeof(raspuns), 0);
       	fprintf(log_file, "Am trimis la client %d raspuns %s\n", socket_desc, raspuns);
    } else if (varianta_dame->clienti_conectati_count >= 2) {
       	// De al doilea client conectat, verificam daca avem jucatori
       	if (varianta_dame->jucator1.nume_client[0] == '\0') {
       		// Daca nu avem jucatori, alegem doi clienti conectati ca jucatori
       		alege_jucatori(varianta_dame);
       		// Toti clientii conectati primesc info cu starea noului joc
       		char raspuns[1024];
       		strcpy(raspuns, INFO_JOC_RSP);
       		tipareste_stare_joc(raspuns, varianta_dame);
       		for (int i = 0; i < MAX_CONECTATI; i++) {
       			if (varianta_dame->clienti_conectati[i].socket_client > 0) {
       				send(varianta_dame->clienti_conectati[i].socket_client, raspuns, sizeof(raspuns), 0);
       				fprintf(log_file, "Am trimis la client %d raspuns %s\n", varianta_dame->clienti_conectati[i].socket_client, raspuns);
       			}
       		}
       	} else {
       		// Daca avem jucatori trimitem la noul client info joc cu jucatorii, piesele si cel care muta
       		char raspuns_non_jucator[1024];
       		strcpy(raspuns_non_jucator, INFO_JOC_RSP);
       		tipareste_stare_joc(raspuns_non_jucator, varianta_dame);
       		send(socket_desc, raspuns_non_jucator, sizeof(raspuns_non_jucator), 0);
       		fprintf(log_file, "Am trimis la client %d raspuns %s\n", socket_desc, raspuns_non_jucator);
       	}
    }
}

void *gestiune_client(void *socket) {
    int socket_desc = *(int *)socket;
    char buffer[256], cod_varianta_joc[10], nume_client[50];
    Dame* varianta_dame;

    while (1) {
        bzero(buffer, sizeof(buffer));
        int recv_size = recv(socket_desc, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            fprintf(log_file, "Clientul %d s-a deconectat\n", socket_desc);
            pthread_mutex_lock(&lock);
            // Sterge din lista de clienti conectati la cele doua variante de joc
            sterge_client_conectat(varianta_dame, socket_desc);
            // Daca era jucator anulam jocul si reluam initierea jocului
            if (is_jucator(varianta_dame, socket_desc)) {
            	int celalalt_socket_desc;
            	if (varianta_dame->jucator1.socket_client == socket_desc) {
            		celalalt_socket_desc = varianta_dame->jucator2.socket_client;
            	} else {
            		celalalt_socket_desc = varianta_dame->jucator1.socket_client;
            	}
            	anuleaza_joc(varianta_dame);
            	fprintf(log_file, "Clientul %d era jucator: Joc intrerupt prin deconectarea unui jucator\n", socket_desc);
            	gestiuneClientConectat(varianta_dame, celalalt_socket_desc);
            }
            pthread_mutex_unlock(&lock);
            break;
        }
        buffer[recv_size] = '\0';
        Comanda comanda = parseaza_comanda(buffer);
        if (strcmp(SET_VARIANTA_CMD, comanda.nume) == 0) {
        	if (strcmp(VARIANTA_PARAM_8x8, comanda.parametri[0].valoare) == 0) {
        		varianta_dame = &dame_8x8;
        	} else if (strcmp(VARIANTA_PARAM_10x10, comanda.parametri[0].valoare) == 0) {
        		varianta_dame = &dame_10x10;
        	}
        	// Pentru comanda Varianta se trimit la client tabla si clasamentul pentru varianta
            char raspuns[256];
            strcpy(raspuns, TABLA_RSP);
            tipareste_tabla(raspuns, varianta_dame);
            send(socket_desc, raspuns, sizeof(raspuns), 0);
            fprintf(log_file, "Am trimis la client %d raspuns %s\n", socket_desc, raspuns);
            memset(raspuns, 0, strlen(raspuns));
            strcpy(raspuns, CLASAMENT_RSP);
            tipareste_clasament(raspuns, varianta_dame->clasament);
            send(socket_desc, raspuns, sizeof(raspuns), 0);
            fprintf(log_file, "Am trimis la client %d raspuns %s\n", socket_desc, raspuns);
        } else if (strcmp(INREGISTREAZA_CMD, comanda.nume) == 0) {
        	// Protejam cu mutex pentru a nu alege jucatori simultan pe 2 threaduri diferite
        	pthread_mutex_lock(&lock);
        	// Pentru comanda inregistreaza se inregistreaza socket descriptor si nume client
        	// la varianta aleasa si apoi trimit mesaje clientilor conectati
            int conectat = adauga_client_conectat(varianta_dame, comanda.parametri[0].valoare, socket_desc);
            if (conectat == 0) {
                gestiuneClientConectat(varianta_dame, socket_desc);
            }else if( conectat==-1){
				char raspuns[50];
				strcpy(raspuns,"EroareNumeDuplicat");
				send(socket_desc,raspuns,strlen(raspuns),0);
			}
            pthread_mutex_unlock(&lock);
        } else if (strcmp(MUTARE_CMD, comanda.nume) == 0) {
        	pthread_mutex_lock(&lock);//blocheaza mutex ul. Previne conflicte
			
        	// Verificam daca este randul jucatorului sa mute
   			if (varianta_dame->jucator_mutare->socket_client == socket_desc) {
   				Mutare mutare = {
   					.startRow = atoi(comanda.parametri[0].valoare),
   					.startCol = atoi(comanda.parametri[1].valoare),
					.endRow = atoi(comanda.parametri[2].valoare),
					.endCol = atoi(comanda.parametri[3].valoare)
   				};
   				actualizeaza_tabla(varianta_dame, mutare);
   				// Trimitem INFO_JOC cu actualizarile la toti clientii conectati
   				char raspuns_mutare[1024];
   				strcpy(raspuns_mutare, INFO_JOC_RSP);
   				tipareste_stare_joc(raspuns_mutare, varianta_dame);
   				for (int i = 0; i < MAX_CONECTATI; i++) {
   					if (varianta_dame->clienti_conectati[i].socket_client > 0) {
   				    	send(varianta_dame->clienti_conectati[i].socket_client, raspuns_mutare, sizeof(raspuns_mutare), 0);
   				    	fprintf(log_file, "Am trimis la client %d raspuns %s\n", varianta_dame->clienti_conectati[i].socket_client, raspuns_mutare);
   					}
   				}
				if(strcmp(MUTARE_VICTORIE, varianta_dame->stare_mutare)==0){
					sleep(10);
					// Muta jucatori la coada clienti conectati
					muta_jucatori_la_coada(varianta_dame);
					anuleaza_joc(varianta_dame);
					fprintf(log_file, "Jocul a fost resetat dupa victorie");
					gestiuneClientConectat(varianta_dame, varianta_dame->clienti_conectati[0].socket_client);
				}			
   			}
   			pthread_mutex_unlock(&lock);
        }
		else if (strcmp(CLASAMENT_CMD, comanda.nume) == 0) {
    	pthread_mutex_lock(&lock);

    	// Construim raspunsul cu clasamentul actualizat
		char raspuns[1024] = {0};
		strcpy(raspuns, CLASAMENT_RSP);
		tipareste_clasament(raspuns, varianta_dame->clasament);

		// Trimitem clasamentul catre client
		send(socket_desc, raspuns, strlen(raspuns), 0);
		fprintf(log_file, "Am trimis clasamentul actualizat la client %d: %s\n", socket_desc, raspuns);

		pthread_mutex_unlock(&lock);
		}
    }

    close(socket_desc);
    pthread_exit(NULL);
}


int main() {
    log_file = fopen("server_log.txt", "w");
    setbuf(log_file, NULL);

    int socket_fd;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { ///creeam un socket TCP (Transmission Control Protocul).
        fprintf(log_file, "Eroare Opening server socket a s\n");
        exit(EXIT_FAILURE);
    }

    const int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        fprintf(log_file, "Eroare Setarea optiunii SO_REUSEADDR pe server socket a esuat\n");
    }
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        fprintf(log_file, "Eroare Setarea optiunii SO_REUSEPORT pe server socket a esuat\n");
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    addr.sin_family = AF_INET;///IP-ipv4
    addr.sin_port = htons(PORT);//28765
    addr.sin_addr.s_addr = INADDR_ANY;


	///bind()-> leaga socketul de un port specific
    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(log_file, "Eroare Bind server socket la a a esuat\n");
        exit(EXIT_FAILURE);
    }
	///listen() face socket-ul disponibil pentru conexiuni.
    if (listen(socket_fd, MAX_CONECTATI) < 0) {
        fprintf(log_file, "Eroare Listen pentru server socket a esuat\n");
        exit(EXIT_FAILURE);
    }

    fprintf(log_file, "Server pornit pe portul %d\n", PORT);

    while (1) {
        int *client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(socket_fd, (struct sockaddr *)&addr, &addr_len)) < 0) { ///accept() -> accepta conexiuni de la clienti
            fprintf(log_file, "Eroare Accept conexiune a esuat\n");
            free(client_socket);
            continue;	
        }

        fprintf(log_file, "Conexiune acceptata pe socket %d\n", *client_socket);

        pthread_t tid;
		///Creez thread uri pentru fiecare client
        if (pthread_create(&tid, NULL, gestiune_client, client_socket) < 0) {
            fprintf(log_file, "Nu s-a putut crea thread-ul pentru socket %d\n", *client_socket);
            free(client_socket);
        } else {
            fprintf(log_file, "Thread creat pentru client socket %d (Thread ID: %lu)\n", *client_socket, pthread_self());
            pthread_detach(tid);///permite gestionarea resurselor thread ului.
        }
    }

    close(socket_fd);
    return 0;
}