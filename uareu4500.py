import ctypes
import json
from ctypes import c_int, c_void_p

# Load C++ DLL
fingerprint_lib = ctypes.CDLL('./uareu4500.dll', winmode=0)

# Set functions return/argument types
fingerprint_lib.python_read_fingerprint_and_get_base64_string.restype = c_void_p
fingerprint_lib.python_capture_full.restype = c_void_p
fingerprint_lib.python_compare_base64_string_with_finger.argtypes = [ctypes.c_char_p]
fingerprint_lib.python_compare_base64_string_with_finger.restype = c_int
fingerprint_lib.python_free_string.argtypes = [c_void_p]
fingerprint_lib.python_free_string.restype = None


def _read_and_free_string(pointer):
	if not pointer:
		raise RuntimeError('Fingerprint DLL returned a null pointer')

	try:
		return ctypes.string_at(pointer).decode('utf-8')
	finally:
		fingerprint_lib.python_free_string(pointer)


def getFingerReadingAsBase64String():
	fmd_base64_str_ptr = fingerprint_lib.python_read_fingerprint_and_get_base64_string()
	base64_string = _read_and_free_string(fmd_base64_str_ptr)

	return base64_string


def captureFull():
	result_ptr = fingerprint_lib.python_capture_full()
	result_json = _read_and_free_string(result_ptr)

	return json.loads(result_json)


def compareBase64StringWithFingerReading(base64_string):
	return bool(fingerprint_lib.python_compare_base64_string_with_finger(base64_string.encode('utf-8')))


# Example of how functions internally work
# For a example of how use them see: test_lib.py
if __name__ == "__main__":
	# Call function to get finger reading as a base64 String pointer
	fmd_base64_str_ptr = fingerprint_lib.python_read_fingerprint_and_get_base64_string()

	# Get actual Base64 String using pointer
	base64_string = _read_and_free_string(fmd_base64_str_ptr)
	
	# Print finger reading
	print("FMD in Base64:", base64_string)

	full_capture = captureFull()
	print("Full capture keys:", full_capture.keys())

	# Compare the finger reading with a new
	comparision_result = fingerprint_lib.python_compare_base64_string_with_finger(base64_string.encode('utf-8'))
	print("Comparision result:", comparision_result)
