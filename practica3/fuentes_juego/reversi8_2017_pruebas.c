

// Tama�o del tablero
#define DIM   8

// Valores que puede devolver la funci�n patron_volteo())
#define PATRON_ENCONTRADO    1
#define NO_HAY_PATRON        0

// Estados de las casillas del tablero
#define CASILLA_VACIA 0
#define FICHA_BLANCA  1
#define FICHA_NEGRA   2

// candidatas: indica las posiciones a explorar
// Se usa para no explorar todo el tablero innecesariamente
// Sus posibles valores son NO, SI, CASILLA_OCUPADA
#define NO               0
#define SI               1
#define CASILLA_OCUPADA  2

enum maquina_est_juego{
	inicial_juego	= 0,
	espera_fila 	= 1,
	aumenta_fila 	= 2,
	espera_col 		= 3,
	aumenta_col 	= 4
};

volatile int estado_juego = 0;

volatile int cuenta_fila = 0;
volatile int cuenta_col = 0;

#include "button.h"
#include "led.h"

/////////////////////////////////////////////////////////////////////////////
// TABLAS AUXILIARES
// declaramos las siguientes tablas como globales para que sean m�s f�ciles visualizarlas en el simulador
// __attribute__ ((aligned (8))): specifies a minimum alignment for the variable or structure field, measured in bytes, in this case 8 bytes

static const char __attribute__ ((aligned (8))) tabla_valor[8][8] =
{
    {8,2,7,3,3,7,2,8},
    {2,1,4,4,4,4,1,2},
    {7,4,6,5,5,6,4,7},
    {3,4,5,0,0,5,4,3},
    {3,4,5,0,0,5,4,3},
    {7,4,6,5,5,6,4,7},
    {2,1,4,4,4,4,1,2},
    {8,2,7,3,3,7,2,8}
};


// Tabla de direcciones. Contiene los desplazamientos de las 8 direcciones posibles
const char vSF[DIM] = {-1,-1, 0, 1, 1, 1, 0,-1};
const char vSC[DIM] = { 0, 1, 1, 1, 0,-1,-1,-1};

//////////////////////////////////////////////////////////////////////////////////////
// Variables globales que no deber�an serlo
// tablero, fila, columna y ready son varibles que se deber�an definir como locales dentro de reversi8.
// Sin embargo, las hemos definido como globales para que sea m�s f�cil visualizar el tablero y las variables en la memoria
//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
// Tablero sin inicializar
////////////////////////////////////////////////////////////////////
char __attribute__ ((aligned (8))) tablero[DIM][DIM] = {
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA},
	        {CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA,CASILLA_VACIA}
	    };

  ////////////////////////////////////////////////////////////////////
     // VARIABLES PARA INTERACCIONAR CON LA ENTRADA SALIDA
     // Pregunta: �hay que hacer algo con ellas para que esto funcione bien?
     // (por ejemplo a�adir alguna palabra clave para garantizar que la sincronizaci�n a trav�s de esa variable funcione)
  char fila=0, columna=0, ready = 0;



// extern int patron_volteo(char tablero[][8], int *longitud,char f, char c, char SF, char SC, char color);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0 indica CASILLA_VACIA, 1 indica FICHA_BLANCA y 2 indica FICHA_NEGRA
// pone el tablero a cero y luego coloca las fichas centrales.
void init_table(char tablero[][DIM], char candidatas[][DIM])
{
    int i, j;

    for (i = 0; i < DIM; i++)
    {
        for (j = 0; j < DIM; j++)
            tablero[i][j] = CASILLA_VACIA;
    }
    tablero[3][3] = FICHA_BLANCA;
    tablero[4][4] = FICHA_BLANCA;
    tablero[3][4] = FICHA_NEGRA;
    tablero[4][3] = FICHA_NEGRA;

    candidatas[3][3] = CASILLA_OCUPADA;
    candidatas[4][4] = CASILLA_OCUPADA;
    candidatas[3][4] = CASILLA_OCUPADA;
    candidatas[4][3] = CASILLA_OCUPADA;

    // casillas a explorar:
    candidatas[2][2] = SI;
    candidatas[2][3] = SI;
    candidatas[2][4] = SI;
    candidatas[2][5] = SI;
    candidatas[3][2] = SI;
    candidatas[3][5] = SI;
    candidatas[4][2] = SI;
    candidatas[4][5] = SI;
    candidatas[5][2] = SI;
    candidatas[5][3] = SI;
    candidatas[5][4] = SI;
    candidatas[5][5] = SI;
}

