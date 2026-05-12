import uareu4500

# Use example

# Get the fingerprint reading as a String, formatted on base64
base64_finger = uareu4500.getFingerReadingAsBase64String()

# Get a single capture with both the raw FID and generated FMD
full_capture = uareu4500.captureFull()

# Compare a base64 formatted fingerprint reading (That could be stored on a file or bd) with a new fingerprint sensor reading
comparision_result = uareu4500.compareBase64StringWithFingerReading(base64_finger)

print('Full capture keys:', full_capture.keys())
print(f'Fingerprints match?: {comparision_result}')
