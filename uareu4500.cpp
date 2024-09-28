#include <iostream>
#include <dpfpdd.h>
#include <dpfj.h>
#include <vector>
#include <fstream>

int init_library() {
	int result;
	 // Inicializa la librería
    result = dpfpdd_init();
    if (result == DPFPDD_SUCCESS) {
        std::cout << "Libreria abierta exitosamente: " << result << std::endl;
		return 0;
    }

	 if (result == DPFPDD_E_FAILURE) {
        std::cerr << "Error al inicializar la librería: " << result << std::endl;
        return -1;
    }

	return -1;
}

int search_devices(std::string &device_name){
	int result;
	// Abrir dispositivos y obtener el nombre del dispositivo
	std::cout << "Buscando dispositivos... " << std::endl;
	// devInfo.size = sizeof(devInfo);
	unsigned int dev_cnt = 0;
	 
    result = dpfpdd_query_devices(&dev_cnt, nullptr );
	std::cout << "Número de dispositivos detectados: " << dev_cnt << std::endl;
	if (result == DPFPDD_E_MORE_DATA || result == DPFPDD_SUCCESS) {
        std::cout << "Número de dispositivos detectados: " << dev_cnt << std::endl;

        // Asignar memoria para almacenar la información de los dispositivos
        DPFPDD_DEV_INFO* dev_infos = new DPFPDD_DEV_INFO[dev_cnt];
        for (unsigned int i = 0; i < dev_cnt; ++i) {
            dev_infos[i].size = sizeof(DPFPDD_DEV_INFO); // Asegúrate de inicializar el campo 'size'
        }

        // Segunda llamada para obtener la información de los dispositivos
        result = dpfpdd_query_devices(&dev_cnt, dev_infos);
        if (result == DPFPDD_SUCCESS) {
            std::cout << "Dispositivos listados exitosamente." << std::endl;

            // Mostrar información de cada dispositivo
            for (unsigned int i = 0; i < dev_cnt; ++i) {
                std::cout << "Dispositivo " << i + 1 << ": " << dev_infos[i].name << std::endl;
            }

			device_name = dev_infos[0].name;

        } else {
            std::cerr << "Error al listar los dispositivos: " << result << std::endl;
        }

        // Liberar la memoria asignada
        delete[] dev_infos;
    } else {
        std::cerr << "Error al obtener el número de dispositivos: " << result << std::endl;
		return -1;
    }

	return -1;
}

int open_device(std::string device_name, DPFPDD_DEV &hReader){
	int result;
	std::cout << "Abriendo dispositivo... " << std::endl;
	
	char* device_name_cstr = const_cast<char*>(device_name.c_str());
	result = dpfpdd_open(device_name_cstr, &hReader);
	if (result == DPFPDD_SUCCESS) {
        std::cout << "Dispositivo abierto exitosamente: " << result << std::endl;
    }

	 if (result == DPFPDD_E_FAILURE) {
        std::cerr << "Fallo inesperado al abrir dispositivo: " << result << std::endl;
        return -1;
    }

	if (result == DPFPDD_E_INVALID_PARAMETER) {
        std::cerr << "No se encuentra un dispositivo con el nombre: " << device_name << std::endl;
        return -1;
    }

	if (result == DPFPDD_E_DEVICE_BUSY) {
        std::cerr << "El dispositivo fue abierto por otro proceso: " << device_name << std::endl;
        return -1;
    }

	if (result == DPFPDD_E_DEVICE_FAILURE) {
        std::cerr << "Error al abrir dispositivo: " << device_name << std::endl;
        return -1;
    }

	return -1;
}

int get_device_status(DPFPDD_DEV hReader, DPFPDD_DEV_STATUS &status){
	int result;
	std::cout << "Consultado status del dispositivo... " << std::endl;
	DPFPDD_DEV_STATUS device_status;
	device_status.size = sizeof(device_status);
	result = dpfpdd_get_device_status(hReader, &device_status);
	if (result == DPFPDD_SUCCESS) {
        std::cout << "Status de dispositivo obtenido exitosamente: " << result << std::endl;
		status = device_status;
    }

	if (result == DPFPDD_E_FAILURE) {
        std::cerr << "Fallo inesperado al obtener status del dispositivo: " << result << std::endl;
        return -1;
    }

	if (result == DPFPDD_E_INVALID_DEVICE) {
        std::cerr << "Dispositivo invalido, no se puede obtener el status: " << result << std::endl;
        return -1;
    }

	if (result == DPFPDD_E_MORE_DATA) {
        std::cerr << "Memoria alojada insuficiente, no se puede obtener el status: " << result << std::endl;
        return -1;
    }

	return -1;
}

