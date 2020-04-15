//Realizado por Alejandro Escalona García

#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"


#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
//probar

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	Action accion = actIDLE;
	// Estoy en el nivel 1  

	actual.fila        = sensores.posF;   //es un estado
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;

	cout << "Fila: " << actual.fila << endl;
	cout << "Col : " << actual.columna << endl;
	cout << "Ori : " << actual.orientacion << endl;

	destino.fila       = sensores.destinoF;
	destino.columna    = sensores.destinoC;

	if (sensores.nivel != 4){
		bool hay_plan = pathFinding (sensores.nivel, actual, destino, plan);
	}
	else {
		// Estoy en el nivel 2
		cout << "En proceso nivel 2" << endl;

		if(!estadoActualIniciado){
			actual.fila = 100;
			actual.columna = 100;
			actual.orientacion = brujula;
			estadoActualIniciado = true;
		}

		if(sensores.superficie[2] == 'a'){
			updateEstadoActual(actual, actTURN_R);
			if(pre-vista){
				pre-vista = false;
			}
			ExistePlan = false;
			return actTURN_R;
		}

		if(!saberLocalizacion){ 
			if(obstaculoEnfrenteImprovisto(actual, sensores.terreno)){
				updateEstadoActual(actual, actTURN_L);
				ExistePlan = false; 
				return actTURN_L;
			}
			if(!ExistePlan){
					if(pre-vista){
						ExistePlan = pathFinding_K(actual, plan);//-------------//s
					}
					else{
						ExistePlan = pathFinding_Desconocido(actual, plan);//-------------//s
					}
			}
			Action sigaction;
			if(ExistePlan && plan.size() > 0){
				sigaction=plan.front();
				plan.erase(plan.begin());
			}
			else{
				sigaction = actIDLE;
			}
			ultimaAccion = sigaction;
			updateMapaSinExplorar(actual, sensores.terreno);
			updateEstadoActual(actual, sigaction);
			if(!pre-vista){
				for(int i = 0; i < 16; i++){
					if(sensores.terreno[i] == 'K'){
						cout << "HE VISTO K" << endl;
						pre-vista = true;
						updateEstadoActual(actual, actIDLE);
						return actIDLE;
					}
				}
			}
			else{
				if(sensores.mensajeF != -1){
					cout << "ESTOY EN K" << endl;
					saberLocalizacion = true;
					ExistePlan = false;
					actual.fila = sensores.mensajeF;
					actual.columna = sensores.mensajeC;
					destino.fila = sensores.destinoF;
					destino.columna = sensores.destinoC;
					return actIDLE;
				}
			}
			if(plan.size() == 0){
				ExistePlan = false;
			}
			return sigaction;
		}
		else{
			if(obstaculoEnfrenteImprovisto(actual, sensores.terreno) && ExistePlan){
				updateEstadoActual(actual, actIDLE);
				ExistePlan = false;
				return actIDLE;
			}
			if(!ExistePlan){
				cout << "Actual: (" << actual.fila << "," << actual.columna << ")" << endl;
				cout << "Destino: (" << destino.fila << "," << destino.columna << ")" << endl;
				ExistePlan = pathFinding (sensores.nivel, actual, destino, plan);
			}
			Action sigaction;
			if(ExistePlan && plan.size() > 0){
				sigaction=plan.front();
				plan.erase(plan.begin());
			}
			ultimaAccion = sigaction;
			updateMapa(actual, sensores.terreno);
			updateEstadoActual(actual, sigaction);

			if(plan.size() == 0){
				if(actual.fila == destino.fila and actual.columna == destino.columna){
					objetivos++;
					cout << "Objetivos alcanzados: " << objetivos << endl;
				}
				destino.fila = sensores.destinoF;
				destino.columna = sensores.destinoC;
				ExistePlan = false;
			}
			return sigaction;
		}
  		return accion;
	} // FIN DEL ELSE

	
	accion = plan.front();
  return accion;
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
				 return pathFinding_CostoUniforme(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
						return pathFinding_CostoUniforme(origen,destino,plan);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Funciones auxiliares ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M' or casilla =='D')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}



struct nodo{
	estado st;
	list<Action> secuencia;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados;						 // Lista de Cerrados (lista de nodos que ya han sido expandidos pero no explorados)
	stack<nodo> pila;											// Lista de Abiertos (lista de nodos visitados)

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

//---------------IMPLEMENTACION DEL ALGORITMO EN ANCHURA----------------------

// Implementación de la búsqueda en Anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	queue<nodo> cola;											// Lista de Abiertos
	set<estado,ComparaEstados> generados; 						// Lista de Cerrados
	

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);


		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

//---------------IMPLEMENTACION DEL ALGORITMO PONDERADO POR PESO----------------------

// Implementación de la búsqueda ponderada por peso.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.

//calculamos el peso según el paso.
void ComportamientoJugador::calcularPeso( nodoPonderado &nodo){
	int columna = nodo.st.columna;
	int fila = nodo.st.fila;
	char casilla = mapaResultado[fila][columna];
	switch(casilla){
		case 'T': nodo.peso = 2; break;
		case 'A': nodo.peso = 100; break;
		case 'B': nodo.peso = 50; break;
		default: nodo.peso = 1; break;
	}
}


bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	queue<nodoPonderado> cola;											// Lista de Abiertos
	set<estado,ComparaEstados> generados;      	// Lista de Cerrados
	set<nodoPonderado,nodosComparados> seleccion;
	

  nodoPonderado current;
	current.st = origen;
	current.secuencia.empty();
	current.peso = 0;

	cola.push(current);

  while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);


		// Generar descendiente de girar a la derecha
		nodoPonderado hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		calcularPeso(hijoTurnR);
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			seleccion.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodoPonderado hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		calcularPeso(hijoTurnL);
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			seleccion.insert(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodoPonderado hijoForward = current;
		calcularPeso(hijoForward);
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				seleccion.insert(hijoForward);
			}
		}

		set<nodoPonderado, nodosComparados>::iterator it;
		for(it=seleccion.begin(); it != seleccion.end(); it++)
			cola.push(*it);

			seleccion.clear();

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}
// ------------- REALIZADOS PARA EL NIVEL 2--------------------
void ComportamientoJugador::updateMapa( vector<unsigned char> V , estado &st){
	mapaResultado[st.fila][st.columna] = V[0];
	if(st.fila > 0 and st.fila < (mapaResultado.size()) and st.columna > 0 and st.fila < (mapaResultado.size())){
		switch (st.orientacion) {
			case 0:
				mapaResultado[st.fila-1][st.columna-1] = V[1];
				mapaResultado[st.fila-1][st.columna] = V[2];
				mapaResultado[st.fila-1][st.columna+1] = V[3];

				mapaResultado[st.fila-2][st.columna-2] = V[4];
				mapaResultado[st.fila-2][st.columna-1] = V[5];
				mapaResultado[st.fila-2][st.columna] = V[6];
				mapaResultado[st.fila-2][st.columna+1] = V[7];
				mapaResultado[st.fila-2][st.columna+2] = V[8];

				mapaResultado[st.fila-3][st.columna-3] = V[9];
				mapaResultado[st.fila-3][st.columna-2] = V[10];
				mapaResultado[st.fila-3][st.columna-1] = V[11];
				mapaResultado[st.fila-3][st.columna] = V[12];
				mapaResultado[st.fila-3][st.columna+1] = V[13];
				mapaResultado[st.fila-3][st.columna+2] = V[14];
				mapaResultado[st.fila-3][st.columna+3] = V[15];
				break;
			case 1:
				mapaResultado[st.fila-1][st.columna+1] = V[1];
				mapaResultado[st.fila][st.columna+1] = V[2];
				mapaResultado[st.fila+1][st.columna+1] = V[3];

				mapaResultado[st.fila-2][st.columna+2] = V[4];
				mapaResultado[st.fila-1][st.columna+2] = V[5];
				mapaResultado[st.fila][st.columna+2] = V[6];
				mapaResultado[st.fila+1][st.columna+2] = V[7];
				mapaResultado[st.fila+2][st.columna+2] = V[8];

				mapaResultado[st.fila-3][st.columna+3] = V[9];
				mapaResultado[st.fila-2][st.columna+3] = V[10];
				mapaResultado[st.fila-1][st.columna+3] = V[11];
				mapaResultado[st.fila][st.columna+3] = V[12];
				mapaResultado[st.fila+1][st.columna+3] = V[13];
				mapaResultado[st.fila+2][st.columna+3] = V[14];
				mapaResultado[st.fila+3][st.columna+3] = V[15];
				break;
			case 2:
				mapaResultado[st.fila+1][st.columna+1] = V[1];
				mapaResultado[st.fila+1][st.columna] = V[2];
				mapaResultado[st.fila+1][st.columna-1] = V[3];

				mapaResultado[st.fila+2][st.columna+2] = V[4];
				mapaResultado[st.fila+2][st.columna+1] = V[5];
				mapaResultado[st.fila+2][st.columna] = V[6];
				mapaResultado[st.fila+2][st.columna-1] = V[7];
				mapaResultado[st.fila+2][st.columna-2] = V[8];

				mapaResultado[st.fila+3][st.columna+3] = V[9];
				mapaResultado[st.fila+3][st.columna+2] = V[10];
				mapaResultado[st.fila+3][st.columna+1] = V[11];
				mapaResultado[st.fila+3][st.columna] = V[12];
				mapaResultado[st.fila+3][st.columna-1] = V[13];
				mapaResultado[st.fila+3][st.columna-2] = V[14];
				mapaResultado[st.fila+3][st.columna-3] = V[15];
				break;
			case 3:
				mapaResultado[st.fila+1][st.columna-1] = V[1];
				mapaResultado[st.fila][st.columna-1] = V[2];
				mapaResultado[st.fila-1][st.columna-1] = V[3];

				mapaResultado[st.fila+2][st.columna-2] = V[4];
				mapaResultado[st.fila+1][st.columna-2] = V[5];
				mapaResultado[st.fila][st.columna-2] = V[6];
				mapaResultado[st.fila-1][st.columna-2] = V[7];
				mapaResultado[st.fila-2][st.columna-2] = V[8];

				mapaResultado[st.fila+3][st.columna-3] = V[9];
				mapaResultado[st.fila+2][st.columna-3] = V[10];
				mapaResultado[st.fila+1][st.columna-3] = V[11];
				mapaResultado[st.fila][st.columna-3] = V[12];
				mapaResultado[st.fila-1][st.columna-3] = V[13];
				mapaResultado[st.fila-2][st.columna-3] = V[14];
				mapaResultado[st.fila-3][st.columna-3] = V[15];
				break;
		}
	}
}


