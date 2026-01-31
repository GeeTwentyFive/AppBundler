#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


char temp[1024];


int DirectoryExists(const char* path) {
        struct stat info;
        return (stat(path, &info) == 0) && S_ISDIR(info.st_mode);
}

int FileExists(const char* path) {
        struct stat info;
        return (stat(path, &info) == 0) && S_ISREG(info.st_mode);
}

#define XOR_HASH_LEN 64
static inline unsigned char* XORHash(const unsigned char* data, size_t data_len) {
        unsigned char* hash = calloc(XOR_HASH_LEN, 1);
        for (size_t i = 0; i < data_len; i++) {
                hash[(i*3) & XOR_HASH_LEN-1] ^= data[i];
        }
        return hash;
}

int main(int argc, const char* argv[]) {
        if (argc < 3) {
                puts("USAGE: <PATH/TO/TARGET/DIR> <PATH/TO/APP/FROM/WITHIN> [ARGS...]");
                return 0;
        }

        if (!DirectoryExists(argv[1])) {
                puts("ERROR: Directory at provided target path not found");
                return 1;
        }

        strcpy(temp, argv[1]);
        strcat(temp, "/");
        strcat(temp, argv[2]);
        if (!FileExists(temp)) {
                puts("ERROR: Target app not found within provided target dir");
                return 1;
        }

        strcpy(temp, "tar cf _TEMP.tar ");
        strcat(temp, argv[1]);
        system(temp);

        FILE* target_file = fopen("_TEMP.tar", "rb");
        if (!target_file) {
                puts("ERROR: Failed to open _TEMP.tar");
                return 1;
        }

        if (fseek(target_file, 0, SEEK_END) != 0) {
                puts("ERROR: Failed to seek to end of _TEMP.tar");
                return 1;
        }
        long target_file_size = ftell(target_file);
        if (fseek(target_file, 0, SEEK_SET) != 0) {
                puts("ERROR: Failed to seek to beginning of _TEMP.tar");
                return 1;
        }

        char* target_file_data = malloc(target_file_size);
        if (target_file_data == NULL) {
                puts("ERROR: Failed to allocate memory for target file data");
                return 1;
        }

        if (fread(target_file_data, 1, target_file_size, target_file) < target_file_size) {
                puts("ERROR: Failed to read data from _TEMP.tar");
                return 1;
        }

        system("rm _TEMP.tar");

        unsigned char* _target_file_data_hash = XORHash(target_file_data, target_file_size);
        unsigned char* target_file_data_hash_hex = calloc(XOR_HASH_LEN*2 + 1, 1);
        for (int i = 0; i < XOR_HASH_LEN; i++) {
                sprintf(
                        target_file_data_hash_hex + i*2,
                        "%02x",
                        _target_file_data_hash[i]
                );
        }
        free(_target_file_data_hash);

        puts(target_file_data_hash_hex); // TEMP; TEST

        // TODO

        free(target_file_data_hash_hex);

        return 0;
}