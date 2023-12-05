
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define WALL '#'
#define START 'a'
#define EXIT 'b'
#define FREE_CORRIDOR ' '
#define PATH '*'

//Program, który wczytuje labirynt, znajduje w nim drogę do wyjścia i wyświetla wynik.
//
//W tym celu program pobierze od użytkownika nazwę pliku (pamięć alokowana dynamicznie na 31 bajtów, w przypadku niepowodzenia program wyświetla komunikat Failed to allocate memory i kończy działanie z kodem błędu 8.).
//Następnie wczyta labirynt w nim zapisany, odnajdzie ścieżkę prowadzącą z punktu a do punktu b i wyświetli labirynt z zaznaczoną drogą.
//
//W przypadku:
//
//-Niemożliwości otworzenia pliku program wyświetla komunikat Couldn't open file i kończy działanie z kodem błędu 4,
//-Kiedy dane w pliku są uszkodzone program wyświetla komunikat File corrupted i kończy działanie z kodem błędu 6,
//-Kiedy nie uda się przydzielić pamięci na labirynt wyświetla komunikat Failed to allocate memory i kończy działanie z kodem błędu 8,
//-Kiedy nie uda się odnaleźć ścieżki program wyświetla komunikat Couldn't find path i kończy działanie z kodem błędu 0.
//Program ma zwrócić kod błędu 0 jeżeli udało się odnaleźć ścieżkę lub tej ścieżki nie było. Brak ścieżki, z punktu widzenia programu do jej szukania, nie jest błędem.
//
//Plik wejściowy jest uszkodzony kiedy nie jest możliwe jego jednoznaczne przeszukanie, np. nie posiada spójnej struktury wierszy i kolumn, posiada więcej niż jedno wejście/wyjście, nie posiada wejścia lub wyjścia.

//Przykładowe pliki:
//
//tylko ściany: only_maze.txt,
//labirynt z wyjściem: exit.txt,
//labirynt bez wyjścia: no_exit.txt.

void free_maze(char **maze) {
    int i;
    for (i = 0; *(maze+i) != NULL; i++) {
        free(*(maze+i));
    }
    free(maze);
}

int count_c(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return -1; // nie udało się otworzyć pliku
    }

    int cols = 0;
    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        cols++;
    }

    fclose(f);
    return cols;
}

int cl = 0;
int r = 0;
int x_a,y_a;
int count_a = 0, count_b = 0;
int load_maze(const char *filename, char ***labirynth){
    if(filename == NULL || labirynth == NULL){
        return 1;
    }

    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        return 2;
    }

    int rows = 1, cols = 0;
    char c;
    int current_cols = 0;
    int first_line_cols = count_c(filename);

    while (fscanf(fin, "%c", &c) == 1) {
        if (c == '\n') {
            rows++;
            if (current_cols > cols) {
                cols = current_cols;
            }
            if(current_cols != first_line_cols){
                fclose(fin);
                return 3;
            }
            current_cols = 0;
        } else {
            current_cols++;
        }
    }

    // last line may not have \n character
    if (current_cols > cols) {
        cols++;
    }
    cols += 1;

    if(rows == 1){
        fclose(fin);
        return 3;
    }

    if (c != '\n' && current_cols > cols) {
        cols = current_cols;
    }

    *labirynth = (char **) malloc((rows+1) * sizeof(char *));
    if (*labirynth == NULL) {
        fclose(fin);
        return 4;
    }

    for (int i = 0; i < rows; ++i) {
        *(*labirynth + i) = (char *) malloc((cols) * sizeof(char));
        if (*(*labirynth + i) == NULL) {
            fclose(fin);
            for (int j = 0; j < i; ++j) {
                free(*(*labirynth + j));
            }
            free(*labirynth);
            return 4;
        }
    }

    fseek(fin, 0, SEEK_SET);



    int row = 0;
    int col = 0;
    while (fscanf(fin, "%c", &c) == 1) {
        if (c == WALL) {
            *(*(*labirynth+row)+col) = WALL;
            col++;
        }
        else if (c == FREE_CORRIDOR || c == START || c == EXIT) {
            if(c == START){
                x_a = row;
                y_a = col;
                count_a++;
            }
            if(c == EXIT) count_b++;

            *(*(*labirynth+row)+col)= c;
            col++;
        }
        else if (c == EOF){
            break;
        }
        else if (c == '\n') {
            *(*(*labirynth+row)+col)= '\0';
            row++;
            col = 0;
        }
        else{
            for (int k = 0; k < row; ++k) {
                free(*(*labirynth+k));
            }
            free(*labirynth);
            fclose(fin);
            return 3;
        }
    }
    if (c != EOF) {
        *(*(*labirynth+row)+col)= '\0';
    }


    cl = cols-1;
    r = rows;

    *(*labirynth+rows) = NULL;

    fclose(fin);

    return 0;
}


