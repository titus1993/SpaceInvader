#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <errno.h>
#include <sys/shm.h> /* shm*  */


#define DELAY 300000
#define MAX_X 102
#define MAX_Y 32
#define SPACE_HORIZONTAL 21
#define SPACE_VERTICAL 7
#define FILEKEY "/bin/cat"
#define KEY 1300

#define MAXBUF 10


#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)

#else

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif

typedef struct{
  char image [6];
}personaje;

typedef struct{
  int tipo; // 0. Defensor - (1 - 4) Invasor - 5. Invasor normal - 6. Bala
  int direccion;
	int pos;
}objeto;

typedef struct{
	bool j1, j2;
	int turno;

	int vidaDefensor;
  int puntosDefensor;
  int vidaInvasor;

	int balasDefensor;
	int balasInvasor;

	objeto * invader1;
	objeto * invader2;
	objeto * invader3;
	objeto * invader4;

  int minutos;
  int segundos;
  objeto *tablero[12][14];
	objeto *balas[12][14];
}juego;

juego *datos;
int x = 0, y = 0;
int max_y = 0, max_x = 0;
int next_x = 0;
int direction = 1;
int rol = 0;

key_t Clave;
int Id_Semaforo;
key_t ClaveJ1;
int Id_SemaforoJ1;
key_t ClaveJ2;
int Id_SemaforoJ2;
struct sembuf Operacion;
//juego * datos;



void limpiarSemaforo(){

	Operacion.sem_op =  -1;
  //Si el semaforo no funciona descomenetarlo
	int valsem2 = semctl(Id_Semaforo, 0, GETVAL);
	printf("%i\n", valsem2);
	if(valsem2 > 0){
		while(valsem2 > 0){
			semop (Id_Semaforo, &Operacion, 1);
			valsem2 = semctl(Id_Semaforo, 0, GETVAL);
		}
	}

  valsem2 = semctl(Id_SemaforoJ1, 0, GETVAL);
	printf("%i\n", valsem2);
	if(valsem2 > 0){
		while(valsem2 > 0){
			semop (Id_SemaforoJ1, &Operacion, 1);
			valsem2 = semctl(Id_SemaforoJ1, 0, GETVAL);
		}
	}

  valsem2 = semctl(Id_SemaforoJ2, 0, GETVAL);
	printf("%i\n", valsem2);
	if(valsem2 > 0){
		while(valsem2 > 0){
			semop (Id_SemaforoJ2, &Operacion, 1);
			valsem2 = semctl(Id_SemaforoJ2, 0, GETVAL);
		}
	}

	Operacion.sem_op =  1;
}

void cargarSemaforo(){
  Clave = ftok ("/bin/ls", 1993);
	if (Clave == (key_t)-1)
	{
		printf("%s\n", "No puedo conseguir clave de semáforo");
		exit(0);
	}

	Id_Semaforo = semget (Clave, 1, 0600 | IPC_CREAT);

  if (Id_Semaforo == -1)
	{
		printf("%s\n", "No puedo crear semáforo");
		exit (0);
	}

  ClaveJ1 = ftok ("/bin/cat", 1994);
	if (ClaveJ1 == (key_t)-1)
	{
		printf("%s\n", "No puedo conseguir clave de semáforo");
		exit(0);
	}

	Id_SemaforoJ1 = semget (ClaveJ1, 1, 0600 | IPC_CREAT);

  if (Id_SemaforoJ1 == -1)
	{
		printf("%s\n", "No puedo crear semáforo");
		exit (0);
	}

  ClaveJ2 = ftok ("/bin/cpio", 1995);
	if (ClaveJ2 == (key_t)-1)
	{
		printf("%s\n", "No puedo conseguir clave de semáforo");
		exit(0);
	}

	Id_SemaforoJ2 = semget (ClaveJ2, 1, 0600 | IPC_CREAT);

  if (Id_SemaforoJ2 == -1)
	{
		printf("%s\n", "No puedo crear semáforo");
		exit (0);
	}

  Operacion.sem_num = 0;
	Operacion.sem_op =  1;
	Operacion.sem_flg = 0;


  //Si el semaforo no funciona descomenetarlo
  limpiarSemaforo();

  int valsem = semctl(Id_Semaforo, 0, GETVAL);//obtenemos el valor del semaforo
  if(valsem >= 2){
    printf("%s\n", "Ya hay dos jugadores, espera hasta que se desocupe el juego ;)");
    exit(0);
  }
}

void imprimirCentrado(int posX, int posY, char * cadena){
  mvprintw(posY,posX - strlen(cadena)/2,"%s",cadena);
  refresh();
}

void imprimirNormal(int posX, int posY, char * cadena){
  mvprintw(posY,posX,"%s",cadena);
  refresh();
}