////////////////////////////////////////////////////////////////////////////////
// Espera a que ready valga 1.
// CUIDADO: si el compilador coloca esta variable en un registro, no funcionar�.
// Hay que definirla como "volatile" para forzar a que antes de cada uso la cargue de memoria 

void esperar_mov()
{
 //   while (*ready == 0) {};  // bucle de espera de respuestas hasta que el se modifique el valor de ready (hay que hacerlo manualmente)
 //   *ready = 0;  //una vez que pasemos el bucle volvemos a fijar ready a 0;
	int eleccion_hecha = 0;
	while(eleccion_hecha==0){
	  switch(estado_juego){
	    case 0:
	      if(izq_pulsado == 1 || dech_pulsado == 1){
  			  izq_pulsado = 0;
  			  dech_pulsado = 0;
	    	  eleccion_hecha = 0;
  			  D8Led_symbol(0x000f);
  			  estado_juego = espera_fila;
	      }
			  break;
	    case 1:
	      if(izq_pulsado == 1){
	    	  	  izq_pulsado = 0;
	    	  	  cuenta_fila = 0;
				  D8Led_symbol(cuenta_fila & 0x000f);
				  estado_juego = aumenta_fila;
	      }
	      break;
	    case 2:
	      if(izq_pulsado == 1){
	    	  izq_pulsado = 0;
	    	  if(cuenta_fila < 7){
  					cuenta_fila++;
  				}else{
  					cuenta_fila = 0;
  				}
  				D8Led_symbol(cuenta_fila & 0x000f);
	      }else if(dech_pulsado == 1){
	    	  	dech_pulsado = 0;
  				D8Led_symbol(0x000c);
  				estado_juego = espera_col;
			  }
	      break;
	    case 3:
  			if (izq_pulsado == 1) {
  				izq_pulsado = 0;
  				cuenta_col = 0;
  				D8Led_symbol(cuenta_col & 0x000f);
  				estado_juego = aumenta_col;
  			}
  			break;
  		case 4:
  			if (izq_pulsado == 1) {
  				izq_pulsado = 0;
  				if(cuenta_col < 7){
  					cuenta_col++;
  				}else{
  					cuenta_col = 0;
  				}
  				D8Led_symbol(cuenta_col & 0x000f);
  			} else if (dech_pulsado == 1) {
  				dech_pulsado = 0;
  				eleccion_hecha = 1;
  				estado_juego = inicial_juego;
  			}
  			break;
  		default:
  			break;	
	  }
	}

	  eleccion_hecha = 0;
	  

}

