import ctypes
from ctypes import c_int, c_char_p, POINTER, c_uint, c_ubyte

# Carga la biblioteca compartida (cambia 'fingerprint.dll' a tu nombre de archivo)
fingerprint_lib = ctypes.CDLL('./uareu4500.dll', winmode=0)

print(fingerprint_lib.init_library)

# Define las firmas de las funciones C++
fingerprint_lib.init_library.restype = c_int
fingerprint_lib.search_devices.restype = c_int
fingerprint_lib.search_devices.argtypes = [POINTER(c_char_p)]
fingerprint_lib.open_device.restype = c_int
fingerprint_lib.open_device.argtypes = [c_char_p, POINTER(ctypes.c_void_p)]
fingerprint_lib.get_device_status.restype = c_int
fingerprint_lib.get_device_status.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_void_p)]
fingerprint_lib.capture_fid.restype = c_int
fingerprint_lib.capture_fid.argtypes = [ctypes.c_void_p, POINTER(c_uint), POINTER(ctypes.POINTER(c_ubyte))]
fingerprint_lib.transform_fid_to_fmd.restype = c_int
fingerprint_lib.transform_fid_to_fmd.argtypes = [c_uint, POINTER(c_ubyte), POINTER(ctypes.POINTER(c_ubyte)), POINTER(c_uint)]
fingerprint_lib.compare_fmds.restype = c_int
fingerprint_lib.compare_fmds.argtypes = [c_uint, POINTER(c_ubyte), c_uint, POINTER(c_ubyte), POINTER(c_uint)]

# Ejemplo de llamada a las funciones desde Python
def init_library():
    return fingerprint_lib.init_library()

def search_devices():
    device_name = ctypes.c_char_p()
    result = fingerprint_lib.search_devices(ctypes.byref(device_name))
    return result, device_name.value

def open_device(device_name):
    hReader = ctypes.c_void_p()
    result = fingerprint_lib.open_device(device_name.encode('utf-8'), ctypes.byref(hReader))
    return result, hReader

def capture_fid(hReader):
    fid_size = ctypes.c_uint()
    fid_data = ctypes.POINTER(c_ubyte)()
    result = fingerprint_lib.capture_fid(hReader, ctypes.byref(fid_size), ctypes.byref(fid_data))
    return result, fid_size.value, fid_data

def transform_fid_to_fmd(fid_size, fid_data):
    fmd_size = ctypes.c_uint()
    fmd_data = ctypes.POINTER(c_ubyte)()
    result = fingerprint_lib.transform_fid_to_fmd(fid_size, fid_data, ctypes.byref(fmd_data), ctypes.byref(fmd_size))
    return result, fmd_size.value, fmd_data

def compare_fmds(fmd1_size, fmd1_data, fmd2_size, fmd2_data):
    score = ctypes.c_uint()
    result = fingerprint_lib.compare_fmds(fmd1_size, fmd1_data, fmd2_size, fmd2_data, ctypes.byref(score))
    return result, score.value

# Ejemplo de uso
if __name__ == "__main__":
    print("Inicializando la librería...")
    result = init_library()
    print("Resultado:", result)
