gcc -O2 ../../payload.c -o PAYLOAD.exe
chcp 65001
tinyxxd.exe -n PAYLOAD -i PAYLOAD.exe > ../../PAYLOAD.h
rm PAYLOAD.exe
gcc -O2 ../../main.c -o AppBundler.exe
