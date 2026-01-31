#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>


char temp[1024];


int DirectoryExists(const char* path) {
        struct stat info;
        return (stat(path, &info) == 0) && S_ISDIR(info.st_mode);
}

int FileExists(const char* path) {
        struct stat info;
        return (stat(path, &info) == 0) && S_ISREG(info.st_mode);
}

char* FileToCArray(const char* path) {
        FILE* f = fopen(path, "rb");
        if (!f) return NULL;

        if (fseek(f, 0, SEEK_END) != 0) return NULL;
        long target_file_size = ftell(f);
        if (fseek(f, 0, SEEK_SET) != 0) return NULL;

        char* file_as_c_array = malloc(256 + (target_file_size * 5));
        if (file_as_c_array == NULL) return NULL;

        strcpy(file_as_c_array, "const unsigned char DATA[] = {");
        int c;
        while ((c = fgetc(f)) != EOF) {
                sprintf(
                        file_as_c_array + strlen(file_as_c_array),
                        "0x%02X,",
                        (unsigned char)c
                );
        }
        fclose(f);
        strcat(file_as_c_array, "}");

        return file_as_c_array;
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

        char* file_data_array = FileToCArray("_TEMP.tar");

        system("rm _TEMP.tar");

        FILE* f = fopen("_TEMP.c", "w");
        const unsigned char INCLUDES[] =
                "#include <stdlib.h>\n"
                "#include <sys/stat.h>\n"
                "#include <sys/types.h>\n"
                "#include <unistd.h>\n"
                "#include <stdio.h>\n"
                "#include <stdlib.h>\n";
        fwrite(INCLUDES, 1, sizeof(INCLUDES), f);
        // TODO
        fclose(f);

        system(
                "cc -O2 _TEMP.c -o OUT"
                #ifdef _WIN32
                        ".exe"
                #endif
        );

        system("rm _TEMP.c");

        return 0;
}