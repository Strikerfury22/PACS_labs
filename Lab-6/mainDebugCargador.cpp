#define cimg_use_jpeg
#include "Cargador.hpp"
#include "join_threads.hpp"
#include <iostream>
#include "CImg.h"

//CargaYDiQueTienes
void cargaYDi(Cargador& cargador, unsigned int indiceBase, unsigned int cantidadACargar, unsigned char* buffer, unsigned int& numCargas){
    numCargas = cargador.cargaImagenes(indiceBase,cantidadACargar,buffer);
    std::cout << "Num_elementos_por_imagen: " << cargador.num_elementos_imagen << std::endl;
}

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "MALA LLAMADA" << std::endl;
        return -1;
    }
    cimg_library::CImg<unsigned char> img("image.jpg");
	unsigned int numImagenesTotal = 3;
    Cargador cargador(numImagenesTotal, img, argv[1]);
    int columnas = img.width();
    int filas = img.height();
    int spectrum = img.spectrum();
    std::vector<std::thread> _threads;
    unsigned int indices[2] = {0,2};
    unsigned int cargas[2] = {0,0};
    unsigned char* buffer = new unsigned char[4 * img.width() * img.height() * img.spectrum()];
    for (int i = 0; i < 2; ++i){
        _threads.push_back(std::thread(cargaYDi,std::ref(cargador),indices[i],2,buffer,std::ref(cargas[i])));
    }
    { 
        join_threads j(_threads);
    }
    const char* names[2] = {"s1.jpg","s2.jpg"}; 
    for (int i = 0; i < 2; ++i){
        std::cout << "El thread " << i <<" ha cargado " << cargas[i] << std::endl;
        cimg_library::CImg<unsigned char> salida(img.width(),img.height(),1,img.spectrum(),0);
        int indice_p = indices[i] * filas * columnas * spectrum;
        if (strcmp(argv[1],"horizontal")==0){
            for (int col = 0; col < columnas; col++){
                for (int fila = 0; fila < filas; fila++){
                    for (int color = 0; color < spectrum; color++){
                        salida(col,fila,0,color) = buffer[indice_p];
                        indice_p++;
                    }
                }
            }
        } else {
            for (int fila = 0; fila < filas; fila++){
                for (int col = 0; col < columnas; col++){
                    for (int color = 0; color < spectrum; color++){
                        salida(col,fila,0,color) = buffer[indice_p];
                        indice_p++;
                    }
                }
            }
        }
        salida.save(names[i]);
    }
    return 0;
}