////////////////////////////////////////////////////////////////////////////////
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IMPORTANTE: AL SUSTITUIR FICHA_VALIDA() Y PATRON_VOLTEO()
// POR RUTINAS EN ENSAMBLADOR HAY QUE RESPETAR LA MODULARIDAD.
// DEBEN SEGUIR SIENDO LLAMADAS A FUNCIONES Y DEBEN CUMPLIR CON EL ATPCS
// (VER TRANSPARENCIAS Y MATERIAL DE PRACTICAS):
//  - DEBEN PASAR LOS PARAMETROS POR LOS REGISTROS CORRESPONDIENTES
//  - GUARDAR EN PILA SOLO LOS REGISTROS QUE TOCAN
//  - CREAR UN MARCO DE PILA TAL Y COMO MUESTRAN LAS TRANSPARENCIAS
//    DE LA ASIGNATURA (CON EL PC, FP, LR,....)
//  - EN EL CASO DE LAS VARIABLES LOCALES, SOLO HAY QUE APILARLAS
//    SI NO SE PUEDEN COLOCAR EN UN REGISTRO.
//    SI SE COLOCAN EN UN REGISTRO NO HACE FALTA
//    NI GUARDARLAS EN PILA NI RESERVAR UN ESPACIO EN LA PILA PARA ELLAS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
////////////////////////////////////////////////////////////////////////////////
// Devuelve el contenido de la posici�n indicadas por la fila y columna actual.
// Adem�s informa si la posici�n es v�lida y contiene alguna ficha.
// Esto lo hace por referencia (en *posicion_valida)
// Si devuelve un 0 no es v�lida o est� vacia.
char ficha_valida(char tablero[][DIM], char f, char c, int *posicion_valida)
{
    char ficha;

    // ficha = tablero[f][c];
    // no puede accederse a tablero[f][c]
    // ya que alg�n �ndice puede ser negativo

    if ((f < DIM) && (f >= 0) && (c < DIM) && (c >= 0) && (tablero[f][c] != CASILLA_VACIA))
    {
        *posicion_valida = 1;
        ficha = tablero[f][c];
    }
    else
    {
        *posicion_valida = 0;
        ficha = CASILLA_VACIA;
    }
    return ficha;
}


////////////////////////////////////////////////////////////////////////////////
// Devuelve el contenido de la posici�n indicadas por la fila y columna actual.
// Adem�s informa si la posici�n es v�lida y contiene alguna ficha.
// Esto lo hace por referencia (en *posicion_valida)
// Si devuelve un 0 no es v�lida o est� vacia.
extern char ficha_valida_arm(char tablero[][DIM], char f, char c, int *posicion_valida);

////////////////////////////////////////////////////////////////////////////////
// Devuelve el contenido de la posici�n indicadas por la fila y columna actual.
// Adem�s informa si la posici�n es v�lida y contiene alguna ficha.
// Esto lo hace por referencia (en *posicion_valida)
// Si devuelve un 0 no es v�lida o est� vacia.
extern char ficha_valida_thumb(char tablero[][DIM], char f, char c, int *posicion_valida);


////////////////////////////////////////////////////////////////////////////////
// La funci�n patr�n volteo es una funci�n recursiva que busca el patr�n de volteo
// (n fichas del rival seguidas de una ficha del jugador actual) en una direcci�n determinada
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// la funci�n devuelve PATRON_ENCONTRADO (1) si encuentra patr�n y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un par�metro por referencia. Sirve para saber la longitud del patr�n que se est� analizando. Se usa para saber cuantas fichas habr�a que voltear
extern int patron_volteo_arm_c(char tablero[][DIM], int *longitud, char FA, char CA, char SF, char SC, char color);

////////////////////////////////////////////////////////////////////////////////
// La funci�n patr�n volteo es una funci�n recursiva que busca el patr�n de volteo
// (n fichas del rival seguidas de una ficha del jugador actual) en una direcci�n determinada
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// la funci�n devuelve PATRON_ENCONTRADO (1) si encuentra patr�n y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un par�metro por referencia. Sirve para saber la longitud del patr�n que se est� analizando. Se usa para saber cuantas fichas habr�a que voltear
extern int patron_volteo_arm_thumb(char tablero[][DIM], int *longitud, char FA, char CA, char SF, char SC, char color);

////////////////////////////////////////////////////////////////////////////////
// La funci�n patr�n volteo es una funci�n recursiva que busca el patr�n de volteo
// (n fichas del rival seguidas de una ficha del jugador actual) en una direcci�n determinada
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// la funci�n devuelve PATRON_ENCONTRADO (1) si encuentra patr�n y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un par�metro por referencia. Sirve para saber la longitud del patr�n que se est� analizando. Se usa para saber cuantas fichas habr�a que voltear
extern int patron_volteo_arm_arm(char tablero[][DIM], int *longitud, char FA, char CA, char SF, char SC, char color);


