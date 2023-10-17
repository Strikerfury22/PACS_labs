#include <iostream>
#include <time.h>
#include <stdlib.h>

struct Matriz{
	int x;
	int y;
	double* matriz;

	Matriz(){
		x = -1;
		y = -1;
		matriz = NULL;
	}

	Matriz(int x, int y){
		this->x = x;
		this->y = y;
		if(x > 0 && y > 0){
			this->matriz = new double[x*y];
			for (int i = 0; i < x*y; i++){
				this->matriz[i] = 0;
			}
		} else {
			this->matriz = NULL;
		}
		
	}

	Matriz(int x){
		this->x = x;
		this->y = x;
		if(x > 0 && y > 0){
			this->matriz = new double[x*x];
			for (int i = 0; i < x*x; i++){
				this->matriz[i] = 0;
			}
		} else {
			this->matriz = NULL;
		}
		
	}

	~Matriz(){
		/*if (this->matriz != NULL){
			delete [] this->matriz;
			this->matriz = NULL;
		}*/
	}

	void replicar(double num){
		for (int i = 0; i < x*y; i++){
			this->matriz[i] = num;
		}		
	}

	void rellenarRandom(int limite){
		srand(time(NULL));
		for (int i = 0; i < x*y; i++){
			this->matriz[i] = rand() % (limite+1);
		}
	}

	void rellenarRandom2(int limite){
		srand(time(NULL));
		for (int i = 0; i < x*y; i++){
			double divisor = (rand() % (limite+1));
			while (divisor < 1){
				divisor = (rand() % (limite+1));
			}
			this->matriz[i] = (rand() % (limite+1) - rand() % (limite+1)) / divisor;
			if(this->matriz[i] < -10 || this->matriz[i] > 10){
				std::cerr << "Algo ha salido mal: " << this->matriz[i] << std::endl;
			}
		}
	}
};

Matriz multiplicaMatrices(const Matriz& m1, const Matriz& m2){
	if(m1.y != m2.x){
		std::cout << "¡No puedo multiplicar estas dimensiones!" << std::endl;
		Matriz resultado = Matriz(-1);
		resultado.x = -1;
		resultado.y = -1;
		return resultado;
	}
	Matriz resultado = Matriz(m1.x, m2.y);
	for (int i = 0; i < m1.x; i++){
		for (int j = 0; j < m2.y; j++){
			resultado.matriz[i*resultado.y + j] = 0;
			//std::cout << "posicion [" << i << "," << j <<"] = " << resultado.matriz[i*resultado.y + j] << std::endl;
			for (int k = 0; k < m1.y; k++){
				//int indice1 = i*m1.y + k;
				//int indice2 = k*m2.y + j;
				int val1 = m1.matriz[i*m1.y + k];
				int val2 = m2.matriz[k*m2.y + j];
				resultado.matriz[i*resultado.y + j] += val1  * val2;
			}
			//std::cout << "posicion [" << i << "," << j <<"] = " << resultado.matriz[i*resultado.y + j] << std::endl;
		}
	}
	std::cout << "Cálculo realizado." << std::endl;
	return resultado;
}

int mostrarMatriz(const Matriz& m){
	if(m.matriz != NULL){
		std::cout << m.x << "--" << m.y << std::endl;
		std::string lim = "";
		for (int i = 0; i < m.y; i++){
			lim += "-------";
		}
		std::cout << lim;
		for (int i = 0; i < m.x; i++){
			std::cout << std::endl;
			for (int j = 0; j < m.y; j++){
				std::cout << m.matriz[i*m.y + j] << "\t";
			}
		}
		std::cout << std::endl << lim << std::endl;
	}
	return 0;
}

int main(int argc, char* argv[]){
	/*Matriz a1 = Matriz(10,10);
	Matriz b1 = Matriz(10,10);
	a1.rellenarRandom2(10);
	b1.rellenarRandom2(2);
	mostrarMatriz(a1);
	mostrarMatriz(b1);
	mostrarMatriz(a1);
	Matriz c1 = multiplicaMatrices(a1,b1);
	mostrarMatriz(c1);
	*/

	if(argc == 2){
		int id = atoi(argv[1]);
		switch(id){
		case 1:
			{
			//Matrices 32x32 = 1024 elementos
			Matriz a = Matriz(32);
			Matriz b = Matriz(32);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 2:
			{
			//Matrices 100x100 = 10000 elementos
			Matriz a = Matriz(100);
			Matriz b = Matriz(100);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 3:
			{
			//Matrices 150x150 = 22500 elementos
			Matriz a = Matriz(150);
			Matriz b = Matriz(150);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 4:
			{
			//Matrices 708x708 = 501264 elementos
			Matriz a = Matriz(708);
			Matriz b = Matriz(708);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			//mostrarMatriz(a);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 5:
			{
			//Matrices 900x900 = 810000 elementos
			Matriz a = Matriz(900);
			Matriz b = Matriz(900);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 6:
			{
			//Matrices 1200x1200 = 1440000 elementos
			Matriz a = Matriz(1200);
			Matriz b = Matriz(1200);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 7:
			{
			//Matrices 2500x2500 = 6250000 elementos
			Matriz a = Matriz(2500);
			Matriz b = Matriz(2500);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 8:
			{
			//Matrices 5000x5000 = 25000000 elementos
			Matriz a = Matriz(5000);
			Matriz b = Matriz(5000);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		case 9:
			{
			//Matrices 10000x10000 = 100000000 elementos
			Matriz a = Matriz(10000);
			Matriz b = Matriz(10000);
			a.rellenarRandom2(10);
			b.rellenarRandom2(10);
			Matriz c = multiplicaMatrices(a,b);
			}
			break;
		default:
			std::cout << "ID de prueba no válido. Su valor debe estar entre  1 y 9 incluidos." << std::endl;
		}
		std::cout << "Saliendo del programa." << std::endl;
	} else {
		std::cout << "La estructura de llamada es: ./programa <id_prueba>. Donde <id_prueba> es un valor entre 1 y 9 incluidos." << std::endl;
	}
	
}
