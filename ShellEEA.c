/* Sistem Programlama Proje-1*/
/* Group EEA*/
/*Azra Öykü Ulukan 22120205059*/
/*Eren Akkoç 22120205045*/
/*Rabia Ece Sert 22120205057*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAX_ARGS 256

int main() {
    char usr[256] = {0};
    char *args[MAX_ARGS] = {NULL}; /* Komutan argümanlarını saklamak için bir dizi oluşturur.*/
    char *token; /* strtok fonksiyonunu kullanmak için tanımladığımız değişken*/
    FILE *log_file;
    log_file = fopen("log.txt", "a");

    while (1) {
        write(1, "$ ", 2); /* Prompt yazdır*/

        fgets(usr, sizeof(usr), stdin); /* Kullanıcı girişini al*/
        usr[strcspn(usr, "\n")] = 0; /* Satır sonu karakterini kaldır*/

        /* Zaman bilgisini tutmak için*/
        struct timeval tv;
        gettimeofday(&tv, NULL);
        long int current_time = tv.tv_sec;

        /* Log dosyasına zaman bilgisini ve komutu yaz*/
        fprintf(log_file, "%ld: %s\n", current_time, usr);
        fflush(log_file); /* Veriyi dosyaya yazmak için arabelleğin temizlenmesi*/

        token = strtok(usr, " "); /*Girdiyi boşluk karakterine göre ayırmak için*/
        int i = 0;
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        /* "cd" komutu kontrolü*/
        if (strcmp(args[0], "cd") == 0) {
            if (chdir(args[1]) != 0) {
                perror("chdir failed");
            }
            continue; /* Döngüyü yeniden başlat*/
        }

        /* Yeni bir işlem oluştur*/
        pid_t pid = fork();

        if (pid < 0) {
            /* Hata durumunda*/
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            /* Child Process'in gerçekleştiği kısımlar*/

            /* STDERR'ı kapat*/
            close(STDERR_FILENO);

            /* STDERR'ı log dosyasına yönlendir*/
            dup2(fileno(log_file), STDERR_FILENO);

            execvp(args[0], args); /* Komutu çalıştır*/
            /* execvp çağrısı başarısız olursa altındaki kodlar çalışır*/

            /* Eğer execvp başarısız olursa child process -1 döndür*/
            exit(EXIT_FAILURE);
        } else {
            /* Fork işlemi başarılıysa ve Parent Process gerçekleşiyorsa bu şartın sağlanması için*/
            int status;
            pid_t terminated_pid = waitpid(pid, &status, 0); /* Child Process'in bitmesini bekle*/
            if (terminated_pid == -1) {
                /* waitpid çağrısı başarısız olursa hata mesajı*/
                perror("waitpid failed");
            } else {
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status == EXIT_FAILURE) {
                        /* Komut bulunamadı hatası ver*/
                        char error_message[256];
                        sprintf(error_message, "%s: command not found\n", args[0]);
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
            }
        }
    }

    fclose(log_file); /* Log dosyasını kapat*/
    return 0;
}