int capture_fid(DPFPDD_DEV hReader, unsigned int &fid_size, unsigned char* &fid_data){
	int result;
	std::cout << "Capturando huella... " << std::endl;
	// Generar parametros de captura
	DPFPDD_IMAGE_FMT image_fmt = DPFPDD_IMG_FMT_ANSI381;
	DPFPDD_IMAGE_PROC image_proc = 0;
 	unsigned int image_res = 500;

	DPFPDD_CAPTURE_PARAM capture_param = { sizeof(DPFPDD_CAPTURE_PARAM), image_fmt, image_proc, image_res }; // Parámetros de captura
	unsigned int timeout = -1; // Timeout indefinido
	unsigned int image_size = 0; // Primero obtener el tamaño requerido
	unsigned char* image_data = nullptr; // Puntero nulo para obtener el tamaño necesario para la imagen de la huella
	DPFPDD_CAPTURE_RESULT capture_result = { sizeof(DPFPDD_CAPTURE_RESULT) }; // <-- Aqui se obtiene el resultado de la captura

	
	// Primera llamada de dpfpdd_capture para obtener el tamaño de imagen necesario para image_data 
	result = dpfpdd_capture(hReader, &capture_param, timeout, &capture_result, &image_size, image_data);
	if (result == DPFPDD_E_MORE_DATA) {
		// Asignar memoria para la imagen
		image_data = new unsigned char[image_size];

		// Segunda llamada con memoria asignada
		result = dpfpdd_capture(hReader, &capture_param, timeout, &capture_result, &image_size, image_data);
		std::cerr << "Resultado de captura: " << result << std::endl;
		if (result == DPFPDD_SUCCESS) {
			std::cout << "Captura exitosa: " << result << std::endl;
			fid_size = image_size;
			fid_data = image_data;
		}

		if (result == DPFPDD_E_FAILURE) {
			std::cerr << "Fallo inesperado al capturar: " << result << std::endl;
			return -1;
		}

		if (result == DPFPDD_E_INVALID_DEVICE) {
			std::cerr << "Handle de dispositivo invalido, no se puede capturar: " << result << std::endl;
			return -1;
		}

		if (result == DPFPDD_E_DEVICE_BUSY) {
			std::cerr << "Dispositivo ocupado, no se puede capturar: " << result << std::endl;
			return -1;
		}

		if (result == DPFPDD_E_MORE_DATA) {
			std::cerr << "La memoria alojada en image_data es insuficiente, no se puede capturar: " << result << std::endl;
			return -1;
		}

		if (result == DPFPDD_E_INVALID_PARAMETER) {
			std::cerr << "Parametro invalido, no se puede capturar: " << result << std::endl;
			return -1;
		}
	} else {
		std::cerr << "Error al capturar desde el dispositivo " << std::endl;
		return -1;
	}

	return -1;
}

int transform_fid_to_fmd(unsigned int fid_size, unsigned char* fid_data, unsigned char* &generated_fmd, unsigned int &generated_fmd_size){
	int result;
	std::cout << "Generando FMD usando la captura... " << std::endl;
	DPFJ_FID_FORMAT fid_type = DPFJ_FID_ANSI_381_2004;
	DPFJ_FMD_FORMAT fmd_type = DPFJ_FMD_ANSI_378_2004;
	unsigned int fmd_size = MAX_FMD_SIZE; 
	unsigned char* fmd  = new unsigned char[fmd_size];
	result = dpfj_create_fmd_from_fid(fid_type, fid_data, fid_size, fmd_type, fmd, &fmd_size);
	if (result == DPFPDD_SUCCESS) {
        std::cout << "FMD generado exitosamente: " << result << std::endl;
		if (fmd_size > 0 && fmd != nullptr) {
			std::cout << "Tamaño del FMD: " << fmd_size << std::endl;
			std::cout << "Contenido del FMD (hexadecimal): ";
			for (unsigned int i = 0; i < fmd_size; ++i) {
				std::cout << std::hex << static_cast<int>(fmd[i]) << " ";
			}

			generated_fmd = fmd;
			generated_fmd_size = fmd_size;
			
			std::cout << std::dec << std::endl;
		}
    }

	if (result == DPFPDD_E_MORE_DATA) {
		std::cerr << "Se obtuvo la informacion para generar el FMD pero la memoria alojada en fmd_size es insuficiente: " << result << std::endl;
		return -1;
	}

	if (result == DPFPDD_E_INVALID_PARAMETER) {
		std::cerr << "Parametro invalido, no se puede generar el FMD: " << result << std::endl;
		return -1;
	}

	if (result == DPFJ_E_FAILURE) {
		std::cerr << "Error al crear FMD: " << result << std::endl;
		return -1;
	}

	return -1;
}

int save_fmd(unsigned int fmd_size, unsigned char* fmd, std::string save_path){
	std::cout << "Salvando FMD... " << std::endl;
	std::ofstream outFile(save_path, std::ios::binary);
	std::cout << "1" << std::endl;
	if (outFile.is_open()) {
		if (fmd == nullptr) {
			std::cerr << "Error: fmd es nullptr" << std::endl;
			return -1;
		}
		outFile.write(reinterpret_cast<char*>(fmd), fmd_size);
		outFile.close();
		std::cout << "FMD guardado en fmd.bin" << std::endl;
		return 0;
	} else {
		std::cerr << "Error al abrir el archivo para guardar el FMD." << std::endl;
		return -1;
	}
}

