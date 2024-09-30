#include <iostream>
#include <dpfpdd.h>
#include <dpfj.h>
#include <vector>
#include <fstream>
#include <cstring>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

extern "C" {
	__declspec(dllexport) int init_library() {
		int result;
		// Initialize library
		result = dpfpdd_init();
		switch (result) {
			case DPFPDD_SUCCESS:
				std::cout << "Libreria abierta exitosamente: " << result << std::endl;
				return 0;

			case DPFPDD_E_FAILURE:
				std::cerr << "Error al inicializar la librería: " << result << std::endl;
		}

		return -1;
	}

	__declspec(dllexport) int search_devices(std::string &device_name){
		int result;
		// Open devices and get device name
		std::cout << "Buscando dispositivos... " << std::endl;
		unsigned int dev_cnt = 0;
		
		result = dpfpdd_query_devices(&dev_cnt, nullptr ); // First call just to get the needed memory size
		if (result == DPFPDD_E_MORE_DATA || result == DPFPDD_SUCCESS) {
			std::cout << "Número de dispositivos detectados: " << dev_cnt << std::endl;

			// Asign memory to store device data
			DPFPDD_DEV_INFO* dev_infos = new DPFPDD_DEV_INFO[dev_cnt];
			for (unsigned int i = 0; i < dev_cnt; ++i) {
				dev_infos[i].size = sizeof(DPFPDD_DEV_INFO);
			}

			// Second call to actually get device data
			result = dpfpdd_query_devices(&dev_cnt, dev_infos);
			if (result == DPFPDD_SUCCESS) {
				std::cout << "Dispositivos listados exitosamente." << std::endl;
				// Show devices info
				// for (unsigned int i = 0; i < dev_cnt; ++i) {
				// 	std::cout << "Dispositivo " << i + 1 << ": " << dev_infos[i].name << std::endl;
				// }

				// Set the first detected device as our reader
				device_name = dev_infos[0].name;

			} else {
				std::cerr << "Error al listar los dispositivos: " << result << std::endl;
			}

			// Free asigned memory
			delete[] dev_infos;
		} else {
			std::cerr << "Error al obtener el número de dispositivos: " << result << std::endl;
		}

		return -1;
	}

	__declspec(dllexport) int open_device(std::string device_name, DPFPDD_DEV &hReader){
		int result;
		std::cout << "Abriendo dispositivo... " << std::endl;
		
		char* device_name_cstr = const_cast<char*>(device_name.c_str());
		result = dpfpdd_open(device_name_cstr, &hReader);

		switch (result){
			case DPFPDD_SUCCESS:
				std::cout << "Dispositivo abierto exitosamente: " << result << std::endl;
				return 0;
			
			case DPFPDD_E_FAILURE:
				std::cerr << "Fallo inesperado al abrir dispositivo: " << result << std::endl;
				break;
			
			case DPFPDD_E_INVALID_PARAMETER:
				std::cerr << "No se encuentra un dispositivo con el nombre: " << device_name << std::endl;
				break;
				
			case DPFPDD_E_DEVICE_BUSY:
				std::cerr << "El dispositivo fue abierto por otro proceso: " << device_name << std::endl;
				break;

			case DPFPDD_E_DEVICE_FAILURE:
				std::cerr << "Error al abrir dispositivo: " << device_name << std::endl;
				break;
		}

		return -1;
	}

	__declspec(dllexport) int close_device(DPFPDD_DEV hReader){
		int result;
		std::cout << "Cerrando dispositivo... " << std::endl;
		result =dpfpdd_close(hReader);

		switch (result){
			case DPFPDD_SUCCESS:
				std::cout << "Dispositivo cerrado exitosamente: " << result << std::endl;
				return 0;
			
			case DPFPDD_E_FAILURE:
				std::cerr << "Fallo inesperado al cerrar dispositivo: " << result << std::endl;
				break;
			
			case DPFPDD_E_INVALID_DEVICE:
				std::cerr << "No se encuentra el dispositivo que se quiere cerrar. " << std::endl;
				break;
				
			case DPFPDD_E_DEVICE_BUSY:
				std::cerr << "El dispositivo esta ocupado por otro proceso. " << std::endl;
				break;

			case DPFPDD_E_DEVICE_FAILURE:
				std::cerr << "Error al cerrar dispositivo. " << std::endl;
				break;
		}

		return -1;
	}

	__declspec(dllexport) int get_device_status(DPFPDD_DEV hReader, DPFPDD_DEV_STATUS &status){
		int result;
		std::cout << "Consultado status del dispositivo... " << std::endl;
		DPFPDD_DEV_STATUS device_status;
		device_status.size = sizeof(device_status);
		result = dpfpdd_get_device_status(hReader, &device_status);
		switch (result){
			case DPFPDD_SUCCESS:
				std::cout << "Status de dispositivo obtenido exitosamente: " << result << std::endl;
				status = device_status;
				return 0;
			
			case DPFPDD_E_FAILURE:
				std::cerr << "Fallo inesperado al obtener status del dispositivo: " << result << std::endl;
				break;
			
			case DPFPDD_E_INVALID_DEVICE:
				std::cerr << "Dispositivo invalido, no se puede obtener el status: " << result << std::endl;
				break;
				
			case DPFPDD_E_DEVICE_BUSY:
				std::cerr << "El dispositivo esta ocupado por otro proceso. " << std::endl;
				break;

			case DPFPDD_E_MORE_DATA:
				std::cerr << "Memoria alojada insuficiente, no se puede obtener el status: " << result << std::endl;
				break;
		}

		return -1;
	}

	__declspec(dllexport) int capture_fid(DPFPDD_DEV hReader, unsigned int &fid_size, unsigned char* &fid_data){
		int result;
		std::cout << "Capturando huella... " << std::endl;
		// Generate capture params
		DPFPDD_IMAGE_FMT image_fmt = DPFPDD_IMG_FMT_ANSI381;
		DPFPDD_IMAGE_PROC image_proc = 0;
		unsigned int image_res = 500;

		DPFPDD_CAPTURE_PARAM capture_param = { sizeof(DPFPDD_CAPTURE_PARAM), image_fmt, image_proc, image_res }; // Parámetros de captura
		unsigned int timeout = -1; // Timeout indefinido
		unsigned int image_size = 0; // Primero obtener el tamaño requerido
		unsigned char* image_data = nullptr; // Puntero nulo para obtener el tamaño necesario para la imagen de la huella
		DPFPDD_CAPTURE_RESULT capture_result = { sizeof(DPFPDD_CAPTURE_RESULT) }; // <-- Aqui se obtiene el resultado de la captura

		
		// first dpfpdd_capture call to get the needed size for image_data 
		result = dpfpdd_capture(hReader, &capture_param, timeout, &capture_result, &image_size, image_data);
		if (result == DPFPDD_E_MORE_DATA) {
			// Asign image memory
			image_data = new unsigned char[image_size];

			// Second call with the correct size on image_data
			result = dpfpdd_capture(hReader, &capture_param, timeout, &capture_result, &image_size, image_data);
			std::cerr << "Resultado de captura: " << result << std::endl;
			switch (result){
				case DPFPDD_SUCCESS:
					std::cout << "Captura exitosa: " << result << std::endl;
					fid_size = image_size;
					fid_data = image_data;
					return 0;
				
				case DPFPDD_E_FAILURE:
					std::cerr << "Fallo inesperado al capturar: " << result << std::endl;
					break;
				
				case DPFPDD_E_INVALID_DEVICE:
					std::cerr << "Handle de dispositivo invalido, no se puede capturar: " << result << std::endl;
					break;
					
				case DPFPDD_E_DEVICE_BUSY:
					std::cerr << "Dispositivo ocupado, no se puede capturar: " << result << std::endl;
					break;

				case DPFPDD_E_MORE_DATA:
					std::cerr << "La memoria alojada en image_data es insuficiente, no se puede capturar: " << result << std::endl;
					break;

				case DPFPDD_E_INVALID_PARAMETER:
					std::cerr << "Parametro invalido, no se puede capturar: " << result << std::endl;
					break;
			}
		} else {
			std::cerr << "Error al capturar desde el dispositivo " << std::endl;
		}

		return -1;
	}

	__declspec(dllexport) int transform_fid_to_fmd(unsigned int fid_size, unsigned char* fid_data, unsigned char* &generated_fmd, unsigned int &generated_fmd_size){
		int result;
		std::cout << "Generando FMD usando la captura... " << std::endl;
		DPFJ_FID_FORMAT fid_type = DPFJ_FID_ANSI_381_2004;
		DPFJ_FMD_FORMAT fmd_type = DPFJ_FMD_ANSI_378_2004;
		unsigned int fmd_size = MAX_FMD_SIZE; 
		unsigned char* fmd  = new unsigned char[fmd_size];
		result = dpfj_create_fmd_from_fid(fid_type, fid_data, fid_size, fmd_type, fmd, &fmd_size);
		switch (result){
			case DPFPDD_SUCCESS:
				if (fmd_size > 0 && fmd != nullptr) {
					std::cout << "FMD generado exitosamente: " << result << std::endl;
					std::cout << "Tamaño del FMD: " << fmd_size << std::endl;
					// std::cout << "Contenido del FMD (hexadecimal): ";
					// for (unsigned int i = 0; i < fmd_size; ++i) {
					// 	std::cout << std::hex << static_cast<int>(fmd[i]) << " ";
					// }

					generated_fmd = fmd;
					generated_fmd_size = fmd_size;
					
					std::cout << std::dec << std::endl;
				} else{
					std::cout << "FMD generado con errores: " << result << std::endl;
				}
				return 0;
			
			case DPFPDD_E_MORE_DATA:
				std::cerr << "Se obtuvo la informacion para generar el FMD pero la memoria alojada en fmd_size es insuficiente: " << result << std::endl;
				break;

			case DPFPDD_E_INVALID_PARAMETER:
				std::cerr << "Parametro invalido, no se puede generar el FMD: " << result << std::endl;
				break;

			case DPFPDD_E_FAILURE:
				std::cerr << "Error al crear FMD: " << result << std::endl;
				break;
		}

		return -1;
	}

	__declspec(dllexport) int save_fmd(unsigned int fmd_size, unsigned char* fmd, std::string save_path){
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

	__declspec(dllexport) int read_fmd_from_file(unsigned int &fmd_size, unsigned char* &fmd, std::string save_path){
		std::cout << "Leyendo FMD... " << std::endl;
		std::ifstream inFile(save_path, std::ios::binary);
		if (!inFile) {
			std::cerr << "Error al abrir el archivo FMD para lectura." << std::endl;
			return -1;
		}

		// Get FMD file size
		inFile.seekg(0, std::ios::end);  // Move to the end to get total size
		std::streamsize fmd_size2 = inFile.tellg();
		inFile.seekg(0, std::ios::beg);  // Get back to file start

		// Create buffer to store FMD
		std::vector<unsigned char> fmd2(fmd_size);

		// Read file data to buffer
		if (inFile.read(reinterpret_cast<char*>(fmd2.data()), fmd_size2)) {
			std::cout << "FMD cargado exitosamente. Tamaño: " << fmd_size2 << std::endl;
			fmd_size = fmd_size2;
			fmd = fmd2.data();

		} else {
			std::cerr << "Error al leer el archivo FMD." << std::endl;
		}

		// Check file state before closing
		if (inFile.good()) {
			std::cout << "El archivo se leyó correctamente, cerrando..." << std::endl;
		} else {
			std::cerr << "Hubo un problema antes de cerrar el archivo." << std::endl;
		}

		// Close file
		inFile.close();
		std::cout << "Archivo cerrado exitosamente." << std::endl;
		return 0;
	}

	__declspec(dllexport) int compare_fmds(unsigned int fmd_size, unsigned char* fmd, unsigned int fmd2_size, unsigned char* fmd2, unsigned int &score){
		int result;
		std::cout << "Comparando FMDs...: " << std::endl;
		DPFJ_FMD_FORMAT fmd_type = DPFJ_FMD_ANSI_378_2004;
		int view_index = 0;
		result = dpfj_compare(fmd_type, fmd, fmd_size, view_index, fmd_type, fmd2, fmd2_size, view_index, &score);
		if (result == DPFPDD_SUCCESS) {
			std::cout << "Comparacion terminada, puntuacion: " << score << std::endl;
			if (score < 2000){
				std::cout << "Las huellas coinciden." << std::endl;
				return 1;
			} else {
				std::cout << "Las huellas no coinciden." << std::endl;
			}
		} else {
			std::cerr << "Error al comparar FMDs" << std::endl;
		}
		
		return 0;
	}

	std::string convert_fmd_to_base64(const unsigned char* fmd, unsigned int fmd_size) {
		BIO* bio, *b64;
		BUF_MEM* bufferPtr;

		b64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());
		bio = BIO_push(b64, bio);

		// Set BIO to not insert line breaks
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		BIO_write(bio, fmd, fmd_size);
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufferPtr);

		std::string base64_data(bufferPtr->data, bufferPtr->length);
		BIO_free_all(bio);

		return base64_data;
	}

	std::vector<unsigned char> convert_base64_to_fmd(const std::string& base64_data) {
		BIO* bio, *b64;
		std::vector<unsigned char> fmd;

		b64 = BIO_new(BIO_f_base64());
		bio = BIO_new_mem_buf(base64_data.data(), base64_data.length());
		bio = BIO_push(b64, bio);

		// Set BIO to not insert line breaks
		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

		// set FMD vector size based on expected lenght
		fmd.resize((base64_data.length() * 3) / 4);

		// Leer desde el BIO Base64 y llenar el vector fmd
		int decoded_length = BIO_read(bio, fmd.data(), fmd.size());
		if (decoded_length <= 0) {
			std::cerr << "Error: No se pudo decodificar los datos Base64 correctamente." << std::endl;
		} else {
			fmd.resize(decoded_length);  // Adjust size of decoded data
		}

		BIO_free_all(bio);
		return fmd;
	}

	__declspec(dllexport) DPFPDD_DEV get_reader_data(){
		init_library();
		std::string device_name;
		search_devices(device_name);
		DPFPDD_DEV hReader = NULL; // Reader handler
		open_device(device_name, hReader);

		return hReader;
	}

	__declspec(dllexport) const char* python_read_fingerprint_and_get_base64_string() {
		DPFPDD_DEV hReader = get_reader_data();
		// Get First Finger
		unsigned int fid_size;
		unsigned char* fid_data;
		capture_fid(hReader, fid_size, fid_data);

		unsigned int fmd_size = MAX_FMD_SIZE; 
		unsigned char* fmd  = new unsigned char[fmd_size];
		transform_fid_to_fmd(fid_size, fid_data, fmd, fmd_size);

		// Returns FMD as base64
		std::string fmd_base64_str = convert_fmd_to_base64(fmd, fmd_size);
		
		// Adds result to heap
		char* result = new char[fmd_base64_str.size() + 1]; // +1 for null terminator
		std::strcpy(result, fmd_base64_str.c_str());
		
		close_device(hReader);
		return result; // Return pointer to string on heap
	}

	__declspec(dllexport) int python_compare_base64_string_with_finger(const char* input_base64_str){
		std::string base64_str = std::string(input_base64_str);
		std::cout << "Received string: " << base64_str << std::endl;

		DPFPDD_DEV hReader = get_reader_data();
		unsigned int fid_size;
		unsigned char* fid_data;
		capture_fid(hReader, fid_size, fid_data);

		unsigned int fmd_size = MAX_FMD_SIZE; 
		unsigned char* fmd  = new unsigned char[fmd_size];
		transform_fid_to_fmd(fid_size, fid_data, fmd, fmd_size);

		std::vector<unsigned char> fmd_vect = convert_base64_to_fmd(base64_str);
		unsigned int fmd2_size = fmd_vect.size(); 
		unsigned char* fmd2 = fmd_vect.data();

		unsigned int score;
		unsigned int comparision_result;
		comparision_result = compare_fmds(fmd_size, fmd, fmd2_size, fmd2, score);
		close_device(hReader);
		return comparision_result;
	}

	__declspec(dllexport) int main() {
		// Testing code, you can ignore it or use it compiling this as a .exe instead of .dll :)
		init_library();
		std::string device_name;
		search_devices(device_name);
		std::cout << "DEVICE NAME: " << device_name << std::endl;
	
		DPFPDD_DEV hReader = NULL; // Reader handler
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

		std::string fmd_string = convert_fmd_to_base64(fmd, fmd_size);
		std::cout << "FMD String: " << fmd_string << std::endl;
		
		std::vector<unsigned char> fmd_vect = convert_base64_to_fmd(fmd_string);

		unsigned int fmd2_size = fmd_vect.size(); 
		unsigned char* fmd2 = fmd_vect.data();

		std::cout << "Size 1: " << fmd_size << std::endl;
		std::cout << "Size 2: " << fmd2_size << std::endl;
		bool areEqual = fmd2_size == fmd_size;
		std::cout << "Are sizes equal?: " << areEqual << std::endl;

		unsigned int score;
		compare_fmds(fmd_size, fmd, fmd2_size, fmd2, score);
		// Get Second Finger
		// unsigned int fid2_size;
		// unsigned char* fid2_data;
		// capture_fid(hReader, fid2_size, fid2_data);

		// unsigned int fmd2_size = MAX_FMD_SIZE; 
		// unsigned char* fmd2  = new unsigned char[fmd2_size];
		// transform_fid_to_fmd(fid2_size, fid2_data, fmd2, fmd2_size);

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
}