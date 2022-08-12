#ifndef __CONSULTA_H__
#define __CONSULTA_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define IPC_KEY 0x00a93540
#define exit_on_error(s,m) if ( s < 0 ) { perror(m); exit(1); }
#define exit_on_null(s,m) if (s==NULL) { perror(m); exit(1); }

struct sembuf UP = { 0, 1, 0 } ;
struct sembuf DOWN = { 0, -1, 0 };

typedef struct {
	long tp;
	struct {
		int type; // Tipo de Consulta: 1-Normal, 2-COVID19, 3-Urgente
		char description[100]; // Descrição da Consulta
		int pid_consult; // PID do processo que quer fazer a consulta
		int status; // Estado da consulta: 1-Pedido, 2-Iniciada,
	} data; // 3-Terminada, 4-Recusada, 5-Cancelada
}Consulta;



#endif