void ComportamientoJugador::updateMapaSinExplorar(vector<unsigned char> V, estado &st){
	sinexplorar[st.fila][st.columna] = V[0];
	if(st.fila > 7 and st.fila < (200-7) and st.columna > 7 and st.fila < (200-7)){
		switch (st.orientacion) {
			case 0:
				sinexplorar[st.fila-1][st.columna-1] = V[1];
				sinexplorar[st.fila-1][st.columna] = V[2];
				sinexplorar[st.fila-1][st.columna+1] = V[3];

				sinexplorar[st.fila-2][st.columna-2] = V[4];
				sinexplorar[st.fila-2][st.columna-1] = V[5];
				sinexplorar[st.fila-2][st.columna] = V[6];
				sinexplorar[st.fila-2][st.columna+1] = V[7];
				sinexplorar[st.fila-2][st.columna+2] = V[8];

				sinexplorar[st.fila-3][st.columna-3] = V[9];
				sinexplorar[st.fila-3][st.columna-2] = V[10];
				sinexplorar[st.fila-3][st.columna-1] = V[11];
				sinexplorar[st.fila-3][st.columna] = V[12];
				sinexplorar[st.fila-3][st.columna+1] = V[13];
				sinexplorar[st.fila-3][st.columna+2] = V[14];
				sinexplorar[st.fila-3][st.columna+3] = V[15];
				break;
			case 1:
				sinexplorar[st.fila-1][st.columna+1] = V[1];
				sinexplorar[st.fila][st.columna+1] = V[2];
				sinexplorar[st.fila+1][st.columna+1] = V[3];

				sinexplorar[st.fila-2][st.columna+2] = V[4];
				sinexplorar[st.fila-1][st.columna+2] = V[5];
				sinexplorar[st.fila][st.columna+2] = V[6];
				sinexplorar[st.fila+1][st.columna+2] = V[7];
				sinexplorar[st.fila+2][st.columna+2] = V[8];

				sinexplorar[st.fila-3][st.columna+3] = V[9];
				sinexplorar[st.fila-2][st.columna+3] = V[10];
				sinexplorar[st.fila-1][st.columna+3] = V[11];
				sinexplorar[st.fila][st.columna+3] = V[12];
				sinexplorar[st.fila+1][st.columna+3] = V[13];
				sinexplorar[st.fila+2][st.columna+3] = V[14];
				sinexplorar[st.fila+3][st.columna+3] = V[15];
				break;
			case 2:
				sinexplorar[st.fila+1][st.columna+1] = V[1];
				sinexplorar[st.fila+1][st.columna] = V[2];
				sinexplorar[st.fila+1][st.columna-1] = V[3];

				sinexplorar[st.fila+2][st.columna+2] = V[4];
				sinexplorar[st.fila+2][st.columna+1] = V[5];
				sinexplorar[st.fila+2][st.columna] = V[6];
				sinexplorar[st.fila+2][st.columna-1] = V[7];
				sinexplorar[st.fila+2][st.columna-2] = V[8];

				sinexplorar[st.fila+3][st.columna+3] = V[9];
				sinexplorar[st.fila+3][st.columna+2] = V[10];
				sinexplorar[st.fila+3][st.columna+1] = V[11];
				sinexplorar[st.fila+3][st.columna] = V[12];
				sinexplorar[st.fila+3][st.columna-1] = V[13];
				sinexplorar[st.fila+3][st.columna-2] = V[14];
				sinexplorar[st.fila+3][st.columna-3] = V[15];
				break;
			case 3:
				sinexplorar[st.fila+1][st.columna-1] = V[1];
				sinexplorar[st.fila][st.columna-1] = V[2];
				sinexplorar[st.fila-1][st.columna-1] = V[3];

				sinexplorar[st.fila+2][st.columna-2] = V[4];
				sinexplorar[st.fila+1][st.columna-2] = V[5];
				sinexplorar[st.fila][st.columna-2] = V[6];
				sinexplorar[st.fila-1][st.columna-2] = V[7];
				sinexplorar[st.fila-2][st.columna-2] = V[8];

				sinexplorar[st.fila+3][st.columna-3] = V[9];
				sinexplorar[st.fila+2][st.columna-3] = V[10];
				sinexplorar[st.fila+1][st.columna-3] = V[11];
				sinexplorar[st.fila][st.columna-3] = V[12];
				sinexplorar[st.fila-1][st.columna-3] = V[13];
				sinexplorar[st.fila-2][st.columna-3] = V[14];
				sinexplorar[st.fila-3][st.columna-3] = V[15];
				break;
		}
	}
}