void imprimirNormalEntero(int posX, int posY, int entero){
  mvprintw(posY,posX,"%i",entero);
  refresh();
}

void imprimirCentradoEntero(int posX, int posY, int entero){
  mvprintw(posY,posX,"%i",entero);
  refresh();
}

void crearMemoria(){
	key_t key = ftok(FILEKEY, KEY);
	int id_zone = shmget (key, sizeof(juego *), 0777 | IPC_CREAT);

	juego *buffer;
	buffer = (juego *)shmat (id_zone, NULL, 0);


	buffer->j1 = FALSE;
	buffer->j2 = FALSE;
	buffer->turno = 1;

	buffer->balasDefensor = 0;
	buffer->balasInvasor =0;
	buffer->vidaDefensor = 5;
	buffer->vidaInvasor = 5;
	buffer->puntosDefensor = 0;

	buffer->invader1 = malloc(sizeof(objeto));
	buffer->invader1->tipo = 1;

	buffer->invader2 = malloc(sizeof(objeto));
	buffer->invader2->tipo = 2;

	buffer->invader3 = malloc(sizeof(objeto));
	buffer->invader3->tipo = 3;

	buffer->invader4 = malloc(sizeof(objeto));
	buffer->invader4->tipo = 4;

	buffer->minutos = 0;
	buffer->segundos = 0;


	int i, j;
	//tablero de balas
	for(i = 0; i < 10; i++){
		for(j = 0; j < 14; j++){
			buffer->tablero[i][j] = NULL;
		}
	}

	j = 2;
	//for para la primera fila de invasores
	for(i = 0; i < 10; i = i + 2){
		buffer->tablero[i][j] = malloc(sizeof(objeto));
		buffer->tablero[i][j]->direccion = 1;
		buffer->tablero[i][j]->pos = 0;
		buffer->tablero[i][j]->tipo = 5;
	}

	j++;
	//segunda fila de invasores
	for(i = 1; i < 10; i = i + 2){
		buffer->tablero[i][j] = malloc(sizeof(objeto));
		buffer->tablero[i][j]->direccion = 1;
		buffer->tablero[i][j]->pos = 1;
		buffer->tablero[i][j]->tipo = 5;
	}

	j++;
	//tercera fila de invasores
	for(i = 0; i < 10; i = i + 2){
		buffer->tablero[i][j] = malloc(sizeof(objeto));
		buffer->tablero[i][j]->direccion = 1;
		buffer->tablero[i][j]->pos = 0;
		buffer->tablero[i][j]->tipo = 5;
	}

	j++;
	//cuarta fila de invasores
	for(i = 1; i < 10; i = i + 2){
		buffer->tablero[i][j] = malloc(sizeof(objeto));
		buffer->tablero[i][j]->direccion = 1;
		buffer->tablero[i][j]->pos = 1;
		buffer->tablero[i][j]->tipo = 5;
	}

	//primer super invasor
	i = 0;
	j = 2;
	buffer->tablero[i][j] = malloc(sizeof(objeto));
	buffer->tablero[i][j]->direccion = 1;
	buffer->tablero[i][j]->pos = 0;
	buffer->tablero[i][j]->tipo = 1;

	//segundo super invasor
	i = 3;
	j = 3;
	buffer->tablero[i][j] = malloc(sizeof(objeto));
	buffer->tablero[i][j]->direccion = 1;
	buffer->tablero[i][j]->pos = 1;
	buffer->tablero[i][j]->tipo = 2;

	//tercer super invasor
	i = 2;
	j = 4;
	buffer->tablero[i][j] = malloc(sizeof(objeto));
	buffer->tablero[i][j] = malloc(sizeof(objeto));
	buffer->tablero[i][j]->direccion = 1;
	buffer->tablero[i][j]->pos = 0;
	buffer->tablero[i][j]->tipo = 3;

	//cuarto super invasor
	i = 5;
	j = 5;
	buffer->tablero[i][j] = malloc(sizeof(objeto));
	buffer->tablero[i][j]->direccion = 1;
	buffer->tablero[i][j]->pos = 1;
	buffer->tablero[i][j]->tipo = 4;

	//tablero de balas
	for(i = 0; i < 10; i++){
		for(j = 0; j < 14; j++){
			buffer->balas[i][j] = NULL;
		}
	}

	datos = buffer;

}

void leerMemoria(){
	/* Key to shared memory */
   key_t key = ftok(FILEKEY, KEY);
   /* we create the shared memory */
   int id_zone = shmget (key, sizeof(juego *), 0777);
   datos = (juego *)shmat (id_zone, NULL, 0);
}

void jugar(){
	while(1){
		clear();
		panelSuperior();
		panelInferior();
		panelCentral();
		usleep(100000);
	}
}

