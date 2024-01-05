__kernel void image_flip(
  __global unsigned char *in_out,
  const unsigned int filas,
  const unsigned int columnas,
  const unsigned int spectrum,
  const unsigned int num_elementos){
  int i = get_global_id(0); //Obtenemos qué pixel debemos hacer

  int mi_columna = i/filas + 1; // parte de la fórmula (empieza desde 1)
  int mi_fila = i % filas; // parte de la fórmuta (empieza desde 0)

  if(i < num_elementos/2){ //Solo tienes trabajo si no te sales de este límite (no deberías generar más workitems.
    //Nota: cada pixel ocupa 3 elementos en el buffer
	unsigned char aux_r = in_out[i*spectrum]; // 3 lecturas de global 
	unsigned char aux_g = in_out[i*spectrum + 1];
	unsigned char aux_b = in_out[i*spectrum + 2];
	unsigned int pixel_opuesto_base = (num_elementos - filas * mi_columna + mi_fila) * spectrum; // 3 canales de color-> spectrum = 3
	//Primer_cambio
	in_out[i*spectrum] = in_out[pixel_opuesto_base]; // 3 lecturas/escrituras de global
	in_out[i*spectrum + 1] = in_out[pixel_opuesto_base + 1];
	in_out[i*spectrum + 2] = in_out[pixel_opuesto_base + 2];
	//Segundo_cambio
	in_out[pixel_opuesto_base] = aux_r; // 3 escrituras de global (Total: 6 Bytes input y 6 Bytes output en total)
	in_out[pixel_opuesto_base + 1] = aux_g;
	in_out[pixel_opuesto_base + 2] = aux_b;
  }
}