//Se comprueba el estado de la casilla de delante, si es valida o no.
bool ComportamientoJugador::estadoCasillaDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // se calcula cual es la casilla de delante
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// se comprueba que el agente no se sale de los limites del mapa
	if (fil<0 or fil>=200) return true;
	if (col<0 or col>200) return true;

	// Se comprueba que no haya un obstaculo que no limite el paso.
	if (desconocidos[fil][col] != 'P' and desconocidos[fil][col] != 'M' and desconocidos[fil][col] != 'D'){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

//Metodo para comprobar si hay un obstaculo enfrente o no.
bool ComportamientoJugador::obstaculoEnfrenteImprovisto(estado &st, vector<unsigned char> V){
	if(V[2] == 'P' || V[2] == 'M'){
		return true;
	}
	else{
		return false;
	}
}

//Metodo que actualiza el estado actual
void ComportamientoJugador::updateEstadoActual(estado &st, Action accion){
		switch (accion) {
			case actFORWARD:
				switch (st.orientacion) {
					case 0: st.fila--; break;
					case 1: st.columna++; break;
					case 2: st.fila++; break;
					case 3: st.columna--; break;
				}
				break;
			case actTURN_L:
				st.orientacion = (st.orientacion+3)%4;
				break;
			case actTURN_R:
				st.orientacion = (st.orientacion+1)%4;
				break;
			case actIDLE:
				break;
		}
}

//REVISAAAR
bool ComportamientoJugador::pathFinding_Desconocido(const estado &origen, list<Action> &plan) {
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and desconocidos[current.st.fila][current.st.columna] != '?'){
		pila.pop();
		generados.insert(current.st);


		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}
		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}
		// Generar descendiente de avanzar
		nodo hijoForward = current;
		switch (hijoForward.st.orientacion) {
			case 0: hijoForward.st.fila--; break;
			case 1: hijoForward.st.columna++; break;
			case 2: hijoForward.st.fila++; break;
			case 3: hijoForward.st.columna--; break;
		}
		if(!estadoForwardnoValido(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";
	if (desconocidos[current.st.fila][current.st.columna] == '?'){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}
	return false;
}

// ------- REVISAAR---
bool ComportamientoJugador::pathFinding_K(const estado &origen, list<Action> &plan) {
	cout << "Calculando camino hacia K" << endl;
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

  while (!cola.empty() and desconocidos[current.st.fila][current.st.columna] != 'K'){
		cola.pop();
		generados.insert(current.st);


		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}
		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}
		// Generar descendiente de avanzar
		nodo hijoForward = current;
		switch (hijoForward.st.orientacion) {
			case 0: hijoForward.st.fila--; break;
			case 1: hijoForward.st.columna++; break;
			case 2: hijoForward.st.fila++; break;
			case 3: hijoForward.st.columna--; break;
		}

		if (generados.find(hijoForward.st) == generados.end()){
			hijoForward.secuencia.push_back(actFORWARD);
			cola.push(hijoForward);
		}


		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}

  cout << "Terminada la busqueda\n";
	if (desconocidos[current.st.fila][current.st.columna] == 'K'){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}
	return false;
}


//---------FUNCIONES PARA PINTAR EL MAPA Y DEMÁS (NO TOCAR )-------

// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}




void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