void iniciarJuego(){
	if(rol == 1){
		crearMemoria();
	}
}

void iniciarPantalla(){
  initscr();//iniciamos la pantalla
  start_color();//iniciamos la opcion de colores
  init_pair(1, COLOR_CYAN, COLOR_BLACK);//establecemos el primer par de colores
  bkgd(COLOR_PAIR(1));//establecemos el par de colores que usaremos en la pantalla
	attron(COLOR_PAIR(1));
  noecho();
  curs_set(FALSE);//ocultamos el cursor
}

void pantallaBienvenida(){
  clear();Operacion.sem_op = 1;
  imprimirCentrado(MAX_X/2, MAX_Y/2 - 5, "SPACE INVADERS \\-.-/");
  imprimirCentrado(MAX_X/2, MAX_Y/2, "Bienvenido");
  imprimirCentrado(MAX_X/2, MAX_Y/2 + 5, "presiona enter para continuar");
  imprimirCentrado(MAX_X/2, MAX_Y/2 + 14, "Marvin Emmanuel Pivaral Orellana - 201213587");

  while (1) {
    int c = getchar();
    if(c == 13){
      break;
    }
  }
}

void esperarJugador(){
  semop (Id_Semaforo, &Operacion, 1);//aumentamos el semaforo

  int punto = 0;
  int valsem = semctl(Id_Semaforo, 0, GETVAL);//obtenemos el valor del semaforo
  while(valsem != 2){//esperamos al segundo jugador
    switch (punto) {
      case 1:
      imprimirCentrado(MAX_X/2, MAX_Y - 1,"esperando al segundo jugador .  ");
      break;

      case 2:
      imprimirCentrado(MAX_X/2, MAX_Y - 1,"esperando al segundo jugador .. ");
      break;

      case 3:
      imprimirCentrado(MAX_X/2, MAX_Y - 1,"esperando al segundo jugador ...");
      break;

      default:
      imprimirCentrado(MAX_X/2, MAX_Y - 1,"esperando al segundo jugador    ");
      break;
    }
    usleep(500000);
    punto++;
    if(punto == 4) punto = 0;
		valsem = semctl(Id_Semaforo, 0, GETVAL);
		//printf("%i\n", valsem);
	}
}

void pantallaSeleccion(){
  clear();
  int pos1Y = (MAX_Y/2)/2;
  int pos2Y = (MAX_Y/2) + pos1Y;
  imprimirCentrado(MAX_X/2, pos1Y - 3, "------------------------------");
  imprimirCentrado(MAX_X/2, pos1Y - 2, "|                            |");
  imprimirCentrado(MAX_X/2, pos1Y - 1, "|                            |");
  imprimirCentrado(MAX_X/2, pos1Y,     "|          Jugador 1         |");
  imprimirCentrado(MAX_X/2, pos1Y + 1, "|          DEFENSOR          |");
  imprimirCentrado(MAX_X/2, pos1Y + 2, "|          presiona 1        |");
  imprimirCentrado(MAX_X/2, pos1Y + 3, "|                            |");
  imprimirCentrado(MAX_X/2, pos1Y + 4, "|                            |");
  imprimirCentrado(MAX_X/2, pos1Y + 5, "------------------------------");

  imprimirCentrado(MAX_X/2, pos2Y - 3, "------------------------------");
  imprimirCentrado(MAX_X/2, pos2Y - 2, "|                            |");
  imprimirCentrado(MAX_X/2, pos2Y - 1, "|                            |");
  imprimirCentrado(MAX_X/2, pos2Y,     "|          Jugador 2         |");
  imprimirCentrado(MAX_X/2, pos2Y + 1, "|          INVASOR           |");
  imprimirCentrado(MAX_X/2, pos2Y + 2, "|          presiona 2        |");
  imprimirCentrado(MAX_X/2, pos2Y + 3, "|                            |");
  imprimirCentrado(MAX_X/2, pos2Y + 4, "|                            |");
  imprimirCentrado(MAX_X/2, pos2Y + 5, "------------------------------");

  int c = getchar();
  if(c == 49){
    int valsem = semctl(Id_SemaforoJ1, 0, GETVAL);//obtenemos el valor del semaforo
    if(valsem >= 1){
      imprimirCentrado(MAX_X/2, MAX_Y/2, "Este rol ya ha sido elegido escoge otro rol");
      usleep(500000);
      pantallaSeleccion();
    }else{
      rol = 1;
      Operacion.sem_op = 1;
      semop (Id_SemaforoJ1, &Operacion, 1);//aumentamos el semaforo
      imprimirCentrado(MAX_X/2, MAX_Y/2, "Has escogido ser un DEFENSOR");
			crearMemoria();
			esperarJugador();
      Operacion.sem_op = -1;
      semop (Id_SemaforoJ1, &Operacion, 1);//aumentamos el semaforo
      Operacion.sem_op = 1;
    }
  }else if(c == 50){
    int valsem = semctl(Id_SemaforoJ2, 0, GETVAL);//obtenemos el valor del semaforo
    if(valsem >= 1){
      imprimirCentrado(MAX_X/2, MAX_Y/2, "Este rol ya ha sido elegido escoge otro rol");
      usleep(500000);
      pantallaSeleccion();
    }else{
      rol = 2;
      Operacion.sem_op = 1;
      semop (Id_SemaforoJ2, &Operacion, 1);//aumentamos el semaforo
      imprimirCentrado(MAX_X/2, MAX_Y/2, "Has escogido ser un INVASOR");
      esperarJugador();
			leerMemoria();
      Operacion.sem_op = -1;
      semop (Id_SemaforoJ2, &Operacion, 1);//aumentamos el semaforo
      Operacion.sem_op = 1;
    }
  }else{
    imprimirCentrado(MAX_X/2, MAX_Y/2, "Presiona 1 o 2 para escoger un bando");
    usleep(500000);
    pantallaSeleccion();
  }
}