int read_fmd_from_file(unsigned int &fmd_size, unsigned char* &fmd, std::string save_path){
	std::cout << "Leyendo FMD... " << std::endl;
	std::ifstream inFile(save_path, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error al abrir el archivo FMD para lectura." << std::endl;
        return -1;
    }

    // Obtener el tamaño del archivo (FMD)
    inFile.seekg(0, std::ios::end);  // Mover al final para obtener el tamaño
    std::streamsize fmd_size2 = inFile.tellg();
    inFile.seekg(0, std::ios::beg);  // Volver al inicio del archivo

    // Crear un buffer para almacenar el FMD
    std::vector<unsigned char> fmd2(fmd_size);

    // Leer los datos del archivo al buffer
    if (inFile.read(reinterpret_cast<char*>(fmd2.data()), fmd_size2)) {
        std::cout << "FMD cargado exitosamente. Tamaño: " << fmd_size2 << std::endl;
		fmd_size = fmd_size2;
		fmd = fmd2.data();

    } else {
        std::cerr << "Error al leer el archivo FMD." << std::endl;
    }

	 // Comprobar el estado del flujo antes de cerrar
    if (inFile.good()) {
        std::cout << "El archivo se leyó correctamente, cerrando..." << std::endl;
    } else {
        std::cerr << "Hubo un problema antes de cerrar el archivo." << std::endl;
    }

    // Cerrar el archivo
    inFile.close();
    std::cout << "Archivo cerrado exitosamente." << std::endl;
	return 0;
}

int compare_fmds(unsigned int fmd_size, unsigned char* fmd, unsigned int fmd2_size, unsigned char* fmd2, unsigned int &score){
	int result;
	std::cout << "Comparando FMDs...: " << std::endl;
	DPFJ_FMD_FORMAT fmd_type = DPFJ_FMD_ANSI_378_2004;
	int view_index = 0;
	result = dpfj_compare(fmd_type, fmd, fmd_size, view_index, fmd_type, fmd2, fmd2_size, view_index, &score);
	if (result == DPFPDD_SUCCESS) {
		std::cout << "Comparacion terminada, puntuacion: " << score << std::endl;
		if (score < 2000){
			std::cout << "Las huellas coinciden." << std::endl;
		} else {
			std::cout << "Las huellas no coinciden." << std::endl;
		}
    } else {
		std::cerr << "Error al comparar FMDs" << std::endl;
	}
	
	return 0;
}

int main() {
    // DPFPDD_DEV_INFO devInfo;
	init_library();
	std::string device_name;
	search_devices(device_name);
	std::cout << "DEVICE NAME: " << device_name << std::endl;
   
	DPFPDD_DEV hReader = NULL; // Manejar del lector
	open_device(device_name, hReader);

	DPFPDD_DEV_STATUS device_status;
	get_device_status(hReader, device_status);

	// Get First Finger
	unsigned int fid_size;
	unsigned char* fid_data;
	capture_fid(hReader, fid_size, fid_data);

	unsigned int fmd_size = MAX_FMD_SIZE; 
	unsigned char* fmd  = new unsigned char[fmd_size];
	transform_fid_to_fmd(fid_size, fid_data, fmd, fmd_size);

	// Get Second Finger
	unsigned int fid2_size;
	unsigned char* fid2_data;
	capture_fid(hReader, fid2_size, fid2_data);

	unsigned int fmd2_size = MAX_FMD_SIZE; 
	unsigned char* fmd2  = new unsigned char[fmd2_size];
	transform_fid_to_fmd(fid2_size, fid2_data, fmd2, fmd2_size);

	unsigned int score;
	compare_fmds(fmd_size, fmd, fmd2_size, fmd2, score);
	// save_fmd(fmd_size, fmd, "test_savefmd.bin");


	// unsigned int fmd2_size = MAX_FMD_SIZE; 
	// unsigned char* fmd2  = new unsigned char[fmd2_size];
	// read_fmd_from_file(fmd2_size, fmd2, "test_savefmd.bin");
	
	// std::cout << "Iniciando enrollment... " << std::endl;
	// DPFJ_FMD_FORMAT fmd_type = DPFJ_FMD_ISO_19794_2_2005;
	// result = dpfj_start_enrollment(fmd_type);
	// if (result == DPFJ_SUCCESS) {
    //     std::cout << "Enrollment iniciado exitosamente: " << result << std::endl;
    // } else {
	// 	std::cerr << "Error al iniciar Enrrollment: " << result << std::endl;
	// 	return -1;
	// }

	// std::cout << "Añadiendo a enrollment... " << std::endl;
	// unsigned char* fmd;
	// unsigned int fmd_size = MAX_FMD_SIZE; 
	// unsigned int fmd_view_idx = 0; 
	// result = dpfj_add_to_enrollment(fmd_type, fmd, fmd_size, fmd_view_idx);
	// if (result == DPFJ_SUCCESS) {
    //     std::cout << "Adicion a enrollment creada exitosamente: " << result << std::endl;
    // } else {
	// 	std::cerr << "Error al añadir a Enrrollment: " << result << std::endl;
	// 	return -1;
	// }

}