////////////////////////////////////////////////////////////////////////////////
// La funci�n patr�n volteo es una funci�n recursiva que busca el patr�n de volteo 
// (n fichas del rival seguidas de una ficha del jugador actual) en una direcci�n determinada
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// la funci�n devuelve PATRON_ENCONTRADO (1) si encuentra patr�n y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un par�metro por referencia. Sirve para saber la longitud del patr�n que se est� analizando. Se usa para saber cuantas fichas habr�a que voltear
int patron_volteo_c_c(char tablero[][DIM], int *longitud, char FA, char CA, char SF, char SC, char color)
{
    int posicion_valida; // indica si la posici�n es valida y contiene una ficha de alg�n jugador
    int patron; //indica si se ha encontrado un patr�n o no
    char casilla;   // casilla es la casilla que se lee del tablero
    FA = FA + SF;
    CA = CA + SC;

    casilla = ficha_valida(tablero, FA, CA, &posicion_valida);
    // mientras la casilla est� en el tablero, no est� vac�a,
    // y es del color rival seguimos buscando el patron de volteo
    if ((posicion_valida == 1) &&(casilla!= color))
    {
        *longitud = *longitud + 1;
        patron = patron_volteo_c_c(tablero, longitud, FA, CA, SF, SC, color);
        return patron;
    }
    // si la ultima posici�n era v�lida y la ficha es del jugador actual,
    // entonces hemos encontrado el patr�n
    else if ((posicion_valida== 1) && (casilla== color))
    {
        if (*longitud > 0) // longitud indica cuantas fichas hay que voltear
            {
            return PATRON_ENCONTRADO; // si hay que voltear una ficha o m�s hemos encontrado el patr�n
            //printf("PATRON_ENCONTRADO \n");
            }
        else {
            return NO_HAY_PATRON; // si no hay que voltear no hay patr�n
            //printf("NO_HAY_PATRON \n");
            }
    }
    // en caso contrario es que no hay patr�n
    else
    {
        return NO_HAY_PATRON;
        //printf("NO_HAY_PATRON \n");
    }
}
////////////////////////////////////////////////////////////////////////////////
// La funci�n patr�n volteo es una funci�n recursiva que busca el patr�n de volteo
// (n fichas del rival seguidas de una ficha del jugador actual) en una direcci�n determinada
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// la funci�n devuelve PATRON_ENCONTRADO (1) si encuentra patr�n y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un par�metro por referencia. Sirve para saber la longitud del patr�n que se est� analizando. Se usa para saber cuantas fichas habr�a que voltear
int patron_volteo_c_arm(char tablero[][DIM], int *longitud, char FA, char CA, char SF, char SC, char color)
{
    int posicion_valida; // indica si la posici�n es valida y contiene una ficha de alg�n jugador
    int patron; //indica si se ha encontrado un patr�n o no
    char casilla;   // casilla es la casilla que se lee del tablero
    FA = FA + SF;
    CA = CA + SC;

    casilla = ficha_valida_arm(tablero, FA, CA, &posicion_valida);
    // mientras la casilla est� en el tablero, no est� vac�a,
    // y es del color rival seguimos buscando el patron de volteo
    if ((posicion_valida == 1) &&(casilla!= color))
    {
        *longitud = *longitud + 1;
        patron = patron_volteo_c_arm(tablero, longitud, FA, CA, SF, SC, color);
        return patron;
    }
    // si la ultima posici�n era v�lida y la ficha es del jugador actual,
    // entonces hemos encontrado el patr�n
    else if ((posicion_valida== 1) && (casilla== color))
    {
        if (*longitud > 0) // longitud indica cuantas fichas hay que voltear
            {
            return PATRON_ENCONTRADO; // si hay que voltear una ficha o m�s hemos encontrado el patr�n
            //printf("PATRON_ENCONTRADO \n");
            }
        else {
            return NO_HAY_PATRON; // si no hay que voltear no hay patr�n
            //printf("NO_HAY_PATRON \n");
            }
    }
    // en caso contrario es que no hay patr�n
    else
    {
        return NO_HAY_PATRON;
        //printf("NO_HAY_PATRON \n");
    }
}
////////////////////////////////////////////////////////////////////////////////
// La funci�n patr�n volteo es una funci�n recursiva que busca el patr�n de volteo
// (n fichas del rival seguidas de una ficha del jugador actual) en una direcci�n determinada
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// la funci�n devuelve PATRON_ENCONTRADO (1) si encuentra patr�n y NO_HAY_PATRON (0) en caso contrario
// FA y CA son la fila y columna a analizar
// longitud es un par�metro por referencia. Sirve para saber la longitud del patr�n que se est� analizando. Se usa para saber cuantas fichas habr�a que voltear
int patron_volteo_c_thumb(char tablero[][DIM], int *longitud, char FA, char CA, char SF, char SC, char color)
{
    int posicion_valida; // indica si la posici�n es valida y contiene una ficha de alg�n jugador
    int patron; //indica si se ha encontrado un patr�n o no
    char casilla;   // casilla es la casilla que se lee del tablero
    FA = FA + SF;
    CA = CA + SC;

    casilla = ficha_valida_thumb(tablero, FA, CA, &posicion_valida);
    // mientras la casilla est� en el tablero, no est� vac�a,
    // y es del color rival seguimos buscando el patron de volteo
    if ((posicion_valida == 1) &&(casilla!= color))
    {
        *longitud = *longitud + 1;
        patron = patron_volteo_c_thumb(tablero, longitud, FA, CA, SF, SC, color);
        return patron;
    }
    // si la ultima posici�n era v�lida y la ficha es del jugador actual,
    // entonces hemos encontrado el patr�n
    else if ((posicion_valida== 1) && (casilla== color))
    {
        if (*longitud > 0) // longitud indica cuantas fichas hay que voltear
            {
            return PATRON_ENCONTRADO; // si hay que voltear una ficha o m�s hemos encontrado el patr�n
            //printf("PATRON_ENCONTRADO \n");
            }
        else {
            return NO_HAY_PATRON; // si no hay que voltear no hay patr�n
            //printf("NO_HAY_PATRON \n");
            }
    }
    // en caso contrario es que no hay patr�n
    else  
    {
        return NO_HAY_PATRON;
        //printf("NO_HAY_PATRON \n");
    }
}
////////////////////////////////////////////////////////////////////////////////
// voltea n fichas en la direcci�n que toque
// SF y SC son las cantidades a sumar para movernos en la direcci�n que toque
// color indica el color de la pieza que se acaba de colocar
// FA y CA son la fila y columna a analizar
void voltear(char tablero[][DIM], char FA, char CA, char SF, char SC, int n, char color)
{
    int i;
    
    for (i = 0; i < n; i++)
    {
        FA = FA + SF;
        CA = CA + SC;
        tablero[FA][CA] = color;
    }
}
////////////////////////////////////////////////////////////////////////////////
// funcion de bloqueo para detectar error
void error_actualizar(){
	error_actualizar();
}


