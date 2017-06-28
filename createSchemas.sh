gobjcopy --input binary --output mach-o-x86-64 --binary-architecture i386 inputschema.json inputschema.o
gobjcopy --input binary --output mach-o-x86-64 --binary-architecture i386 serverschema.json serverschema.o


#ld -r  -o inputschema.o inputschema.json
#ld -r -o serverschema.o serverschema.json