int isSafe(char **lab, int x, int y) {
    // sprawdzenie, czy pozycja jest w granicach planszy
    return (lab && x >= 0 && y >= 0 && x < cl && y < r &&
            ( *(*(lab+y)+x) == FREE_CORRIDOR || *(*(lab+y)+x) == START || *(*(lab+y)+x) == EXIT) &&
            *(*(lab+y)+x) != 'v');
}

int solve_maze(char **maze, int x, int y){

    if(maze == NULL){
        return -1;
    }

    int rows = 0;
    while (*(maze+rows) != NULL) {
        rows++;
    }

    int temp_cols;
    for (int i = 0; i < rows; ++i) {
        temp_cols = 0;
        while(*(*(maze+i)+temp_cols) != '\0'){
            temp_cols++;
        }
        if(temp_cols != cl){
            return -1;
        }
    }


    if(x < 0 || y < 0 || x >= cl || y >= r || rows != r){
        return -1;
    }

    if(*(*(maze+y)+x)==EXIT){
        return 1;
    }

    if(*(*(maze+y)+x) != PATH && *(*(maze+y)+x) != START) {
        *(*(maze+y)+x) = 'v';
    }

    if(isSafe(maze,x+1,y)){
        if(solve_maze(maze,x+1,y)){
            if(*(*(maze+y)+x) != START) {
                *(*(maze+y)+x) = PATH;
            }
            return 1;
        }
    }

    if(isSafe(maze,x,y+1)){
        if(solve_maze(maze,x,y+1)){
            if(*(*(maze+y)+x) != START) {
                *(*(maze+y)+x) = PATH;
            }
            return 1;
        }
    }

    if(isSafe(maze,x-1,y)){
        if(solve_maze(maze,x-1,y)){
            if(*(*(maze+y)+x) != START) {
                *(*(maze+y)+x) = PATH;
            }
            return 1;
        }
    }

    if(isSafe(maze,x,y-1)){
        if(solve_maze(maze,x,y-1)){
            if(*(*(maze+y)+x) != START) {
                *(*(maze+y)+x) = PATH;
            }
            return 1;
        }
    }

    if(*(*(maze+y)+x) != PATH && *(*(maze+y)+x) != START) {
        *(*(maze+y)+x) = FREE_CORRIDOR;
    }

    return 0;
}

int main() {

    char *input_text = malloc(sizeof(char) * 31);
    if(input_text == NULL){
        printf("Failed to allocate memory");
        return 8;
    }

    printf("Enter filename:");
    scanf("%30s",input_text);

    char **maze;

    int check = load_maze(input_text, &maze);
    if(check == 1 || check == 2){
        printf("Couldn't open file");
        free(input_text);
        return 4;
    }
    if(check == 3){
        printf("File corrupted");
        free(input_text);
        return 6;
    }
    if(check == 4){
        printf("Failed to allocate memory");
        free(input_text);
        return 8;
    }

    if(count_a > 1 || count_b > 1){
        printf("File corrupted");
        free(input_text);
        free_maze(maze);
        return 6;
    }
    if(count_a > 1 || count_b == 0){
        printf("File corrupted");
        free(input_text);
        free_maze(maze);
        return 6;
    }

    int solve = solve_maze(maze,y_a,x_a);

    if(solve == 0 || count_b == 0 || count_a == 0){
        printf("Couldn't find path");
        free(input_text);
        free_maze(maze);
        return 0;
    }




    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < cl; ++j) {
            printf("%c", *(*(maze+i)+j));
        }
        printf("\n");
    }

    free_maze(maze);
    free(input_text);

    return 0;
}
