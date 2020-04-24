//Realizado por Alejandro Escalona García
#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
//tareas de hacer el metodo para buscar botas, push,push

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	//Action accion = actIDLE;
	// Estoy en el nivel 1

	/* actual.fila        = sensores.posF;   //es un estado
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;

	cout << "Fila: " << actual.fila << endl;
	cout << "Col : " << actual.columna << endl;
	cout << "Ori : " << actual.orientacion << endl;

	destino.fila       = sensores.destinoF;
	destino.columna    = sensores.destinoC; */

	/* if (sensores.nivel != 4){
		bool hay_plan = pathFinding (sensores.nivel, actual, destino, plan);
	}
	else { */
		// Estoy en el nivel 2

	/* mapaResultado(sensores.posF)

		cout << "Nivel 2 en procesito" << endl;
	}
	accion = plan.front();
  return accion; */


	/* mapaResultado[sensores.posF][sensores.posC] = sensores.terreno[0];
	switch (sensores.sentido){
		case norte: mapaResultado[sensores.posF - 1][sensores.posC] = sensores.terreno[2];
		break;

		case este: mapaResultado[sensores.posF][sensores.posC + 1] = sensores.terreno[2];
		break;

		case sur: mapaResultado[sensores.posF + 1][sensores.posC] = sensores.terreno[2];
		break;

		case oeste: mapaResultado[sensores.posF][sensores.posC - 1] = sensores.terreno[2];
		break;

} */

	updateMapa(actual, sensores.terreno);//
	

	if(!hayplan){
		actual.fila = sensores.posF;
		actual.columna = sensores.posC;
		actual.orientacion = sensores.sentido;
		destino.fila = sensores.destinoF;
		destino.columna = sensores.destinoC;
		hayplan = pathFinding(sensores.nivel, actual, destino, plan);
	}

	Action sigAccion;
	if( hayplan && plan.size()>0){
		sigAccion = plan.front();
		plan.erase(plan.begin());
		updateMapa(actual, sensores.terreno);//
		updateEstadoActual(actual, sigAccion);//
	}
	else
	{
						
				hayplan = pathFinding(sensores.nivel, actual, destino, plan);
				updateMapa(actual, sensores.terreno);//

				//sigAccion = plan.front();
		

	}
	return sigAccion;



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
						// Incluir aqui la llamada al algoritmo de búsqueda usado en el nivel 2
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

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
			//Aqui falta comprobar que no está en cerrados.
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

//---------------IMPLEMENTACION DEL ALGORITMO PODERADO POR PESO----------------------

