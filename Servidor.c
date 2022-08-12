#include <sys/shm.h>
#include <sys/wait.h>
#include "Consulta.h"
#define duracao 10

Consulta* lc; // apontador para a memoria partilhada
int *lost, *a , *b , *c; // apontador para os varios tipos

Consulta cs;
Consulta lista_consultas[10]; 

int msgStats, msg_id, semStat, sem_ID, chld, chpid;
int count, t = 0;


void semCreat(){
    sem_ID = semget ( IPC_KEY, 1, IPC_CREAT | 0666 );
    exit_on_error (sem_ID, "Criação/Ligação");

    printf ("Criado semáforo com id %d\n", sem_ID);

    semStat = semctl ( sem_ID, 0, SETVAL, 1);
    exit_on_error (semStat, "Inicialização");
}

void initSmh(Consulta* lc){
    for(int i = 0; i < 10; i++){
        lc[i].data.type = -1;
    }
    *a = 0;
    *b = 0;
    *c = 0;
    *lost = 0;
    }

void signalTreat(int signal){
    switch(signal){
        case 2:
            printf("\e[1;1H\e[2J");
            printf("\nType 1: %d", *a);
            printf("\nType 2: %d", *b);
            printf("\nType 3: %d", *c);
            printf("\n Losts: %d\n", *lost);
        break;

        case 14:
            t = 1;
        break;
    }
}


void sendMsg(int numb){
    cs.tp = cs.data.pid_consult;
    cs.data.status = numb;

    msg_id = msgget( IPC_KEY, IPC_CREAT | 0600);
	exit_on_error(msg_id, "Error on connection. 60");

    msgStats = msgsnd(msg_id, &cs, sizeof(cs), 0);
	exit_on_error(msg_id, "Error on sending. 74");
}

//checks to see if there's any room available to be booked
int check(){
for(int i = 0; i < 10; i++){
	if(lc[i].data.type == -1){ // check for a free position on the rooms
		count = i; // if it found any, count is equal to that position
		return 1;
        }
	}
return 0;
}

// checks, using the check() function for any room available
int vacancy(){
    if (check() != 1){ // if the previous function doesn't find any free position
	    *lost = *lost + 1; // and increments the lost int
	    return 1;
	}
    return 0;
}

void addApp(){
    semStat = semop ( sem_ID, &DOWN, 1 );
    lc[count] = cs; // it adds the current appointment to that room
    printf("\nAppointment booked to room: %d\n", count); //and prints that it found a room and gives the room's number
    semStat = semop ( sem_ID, &UP, 1 );
}

void incrType(){
    semStat = semop ( sem_ID, &DOWN, 1 );
    switch(cs.data.type){
        case 1:
            *a = (*a)+1;
        break;
        case 2:
            *b = (*b)+1;
        break;
        case 3:
            *c = (*c)+1 ;
        break;
    }
    semStat = semop ( sem_ID, &UP, 1 );
}

void waitMsg(){

//S2 wait for  message 

 while(1){
        msg_id = msgget( IPC_KEY , IPC_CREAT | 0600 );
        exit_on_error (msg_id, "Error 46");

        msgStats = msgrcv( msg_id, &cs, sizeof(cs) ,1 ,0);
        exit_on_error (msgStats, "Error 404");

    //S3
     if(cs.data.status == 1){
        printf("\nA new appointment of the type: %d\nWith the following: %s\nWith the ID: %d\n",cs.data.type, cs.data.description, cs.data.pid_consult);
        //S3.3.1 //if it finds an oppen room then:
        if(vacancy() != 1){
            chld = fork();
                if((chld) == 0){ // child code
                    chpid = getpid();
                    addApp(); //adds the appointment to that room
                    sendMsg(2); //sends a message of type 2
                    alarm(duracao); //sleeps for X seconds ( 10 in our case )
                        while(t == 0){
                            msgStats = msgrcv( msg_id, &cs, sizeof(cs) ,cs.data.pid_consult , IPC_NOWAIT );
                            if(msgStats < 0){
                            }else{
                            printf("\nAppointment cancelled for ID: %d\n",cs.tp);
                            semStat = semop ( sem_ID, &DOWN, 1 );//semafro down
                            lc[count].data.type = -1;
                            semStat = semop ( sem_ID, &UP, 1 ); //semafro up
                            exit(0);
                            }
                        }
                    sendMsg(3); //sends another message but of type 3
                    semStat = semop ( sem_ID, &DOWN, 1 );//semafro down
                    lc[count].data.type = -1; // then resets that room to -1 when it's done with it.
                    semStat = semop ( sem_ID, &UP, 1 ); //semafro up
                    incrType(); //increments the type
                    exit(0);
                }
            waitpid(chpid, NULL, IPC_NOWAIT);
        }else{
            cs.tp = cs.data.pid_consult;
            cs.data.status = 4;
            printf("Appointment list full!\n");
            sendMsg(4);
        }
        }
    }
}


//S1 everything to do with the memory 
void shrdMem(){
    char needSrtSMH = 0;
    int mem_id = shmget(IPC_KEY, 10*(sizeof(Consulta)) + (5*sizeof(int)), IPC_CREAT | IPC_EXCL | 0666);
    
    if(mem_id > 0){
        needSrtSMH = 1;
    }else{ 
        mem_id = shmget(IPC_KEY, 10*(sizeof(Consulta)) + (5*sizeof(int)), 0);
        exit_on_error(mem_id, "Error 12: Openning");
    }

    lc = (Consulta*) shmat(mem_id, NULL, 0);
    exit_on_null(lc, "Error 21: lc");
    lost = (int*)((void*)lc + 10 + sizeof(Consulta));
    a = lost+1;
    b = a+1;
    c = b+1;

    if (needSrtSMH){
        initSmh(lc);
        printf("\nShared memory ID: %d\n", mem_id);
    }
}

int main(){
    semCreat();
    signal(SIGALRM, signalTreat);
    signal(SIGINT, signalTreat);
    shrdMem();
    waitMsg();
}