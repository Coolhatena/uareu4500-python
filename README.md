# U.ARE.U 4500 on Python  
I needed to control a Digital Persona U.ARE.U 4500 Fingerprint Reader using Python to make an integration with LabVIEW, 
but the only SDK's i found were for C++, Java and C# .NET, so i decided to make my own wrapper for the C++ SDK and 
manipulate it from Python

## Concept
Basically all the heavy lifting is done on uareu4500.cpp, here i made some functions to manipulate the SDK
and some other ones (marked with "python_" at the start of their name) that use all those manipulation functions to create
entire functionalities that can be used on Python to get specific values or results (Like get get the fingerprint reading as a string
or compare a reading stored as string with a reading from the scanner).

## Compiling
I made all the compilations of this project on Windows (Thats my target OS so develop and compiling there made everything smoother) using gcc (MSYS)

### Generate DLL:
If you want to modify the uareu4500.cpp file and make you own custom dll, you gotta use the following command:

```
g++ -shared -o uareu4500.dll uareu4500.cpp -Idpfpdd/include -Ldpfpdd/lib -ldpfpdd -Idpfj/include -Ldpfj/lib -ldpfj -I/mingw64/include -L/mingw64/lib -lssl -lcrypto
```

Note: you have to install OpenSSL, for MSYS on Windows you can use the following command

```
pacman -S mingw-w64-x86_64-openssl
```

### Generate DLL:
In the other hand, to generate a .exe (Mostly for debugging and testing purposes) i used the following command:

```
g++ -o uareu4500.exe uareu4500.cpp -Idpfpdd/include -Ldpfpdd/lib -ldpfpdd -Idpfj/include -Ldpfj/lib -ldpfj -I/mingw64/include -L/mingw64/lib -lssl -lcrypto
```

## Side notes
If you want to make your own python library based on this project, consider that you MUST add `winmode=0` on the DLL object creation, like this:
```
fingerprint_lib = ctypes.CDLL('./uareu4500.dll', winmode=0)
```
If this argument its not provided, the code will just not recognize any of the DLL dependencies.



