#pragma once
#include <cstdlib>
#include <memory>
#include <mutex>
//#include <vector>
#include "CImg.h"
//#include <iostream> //Para debug

class Cargador
{
  private:
    mutable std::mutex _m; //mutable allows to modify an object in a const function member. We need this modification for the empty() function.   
    unsigned int num_imagenes_cargadas;
    unsigned int num_imagenes_total;
    cimg_library::CImg<unsigned char> imagen;
    //std::vector<int> num_imagenes_hilo;

  public:
    char* modo;
    unsigned int num_elementos_imagen;
    int columnas, filas, spectrum;
    /*Empty constructor*/
    Cargador() {} 
    
    /*Normal Constructor*/
    Cargador(const Cargador& other)
    {
      std::lock_guard<std::mutex> lk(other._m);
      imagen = other.imagen;
      num_imagenes_cargadas = other.num_imagenes_cargadas;
      num_imagenes_total = other.num_imagenes_total;
      modo = other.modo;
      num_elementos_imagen = other.num_elementos_imagen;
    }

    /*Constructor a partir de la imagen y el número máximo de cargas a realizar*/
    Cargador(const unsigned int _num_imagenes_total, const cimg_library::CImg<unsigned char> _imagen, char* _modo) //, int numHilos)
    {
      imagen = _imagen;
      num_imagenes_cargadas = 0;
      num_imagenes_total = _num_imagenes_total;
      modo = _modo;
      num_elementos_imagen = imagen.width() * imagen.height() * imagen.spectrum(); 
      columnas = imagen.width();
      filas = imagen.height();
      spectrum = imagen.spectrum();
      /*for (int i = 0; i < numHilos; ++i){
        _num_imagenes_hilo.push_back(0);
      }*/
    }

    Cargador& operator=(const Cargador&) = delete;

    /*Nótese que no hay exclusión mutua al cargar las imagenes en el buffer "pixelesEntrada" porque se
     *supone que se han dividido las secciones que utiliza cada uno previamente, por lo que no debe haber
     *solapamiento. Que esto se cumpla queda a discreción de la llamada a la función.
     *
     *Parámetros:
     *	indiceBase: índice a partir del que se cargan imagenes en "pixeles_entrada". Si "pixeles_entrada"
     *				contiene X imágenes, puede tomar valores entre 0 y X-1. (Entre hilos no deben coincidir)
     *	numImagenes: Número de imagenes que hay que cargar en el buffer "pixeles_entrada". Si 
     *				"indice_base" = X, y "num_imagenes" = Y, el rango que escribe de "pixeles_entrada" es
     *				[X,X+Y-1].
     *	pixelesEntrada: El buffer en el que se cargarán las imagenes (compartido por los hilos).
     *Devuelve:
     *	En "pixelesEntrada" devuelve las imagenes cargadas en el rango especificado. La función devuelve el
     *	número de imagenes que se han cargado en realidad dentro de "pixelesEntrada".
     */
    unsigned int cargaImagenes(const unsigned int indiceBase, const unsigned int numImagenes, unsigned char* pixelesEntrada){
      unsigned int numCargas = 0;
      for (int i = 0; i < numImagenes; ++i){
        {
          std::lock_guard<std::mutex> lk(_m);
          if (num_imagenes_cargadas < num_imagenes_total){
            //Indicamos que se carga una imagen más
            ++num_imagenes_cargadas;
            ++numCargas;
          } else {
            //Dejamos de cargar imágenes
            break;
          }
        }
        int indice_p = indiceBase * num_elementos_imagen;
        if (strcmp(modo,"horizontal")==0){
          for (int col = 0; col < columnas; col++){
            for (int fila = 0; fila < filas; fila++){
              for (int color = 0; color < spectrum; color++){
                pixelesEntrada[indice_p] = imagen(col,fila,0,color);
                ++indice_p;
              }
            }
          }
        } else {
          for (int fila = 0; fila < filas; fila++){
            for (int col = 0; col < columnas; col++){
              for (int color = 0; color < spectrum; color++){
                pixelesEntrada[indice_p] = imagen(col,fila,0,color);
                ++indice_p;
              }
            }
          }
        }
      }
      return numCargas;
    }
};
