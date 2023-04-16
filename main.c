#include <ncurses.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>


typedef struct {
    enum {
        REGULAR,
        DIRECTORY,
    } kind;
    char *name;
} File;

File *newfile(int kind, char *name) {
    File *file = malloc(sizeof(File));
    file->kind = kind;
    file->name = name;
    return file;
}

File **listdir(void) {
    DIR *directory = opendir(".");
    struct dirent *dir;
    int index = 0;
    File **files = malloc(sizeof(char));
    while ((dir = readdir(directory)) != NULL) {
        int kind;
        if (dir->d_type == DT_REG) kind = REGULAR;
        else kind = DIRECTORY;
        char *name = malloc(sizeof(dir->d_name));
        strcpy(name, dir->d_name);
        files[index++] = newfile(kind, name);
        files = realloc(files, (sizeof(files) + sizeof(File)) * sizeof(File));
    }
    closedir(directory);
    return files;
}

void setup(void) {
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, 1);
}

void draw(void) {
    bool quit = false;
    int cursor = 1, y = 4, x = 5, maxy, maxx, longername = 0;
    getmaxyx(stdscr, maxy, maxx);
    File **files = listdir();
    char home[PATH_MAX];
    getcwd(home, sizeof(home));
    while (!quit) {
        clear();
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        mvaddstr(y - 3, x, cwd);
        mvaddstr(maxy - 3, 1, "Q/ESC: exit, H: home, N: new, Enter: open, C: copy, R: rename, E: edit, P: prompt");
        for (int i = 1; files[i] != NULL; i++) {
            if (longername < strlen(files[i]->name)) longername = strlen(files[i]->name);
            if (y > maxy - 1) y = 4, x = maxx - longername;
            if (i == cursor) mvaddch(y, x - 3, '>');
            mvaddstr(y, x, files[i]->name);
            if (strcmp(files[i]->name, "..") == 0)
                mvaddstr(y, x + 4, "(go back)");
            if (files[i]->kind == DIRECTORY)
                mvaddch(y, x + strlen(files[i]->name), '/');
            y += 2;
        }
        y = 4, x = 5;
        int key = getch();
        switch (key) {
            case 27:
            case 'q': {
                quit = true;
                break;
            }
            case KEY_UP:
                if (cursor == 1) break;
                cursor--;
                break;
            case KEY_DOWN: {
                int length = 0;
                for (; files[length] != NULL; length++);
                if (cursor != length - 1) cursor++;
                break;
            }
            case 10: {
                const char *target = files[cursor]->name;
                if (files[cursor]->kind != DIRECTORY) break;
                chdir(target);
                files = listdir();
                cursor = 1;
                break;
            }
            case 'h': {
                chdir(home);
                files = listdir();
                cursor = 1;
                break;
            case 'e':
                if (files[cursor]->kind != REGULAR) break;
                char *command = malloc(5 + strlen(files[cursor]->name));
                sprintf(command, "vi %s", files[cursor]->name);
                system(command);
                break;
            }
            case 'c': {
                mvprintw(maxy - 1, 1, "copy (%s): ", files[cursor]->name);
                curs_set(1);
                echo();
                char name[PATH_MAX];
                int key = 0, index = 0;
                while ((key = getch()) != 10) name[index++] = key;
                mvaddstr(maxy - 2, 1, "are you sure? (y/n): ");
                key = getch();
                if (key != 'y') {
                    noecho();
                    curs_set(0);
                    break;
                }
                noecho();
                curs_set(0);
                char *command = malloc(4 + strlen(files[cursor]->name) + strlen(name));
                sprintf(command, "cp %s %s", files[cursor]->name, name);
                system(command);
                files = listdir();
                break;
            }
            case 'r': {
                mvprintw(maxy - 1, 1, "rename (%s): ", files[cursor]->name);
                curs_set(1);
                echo();
                char name[PATH_MAX];
                int key = 0, index = 0;
                while ((key = getch()) != 10) name[index++] = key;
                mvaddstr(maxy - 2, 1, "are you sure? (y/n): ");
                key = getch();
                if (key != 'y') {
                    noecho();
                    curs_set(0);
                    break;
                }
                noecho();
                curs_set(0);
                char *command = malloc(4 + strlen(files[cursor]->name) + strlen(name));
                sprintf(command, "mv %s %s", files[cursor]->name, name);
                system(command);
                files = listdir();
                break;
            }
            case 'd': {
                curs_set(1);
                echo();
                mvaddstr(maxy - 2, 1, "are you sure? (y/n): ");
                key = getch();
                if (key != 'y') {
                    noecho();
                    curs_set(0);
                    break;
                }
                noecho();
                curs_set(0);
                char *command = malloc(6 + strlen(files[cursor]->name));
                sprintf(command, "rm -r %s", files[cursor]->name);
                system(command);
                files = listdir();
                int i = 0;
                for (; files[i] != NULL; i++);
                if (cursor > i - 1) cursor--;
                break;
            }
            case 'n': {
                mvaddstr(maxy - 1, 1, "new: ");
                curs_set(1);
                echo();
                char name[PATH_MAX];
                int key = 0, index = 0;
                while ((key = getch()) != 10) name[index++] = key;
                mvaddstr(maxy - 2, 1, "are you sure? (y/n): ");
                key = getch();
                if (key != 'y') {
                    noecho();
                    curs_set(0);
                    break;
                }
                noecho();
                curs_set(0);
                char *command = malloc(6 + strlen(name));
                sprintf(command, "touch %s", name);
                system(command);
                files = listdir();
                break;
            }
            case 'p': {
                clear();
                mvaddstr(maxy - 1, 1, "prompt: ");
                curs_set(1);
                echo();
                refresh();
                char *command = malloc(sizeof(char));
                int key = 0, index = 0;
                while ((key = getch()) != 10) command[index++] = key;
                system(command);
                getch();
                noecho();
                curs_set(0);
                files = listdir();
                break;
            }
        }
        refresh();
    }
}

int main(void) {
    setup();
    draw();
    endwin();
    return 0;
}
