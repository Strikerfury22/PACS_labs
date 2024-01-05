__kernel void images_flip(
  __global unsigned char *in_out,
  const unsigned int filas,
  const unsigned int columnas,
  const unsigned int spectrum,
  const unsigned int num_elementos,
  const unsigned int num_wi_por_imagen,
  const unsigned int num_imagenes){
  int i = get_global_id(0); //Obtenemos qué pixel debemos hacer, y la imagen también
  
  int imagen = i / num_wi_por_imagen; //Averiguamos qué imagen debe tratar el wi
  int i_de_la_imagen = i % num_wi_por_imagen; //Averiguamos su posición dentro de la imagen
  
  int mi_columna = i_de_la_imagen/filas + 1; // parte de la fórmula (empieza desde 1)
  int mi_fila = i_de_la_imagen % filas; // parte de la fórmuta (empieza desde 0)

  if(imagen < num_imagenes){ //Solo tienes trabajo si no te sales de este límite (no deberías generar más workitems).
    //Nota: cada pixel ocupa 3 elementos en el buffer
	unsigned char aux_r = in_out[(num_elementos * spectrum * imagen) + i_de_la_imagen*spectrum]; // 3 lecturas de global 
	unsigned char aux_g = in_out[(num_elementos * spectrum * imagen) + i_de_la_imagen*spectrum + 1];
	unsigned char aux_b = in_out[(num_elementos * spectrum * imagen) + i_de_la_imagen*spectrum + 2];
	unsigned int pixel_opuesto_base = (num_elementos * spectrum * imagen) + (num_elementos - filas * mi_columna + mi_fila) * spectrum; // 3 canales de color-> spectrum = 3
	//Primer_cambio
	in_out[(num_elementos * spectrum * imagen) + i_de_la_imagen*spectrum] = in_out[pixel_opuesto_base]; // 3 lecturas/escrituras de global
	in_out[(num_elementos * spectrum * imagen) + i_de_la_imagen*spectrum + 1] = in_out[pixel_opuesto_base + 1];
	in_out[(num_elementos * spectrum * imagen) + i_de_la_imagen*spectrum + 2] = in_out[pixel_opuesto_base + 2];
	//Segundo_cambio
	in_out[pixel_opuesto_base] = aux_r; // 3 escrituras de global (Total: 6 Bytes input y 6 Bytes output en total)
	in_out[pixel_opuesto_base + 1] = aux_g;
	in_out[pixel_opuesto_base + 2] = aux_b;
  }
}