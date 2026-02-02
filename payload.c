#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#ifdef _WIN32
#define fseek64 _fseeki64
#define ftell64 _ftelli64
#else
#define fseek64 fseeko
#define ftell64 ftello
#endif


#ifdef _WIN32
#define RUN_PREFIX ".\\"
#else
#define RUN_PREFIX "./"
#endif

#define HASH_LEN 128


static inline int DirectoryExists(const char* path) {
        struct stat info;
        return (stat(path, &info) == 0) && S_ISDIR(info.st_mode);
}

int main(int argc, const char* argv[]) {
        FILE* f = fopen(argv[0], "rb");
        if (!f) {
                puts("ERROR: Failed to open self executable for reading");
                return 1;
        }
        fseek64(f, 0, SEEK_END);

        fseek64(f, -sizeof(uint32_t), SEEK_CUR);
        uint32_t args_count;
        fread(&args_count, sizeof(uint32_t), 1, f);

        int total_args_len = 0;
        char** args = calloc(args_count, sizeof(char*));
        if (args == NULL) {
                puts("ERROR: Failed to allocate memory for extra args");
                return 1;
        }
        for (int32_t i = args_count-1; i >= 0; i--) {
                fseek64(f, -sizeof(int32_t), SEEK_CUR);
                int32_t arg_len;
                fread(&arg_len, sizeof(int32_t), 1, f);

                fseek64(f, -arg_len, SEEK_CUR);
                char* arg = calloc(arg_len+1, sizeof(char));
                if (arg == NULL) {
                        puts("ERROR: Failed to allocate memory for extra arg");
                        return 1;
                }
                fread(arg, sizeof(char), arg_len, f);

                total_args_len += arg_len;
                args[i] = arg;
        }

        fseek64(f, -(HASH_LEN+1), SEEK_CUR);
        char hash[HASH_LEN+1] = {0};
        fread(hash, sizeof(char), HASH_LEN+1, f);

        char* tmpdir =
        #ifdef _WIN32
                getenv("TEMP")
        #else
                "/tmp"
        #endif
        ;
        if (tmpdir == NULL) {
                puts("ERROR: Failed to get path to temp directory");
                return 1;
        }
        if (chdir(tmpdir) != 0) {
                puts("ERROR: Failed to change current working dir to temp dir");
                return 1;
        }

        if (!DirectoryExists(hash)) {
                if (mkdir(hash
                #ifndef _WIN32
                        , 0755
                #endif
                ) != 0) {
                        puts("ERROR: Failed to create output dir for app");
                        return 1;
                }
                if (chdir(hash) != 0) {
                        puts("ERROR: Failed to change current working dir to app output dir");
                        return 1;
                }

                uint64_t payload_len;
                fseek64(f, -sizeof(uint64_t), SEEK_CUR);
                fread(&payload_len, sizeof(uint64_t), 1, f);

                FILE* out = fopen("_TEMP.tar.gz", "wb");
                if (!out) {
                        puts("ERROR: Failed to create temp program output file");
                        return 1;
                }
                fseek64(f, -payload_len, SEEK_CUR);
                for (int i = 0; i < payload_len; i++) fputc(fgetc(f), out); // TODO: Fix...?
                fclose(out);

                system("tar xf _TEMP.tar.gz");
                system("rm _TEMP.tar.gz");
        }
        else {
                if (chdir(hash) != 0) {
                        puts("ERROR: Failed to change current working dir to extracted app dir");
                        return 1;
                }
        }

        fclose(f);

        int run_cmd_len = strlen(RUN_PREFIX) + total_args_len + args_count + 1;
        char* run_cmd = calloc(run_cmd_len, 1);
        strcpy(run_cmd, RUN_PREFIX);
        for (int i = 0; i < args_count; i++) {
                strcat(run_cmd, args[i]);
                strcat(run_cmd, " ");
        }
        //system(run_cmd);
        puts(run_cmd); // TODO: FIX

        return 0;
}