// Implementación de la búsqueda ponderada por peso.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.

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
		//calcularPeso(hijoTurnR);
		hijoTurnR.peso = current.peso+1;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			seleccion.insert(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodoPonderado hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		hijoTurnL.peso = current.peso+1;
		//calcularPeso(hijoTurnL);
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			seleccion.insert(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodoPonderado hijoForward = current;
		int pesito = hijoForward.peso;
		calcularPeso(hijoForward);
		hijoForward.peso + pesito;
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

//metodo para ir explorando el mapa
void ComportamientoJugador::updateMapa(estado &st, vector<unsigned char> vector){
	mapaResultado[st.fila][st.columna] = vector[0];
	if(st.fila > 0 and st.fila < (mapaResultado.size()) and st.columna > 0 and st.fila < (mapaResultado.size())){
		switch (st.orientacion) {
			case 0:
				mapaResultado[st.fila-1][st.columna-1] = vector[1];
				mapaResultado[st.fila-1][st.columna] = vector[2];
				mapaResultado[st.fila-1][st.columna+1] = vector[3];

				mapaResultado[st.fila-2][st.columna-2] = vector[4];
				mapaResultado[st.fila-2][st.columna-1] = vector[5];
				mapaResultado[st.fila-2][st.columna] = vector[6];
				mapaResultado[st.fila-2][st.columna+1] = vector[7];
				mapaResultado[st.fila-2][st.columna+2] = vector[8];

				mapaResultado[st.fila-3][st.columna-3] = vector[9];
				mapaResultado[st.fila-3][st.columna-2] = vector[10];
				mapaResultado[st.fila-3][st.columna-1] = vector[11];
				mapaResultado[st.fila-3][st.columna] = vector[12];
				mapaResultado[st.fila-3][st.columna+1] = vector[13];
				mapaResultado[st.fila-3][st.columna+2] = vector[14];
				mapaResultado[st.fila-3][st.columna+3] = vector[15];
				break;
			case 1:
				mapaResultado[st.fila-1][st.columna+1] = vector[1];
				mapaResultado[st.fila][st.columna+1] = vector[2];
				mapaResultado[st.fila+1][st.columna+1] = vector[3];

				mapaResultado[st.fila-2][st.columna+2] = vector[4];
				mapaResultado[st.fila-1][st.columna+2] = vector[5];
				mapaResultado[st.fila][st.columna+2] = vector[6];
				mapaResultado[st.fila+1][st.columna+2] = vector[7];
				mapaResultado[st.fila+2][st.columna+2] = vector[8];

				mapaResultado[st.fila-3][st.columna+3] = vector[9];
				mapaResultado[st.fila-2][st.columna+3] = vector[10];
				mapaResultado[st.fila-1][st.columna+3] = vector[11];
				mapaResultado[st.fila][st.columna+3] = vector[12];
				mapaResultado[st.fila+1][st.columna+3] = vector[13];
				mapaResultado[st.fila+2][st.columna+3] = vector[14];
				mapaResultado[st.fila+3][st.columna+3] = vector[15];
				break;
			case 2:
				mapaResultado[st.fila+1][st.columna+1] = vector[1];
				mapaResultado[st.fila+1][st.columna] = vector[2];
				mapaResultado[st.fila+1][st.columna-1] = vector[3];

				mapaResultado[st.fila+2][st.columna+2] = vector[4];
				mapaResultado[st.fila+2][st.columna+1] = vector[5];
				mapaResultado[st.fila+2][st.columna] = vector[6];
				mapaResultado[st.fila+2][st.columna-1] = vector[7];
				mapaResultado[st.fila+2][st.columna-2] = vector[8];

				mapaResultado[st.fila+3][st.columna+3] = vector[9];
				mapaResultado[st.fila+3][st.columna+2] = vector[10];
				mapaResultado[st.fila+3][st.columna+1] = vector[11];
				mapaResultado[st.fila+3][st.columna] = vector[12];
				mapaResultado[st.fila+3][st.columna-1] = vector[13];
				mapaResultado[st.fila+3][st.columna-2] = vector[14];
				mapaResultado[st.fila+3][st.columna-3] = vector[15];
				break;
			case 3:
				mapaResultado[st.fila+1][st.columna-1] = vector[1];
				mapaResultado[st.fila][st.columna-1] = vector[2];
				mapaResultado[st.fila-1][st.columna-1] = vector[3];

				mapaResultado[st.fila+2][st.columna-2] = vector[4];
				mapaResultado[st.fila+1][st.columna-2] = vector[5];
				mapaResultado[st.fila][st.columna-2] = vector[6];
				mapaResultado[st.fila-1][st.columna-2] = vector[7];
				mapaResultado[st.fila-2][st.columna-2] = vector[8];

				mapaResultado[st.fila+3][st.columna-3] = vector[9];
				mapaResultado[st.fila+2][st.columna-3] = vector[10];
				mapaResultado[st.fila+1][st.columna-3] = vector[11];
				mapaResultado[st.fila][st.columna-3] = vector[12];
				mapaResultado[st.fila-1][st.columna-3] = vector[13];
				mapaResultado[st.fila-2][st.columna-3] = vector[14];
				mapaResultado[st.fila-3][st.columna-3] = vector[15];
				break;
		}
	}
}

//metodo para actualizar el estado actual
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