////////////////////////////////////////////////////////////////////////////////
// comprueba si hay que actualizar alguna ficha
// no comprueba que el movimiento realizado sea v�lido
// f y c son la fila y columna a analizar
// char vSF[DIM] = {-1,-1, 0, 1, 1, 1, 0,-1};
// char vSC[DIM] = { 0, 1, 1, 1, 0,-1,-1,-1};    
int actualizar_tablero(char tablero[][DIM], char f, char c, char color)
{
    char SF, SC; // cantidades a sumar para movernos en la direcci�n que toque
    int i;
    int flip_c_c,flip_c_arm,flip_c_thumb,flip_arm_c,flip_arm_arm,flip_arm_thumb;
    int patron_c_c,patron_c_arm,patron_c_thumb,patron_arm_c,patron_arm_arm,patron_arm_thumb;

    for (i = 0; i < DIM; i++) // 0 es Norte, 1 NE, 2 E ...
    {
        SF = vSF[i];
        SC = vSC[i];
        // flip: numero de fichas a voltear
        flip_c_c = flip_c_arm = flip_c_thumb = flip_arm_c = flip_arm_arm = flip_arm_thumb=0;

        patron_c_c = patron_volteo_c_c(tablero,&flip_c_c, f, c, SF, SC, color);
        //patron_c_arm = patron_volteo_c_arm(tablero,&flip_c_arm, f, c, SF, SC, color);
        //patron_c_thumb = patron_volteo_c_thumb(tablero,&flip_c_thumb, f, c, SF, SC, color);
        //patron_arm_c = patron_volteo_arm_c(tablero,&flip_arm_c, f, c, SF, SC, color);
        //patron_arm_arm = patron_volteo_arm_arm(tablero,&flip_arm_arm, f, c, SF, SC, color);
        //patron_arm_thumb = patron_volteo_arm_thumb(tablero,&flip_arm_thumb, f, c, SF, SC, color);
        /*
        if  (
        	(patron_c_c == patron_c_arm == patron_c_thumb == patron_arm_c == patron_arm_arm == patron_arm_thumb == PATRON_ENCONTRADO)
        	)
        {
        	//bien
        }else{
        	patron_c_c = patron_volteo_c_c(tablero,&flip_c_c, f, c, SF, SC, color);
        	        patron_c_arm = patron_volteo_c_arm(tablero,&flip_c_arm, f, c, SF, SC, color);
        	        patron_c_thumb = patron_volteo_c_thumb(tablero,&flip_c_thumb, f, c, SF, SC, color);
        	        patron_arm_c = patron_volteo_arm_c(tablero,&flip_arm_c, f, c, SF, SC, color);
        	        patron_arm_arm = patron_volteo_arm_arm(tablero,&flip_arm_arm, f, c, SF, SC, color);
        	        patron_arm_thumb = patron_volteo_arm_thumb(tablero,&flip_arm_thumb, f, c, SF, SC, color);

        	error_actualizar();
        }
		*/
        //printf("Flip: %d \n", flip);
        if (patron_c_c == PATRON_ENCONTRADO)
        {
            voltear(tablero, f, c, SF, SC, flip_c_c, color);
        }
        if (patron_c_arm == PATRON_ENCONTRADO)
        {
            voltear(tablero, f, c, SF, SC, flip_c_arm, color);
        }
        if (patron_c_thumb == PATRON_ENCONTRADO)
        {
            voltear(tablero, f, c, SF, SC, flip_c_thumb, color);
        }
        if (patron_arm_c == PATRON_ENCONTRADO)
        {
            voltear(tablero, f, c, SF, SC, flip_arm_c, color);
        }
        if (patron_arm_arm == PATRON_ENCONTRADO)
        {
            voltear(tablero, f, c, SF, SC, flip_arm_arm, color);
        }
        if (patron_arm_thumb == PATRON_ENCONTRADO)
        {
            voltear(tablero, f, c, SF, SC, flip_arm_thumb, color);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// funcion de bloqueo para detectar error
void error_elegirmov(){
	error_elegirmov();
}


/////////////////////////////////////////////////////////////////////////////////
// Recorre todo el tablero comprobando en cada posici�n si se puede mover
// En caso afirmativo, consulta la puntuaci�n de la posici�n y si es la mejor
// que se ha encontrado la guarda
// Al acabar escribe el movimiento seleccionado en f y c

// Candidatas
// NO    0
// SI    1
// CASILLA_OCUPADA 2
int elegir_mov(char candidatas[][DIM], char tablero[][DIM], char *f, char *c,char tipoficha)
{
    int i, j, k, found;
    int mf = -1; // almacena la fila del mejor movimiento encontrado
    int mc;      // almacena la columna del mejor movimiento encontrado
    char mejor = 0; // almacena el mejor valor encontrado
    int longitud_c_c,longitud_c_arm,longitud_c_thumb,longitud_arm_c,longitud_arm_arm,longitud_arm_thumb;
    int patron_c_c,patron_c_arm,patron_c_thumb,patron_arm_c,patron_arm_arm,patron_arm_thumb;
    char SF, SC; // cantidades a sumar para movernos en la direcci�n que toque

    // Recorremos todo el tablero comprobando d�nde podemos mover
    // Comparamos la puntuaci�n de los movimientos encontrados y nos quedamos con el mejor
    for (i=0; i<DIM; i++)
    {
        for (j=0; j<DIM; j++)
        {   // indica en qu� casillas quiz� se pueda mover
            if (candidatas[i][j] == SI)
            {
                if (tablero[i][j] == CASILLA_VACIA)
                {
                    found = 0;
                    k = 0;

                    // en este bucle comprobamos si es un movimiento v�lido
                    // (es decir si implica voltear en alguna direcci�n)
                    while ((found == 0) && (k < DIM))   
                    {
                        SF = vSF[k];    // k representa la direcci�n que miramos
                        SC = vSC[k];    // 1 es norte, 2 NE, 3 E ...

                        // nos dice qu� hay que voltear en cada direcci�n
                        longitud_c_c = longitud_c_arm = longitud_c_thumb = longitud_arm_c = longitud_arm_arm = longitud_arm_thumb = 0;

                        patron_c_c = patron_volteo_c_c(tablero, &longitud_c_c, i, j, SF, SC, tipoficha);
                        //patron_c_arm = patron_volteo_c_arm(tablero, &longitud_c_arm, i, j, SF, SC, tipoficha);
                        //patron_c_thumb = patron_volteo_c_thumb(tablero, &longitud_c_thumb, i, j, SF, SC, tipoficha);
                        //patron_arm_c = patron_volteo_arm_c(tablero, &longitud_arm_c, i, j, SF, SC, tipoficha);
                        //patron_arm_arm = patron_volteo_arm_arm(tablero, &longitud_arm_arm, i, j, SF, SC, tipoficha);
                        //patron_arm_thumb = patron_volteo_arm_thumb(tablero, &longitud_arm_thumb, i, j, SF, SC, tipoficha);
                        //  //printf("%d ", patron);
                        /*
                        if (patron_c_c == patron_c_arm == patron_c_thumb == patron_arm_c == patron_arm_arm == patron_arm_thumb == PATRON_ENCONTRADO)
                        {*/
                            found = 1;
                            if (tabla_valor[i][j] > mejor)
                            {
                                mf = i;
                                mc = j;
                                mejor = tabla_valor[i][j];
                            }
                        /*}else{
                        	patron_c_c = patron_volteo_c_c(tablero, &longitud_c_c, i, j, SF, SC, tipoficha);
                        	patron_c_arm = patron_volteo_c_arm(tablero, &longitud_c_arm, i, j, SF, SC, tipoficha);
							patron_c_thumb = patron_volteo_c_thumb(tablero, &longitud_c_thumb, i, j, SF, SC, tipoficha);
							patron_arm_c = patron_volteo_arm_c(tablero, &longitud_arm_c, i, j, SF, SC, tipoficha);
							patron_arm_arm = patron_volteo_arm_arm(tablero, &longitud_arm_arm, i, j, SF, SC, tipoficha);
							patron_arm_thumb = patron_volteo_arm_thumb(tablero, &longitud_arm_thumb, i, j, SF, SC, tipoficha);
                        	error_elegirmov();
                        }
                        */
                        k++;
                        // si no hemos encontrado nada probamos con la siguiente direcci�n
                    }
                }
            }
        }
    }
    *f = (char) mf;
    *c = (char) mc;
    // si no se ha encontrado una posici�n v�lida devuelve -1
    return mf;
}
////////////////////////////////////////////////////////////////////////////////
// Cuenta el n�mero de fichas de cada color.
// Los guarda en la direcci�n b (blancas) y n (negras)
void contar(char tablero[][DIM], int *b, int *n)
{
    int i,j;

    *b = 0;
    *n = 0;

    // recorremos todo el tablero contando las fichas de cada color
    for (i=0; i<DIM; i++)
    {
        for (j=0; j<DIM; j++)
        {
            if (tablero[i][j] == FICHA_BLANCA)
            {
                (*b)++;
            }
            else if (tablero[i][j] == FICHA_NEGRA)
            {
                (*n)++;
            }
        }
    }
}

void actualizar_candidatas(char candidatas[][DIM], char f, char c)
{
    // donde ya se ha colocado no se puede volver a colocar
    // En las posiciones alrededor s�
    candidatas[f][c] = CASILLA_OCUPADA;
    if (f > 0)
    {
        if (candidatas[f-1][c] != CASILLA_OCUPADA)
            candidatas[f-1][c] = SI;

        if ((c > 0) && (candidatas[f-1][c-1] != CASILLA_OCUPADA))
            candidatas[f-1][c-1] = SI;

        if ((c < 7) && (candidatas[f-1][c+1] != CASILLA_OCUPADA))
            candidatas[f-1][c+1] = SI;
    }
    if (f < 7)
    {
        if (candidatas[f+1][c] != CASILLA_OCUPADA)
            candidatas[f+1][c] = SI;

        if ((c > 0) && (candidatas[f+1][c-1] != CASILLA_OCUPADA))
            candidatas[f+1][c-1] = SI;

        if ((c < 7) && (candidatas[f+1][c+1] != CASILLA_OCUPADA))
            candidatas[f+1][c+1] = SI;
    }
    if ((c > 0) && (candidatas[f][c-1] != CASILLA_OCUPADA))
        candidatas[f][c-1] = SI;

    if ((c < 7) && (candidatas[f][c+1] != CASILLA_OCUPADA))
        candidatas[f][c+1] = SI;
}




////////////////////////////////////////////////////////////////////////////////
// Proceso principal del juego
// Utiliza el tablero,
// y las direcciones en las que indica el jugador la fila y la columna
// y la se�al de ready que indica que se han actualizado fila y columna
// tablero, fila, columna y ready son variables globales aunque deber�an ser locales de reversi8,
// la raz�n es que al meterlas en la pila no las pone juntas, y as� jugar es m�s complicado.
// en esta versi�n el humano lleva negras y la m�quina blancas
// no se comprueba que el humano mueva correctamente.
// S�lo que la m�quina realice un movimiento correcto.
extern void genera_exception_dabort();

void reversi8()
{

	//genera_exception_dabort();

	 ////////////////////////////////////////////////////////////////////
	 // Tablero candidatas: se usa para no explorar todas las posiciones del tablero
	// s�lo se exploran las que est�n alrededor de las fichas colocadas
	 ////////////////////////////////////////////////////////////////////
	char __attribute__ ((aligned (8))) candidatas[DIM][DIM] =
    {
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO},
        {NO,NO,NO,NO,NO,NO,NO,NO}
    };


    int done;     // la m�quina ha conseguido mover o no
    int move = 0; // el humano ha conseguido mover o no
    int blancas, negras; // n�mero de fichas de cada color
    int fin = 0;  // fin vale 1 si el humano no ha podido mover
                  // (ha introducido un valor de movimiento con alg�n 8)
                  // y luego la m�quina tampoco puede
    char f, c;    // fila y columna elegidas por la m�quina para su movimiento

    init_table(tablero, candidatas);
    while (fin == 0)
    {
    	int x = timer2_leer();

    	move = 0;
    	esperar_mov();
		fila = cuenta_fila;
		columna = cuenta_col;
		// si la fila o columna son 8 asumimos que el jugador no puede mover
		if (((fila) != DIM) && ((columna) != DIM))
		{
			tablero[fila][columna] = FICHA_NEGRA;
			actualizar_tablero(tablero, fila, columna, FICHA_NEGRA);
			actualizar_candidatas(candidatas, fila, columna);
			move = 1;
		}
    	// escribe el movimiento en las variables globales fila columna
        done = elegir_mov(candidatas, tablero, &f, &c,FICHA_BLANCA);
        if (done == -1){
            if (move == 0) fin = 1;
        }
        else
        {
            tablero[f][c] = FICHA_BLANCA;
            actualizar_tablero(tablero, f, c, FICHA_BLANCA);
            actualizar_candidatas(candidatas, f, c);
        }
        int y = timer2_leer();
        int res = y - x;

    }
    contar(tablero, &blancas, &negras);
}
