#pragma once

typedef short int Word;
typedef int Integer;
typedef long long int Int64;
typedef long long int _Int64;
typedef void* Pointer;
typedef void** PPointer;
typedef char Byte;
typedef bool Boolean;
typedef unsigned int Cardinal;
#ifdef WIN32
typedef unsigned int NativeUInt;
#else
typedef unsigned long long int NativeUInt;
#endif
typedef NativeUInt* PNativeUInt;