void panelSuperior(){
	char str[2];
	sprintf(str, "%d", datos->vidaInvasor);
	char texto[7];
	strcpy(texto, "Vida: ");
	strcat(texto, str);
	int posT = 2;
	imprimirCentrado(21/2, posT, texto);
	int i;
	for(i = 0; i < 102; i++){
		imprimirCentrado(i, posT+2, "-");
	}
	imprimirCentrado(50, posT, "INVASOR");
	imprimirCentrado(101 - 21/2, posT, "TIEMPO: 00:00");

	i = 6;
	if(datos->segundos >= 10){
		i = 5;
	}else{
		imprimirCentradoEntero(101 - 21/2 + i - 1, posT, 0);
	}

	imprimirCentradoEntero(101 - 21/2 + i, posT, datos->segundos);

	i = 3;
	if(datos->minutos >= 10){
		i = 2;
	}else{
			imprimirCentradoEntero(101 - 21/2 + i - 1, posT, 0);
	}
	imprimirCentradoEntero(101 - 21/2 + i, posT, datos->minutos);
}

void panelInferior(){
	char str[2];
	sprintf(str, "%d", datos->vidaInvasor);
	char texto[7];
	strcpy(texto, "Vida: ");
	strcat(texto, str);
	int posT = 29;
	imprimirCentrado(21/2, posT, texto);
	int i;
	for(i = 0; i < 102; i++){
		imprimirCentrado(i, posT-2, "-");
	}
	imprimirCentrado(50, posT, "DEFENSOR");
	imprimirCentrado(101 - 21/2, posT, "PUNTEO: 000");

	i = 5;
	if(datos->puntosDefensor >= 10){
		i = 4;
	}

	if(datos->puntosDefensor == 100){
		i = 3;
	}
	imprimirCentradoEntero(101 - 21/2 + i, posT, datos->puntosDefensor);
}

void panelCentral(){
	int posX = 21;
	int posY = 7;
	int i, j;

	for(i = 0; i < 12; i++){
		 for(j = 0; j < 14; j++){
			 if(datos->tablero[i][j] != NULL){
				 switch(datos->tablero[i][j]->tipo){
					 case 1:
					 	imprimirNormal(i*5 + posX, j + posY, "\\-1-/");
					 	break;

					 case 2:
					 	imprimirNormal(i*5 + posX, j + posY, "\\-2-/");
					 	break;

					 case 3:
					 	imprimirNormal(i*5 + posX, j + posY, "\\-3-/");
					 	break;

					 case 4:
					 	imprimirNormal(i*5 + posX, j + posY, "\\-4-/");
					 	break;

					 case 5:
						 imprimirNormal(i*5 + posX, j + posY, "\\-.-/");
						 break;
				 }
				 //imprimirNormal(i*5 + posX, j + posY, "h");
			 }
		 }
	}
}

void panelIzquierdo(){

}

void panelDerecho(){

}

int main(int argc, char *argv[]) {
  cargarSemaforo();
  iniciarPantalla();//configuraciones iniciales de la pantalla
  pantallaBienvenida();
  pantallaSeleccion();
	iniciarJuego();
	jugar();

	/*personaje a;
  strcpy(a.image, "\\-.-/");


  // Global var `stdscr` is created by the call to `initscr()`
  while(1) {
    getmaxyx(stdscr, max_y, max_x);
    clear();
    mvprintw(y, x, a.image);
    refresh();

    usleep(DELAY);

    next_x = x + direction;

    if (next_x >= MAX_X || next_x < 0) {
      direction*= -1;
    } else {
      x+= direction;
    }
  }*/

  endwin();
}
