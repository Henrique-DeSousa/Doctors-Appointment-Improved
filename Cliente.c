#include<ctype.h>
#include "Consulta.h"
//#define IPC_KEY 0x00a93540

Consulta c;
int msg_id, msgStats, pid;

//Treating the messages
void msgTreat(){
	msg_id = msgget( IPC_KEY, IPC_CREAT | 0600);
	exit_on_error(msg_id, "Error on connection");

	msgStats = msgsnd(msg_id, &c, sizeof(c), 0);
	exit_on_error(msgStats, "Error on sending");
}

//C7 armar o sinal SIGINT 
void signalTreat(){
	c.tp = c.data.pid_consult;
	c.data.status = 5;
	printf("\nCanceling on user request!\n");
	msgStats = msgsnd(msg_id, &c, sizeof(c), 0);
	exit_on_error(msgStats, "Error sending");
	exit(0);
}

//C3,C4,C5,C6
void waitMsg(){
	int isIn = -1;

	while(1){
		//C3
		msg_id = msgget( IPC_KEY , IPC_CREAT | 0600 );
		exit_on_error (msg_id, "Error connecting. 78");

		msgStats = msgrcv( msg_id, &c, sizeof(c), getpid(), 0);
		exit_on_error (msgStats, "Error recieving. 40");	
		
		switch(c.data.status){
			//C4 Iniciated appointment
			case 2:
				printf("\nAppointment iniciated for the process: %d\n", c.data.pid_consult);
				isIn = 1;
			break;
			
			case 3:
				//C5 Terminada Consulta
				if(isIn > 0){
						printf("\nAppointment concluded for the process: %d\n", c.data.pid_consult);
						isIn = -1;
						exit(0);
				}else{
						exit_on_error(isIn, "Error: No iniciated!");
				}

			break;

			//C6 Refused appointment
			case 4:
				printf("\nAppointment not possible for the process: %d!\n", c.data.pid_consult);
				exit(0);
			break;
		}
	}
}

//C1 & C2	
void typeAppoint(){
	//Choosing the type of appointment (as seen in Project II)
	printf ("Select the type of your visit: \n1- Clinica Geral\n2- COVID19\n3- Urgente\n");
	scanf("%d", &c.data.type);
	if(c.data.type > 3 || c.data.type <= 0){ // if value higher than what we provide, breaks
		perror("Invalid selection!\n");
		exit(1);
	}
	//read the description 
	printf("Describe your symptoms: ");
	scanf("%s", &c.data.description);
	for(int i = 0, s = strlen(c.data.description); i < s; i++){
		if(!isalpha(c.data.description[i])){
			perror("Please describe your symptoms!\n");
			exit(1);
		}
	}
	c.data.pid_consult = getpid();
	c.tp = 1;
	c.data.status = 1;
	printf("Booked an appointment of type: %d, with the following symptoms: %s.\n", c.data.type, c.data.description);
	msgTreat();
}

int main(){
	signal(SIGINT, signalTreat);//C7
	typeAppoint();
	waitMsg();
}
