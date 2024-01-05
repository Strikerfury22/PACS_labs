__kernel void image_flip(
  __global unsigned char *in_out,
  const unsigned int filas,
  const unsigned int columnas,
  const unsigned int spectrum,
  const unsigned int num_elementos){
  int i = get_global_id(0); //Obtenemos qué pixel debemos hacer

  int mi_columna = i / filas; // parte de la fórmula (empieza desde 0)
  int mi_fila = i % filas; // parte de la fórmuta (empieza desde 0)

  if(i < num_elementos/2){ //Solo tienes trabajo si no te sales de este límite (no deberías generar más workitems.
    int pixel_origen_base = (mi_columna * filas + mi_fila) * spectrum;
	unsigned char aux_r = in_out[pixel_origen_base]; // 3 canales de color -> i*3
	unsigned char aux_g = in_out[pixel_origen_base + 1];
	unsigned char aux_b = in_out[pixel_origen_base + 2];
	unsigned int pixel_opuesto_base = ((columnas - mi_columna - 1) * filas + mi_fila) * spectrum;
	//Primer_cambio
	in_out[pixel_origen_base] = in_out[pixel_opuesto_base];
	in_out[pixel_origen_base + 1] = in_out[pixel_opuesto_base + 1];
	in_out[pixel_origen_base + 2] = in_out[pixel_opuesto_base + 2];
	//Segundo_cambio
	in_out[pixel_opuesto_base] = aux_r;
	in_out[pixel_opuesto_base + 1] = aux_g;
	in_out[pixel_opuesto_base + 2] = aux_b;
  }
}