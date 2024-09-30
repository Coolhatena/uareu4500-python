import ctypes
from ctypes import c_int, c_char_p, POINTER, c_uint, c_ubyte

# Load C++ DLL 
fingerprint_lib = ctypes.CDLL('./uareu4500.dll', winmode=0)

# Set functions return/argument types
fingerprint_lib.python_read_fingerprint_and_get_base64_string.restype = c_char_p
fingerprint_lib.python_compare_base64_string_with_finger.argtypes = [ctypes.c_char_p]
fingerprint_lib.python_compare_base64_string_with_finger.restype = c_int

def getFingerReadingAsBase64String():
	fmd_base64_str_ptr = fingerprint_lib.python_read_fingerprint_and_get_base64_string()
	base64_string = ctypes.string_at(fmd_base64_str_ptr).decode('utf-8')

	return base64_string

def compareBase64StringWithFingerReading(base64_string):
	return bool(fingerprint_lib.python_compare_base64_string_with_finger(base64_string.encode('utf-8')))


# Example of how functions internally work
# For a example of how use them see: test_lib.py
if __name__ == "__main__":
	# Call function to get finger reading as a base64 String pointer
	fmd_base64_str_ptr = fingerprint_lib.python_read_fingerprint_and_get_base64_string()

	# Get actual Base64 String using pointer 
	base64_string = ctypes.string_at(fmd_base64_str_ptr).decode('utf-8')
	
	# Print finger reading
	print("FMD in Base64:", base64_string)

	# Compare the finger reading with a new 
	comparision_result = fingerprint_lib.python_compare_base64_string_with_finger(base64_string.encode('utf-8'))
	print("Comparision result:", comparision_result)
