/*
    edline
    2021 (c) A.Higano
----------------------------------------------------
Ver   :0.01
作成日:2021.02.20
作成者:A.Higano
内容  :ラインエデイタedlineを作成
----------------------------------------------------
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 65536
#define FORMAT "%5d:%s"
#define NOT_SET -1
#define SUCCESS 0
#define ERROR_ARGUMENT_NUMBER 1
#define ERROR_CANNOT_OPEN_FILE 2
#define ERROR_CANNOT_CLOSE_FILE 3

char *ErrorMessage[4] = {
    "成功",
    "ファイル名を指定してください(-?でヘルプ)",
    "ファイルを開けませんでした",
    "ファイルをを閉じれませんでした"};

char *HelpMessage =
    "a : 追加(行頭のCtrl+zで終了)\n"
    "d : 削除(「1d」「1-2d」のように行指定)\n"
    "i : 挿入(「1i」のように行指定,行頭のCtrl+zで終了)\n"
    "l : 表示(「2l」「2-5l」のように行指定)\n"
    "q : 終了\n(行番号):編集\n";

char *FileName;
char **Line = NULL;
int LineCount = 0;

void Append();
void Delete(int start, int end);
void Edit(int start);
char GetCommandChar(char *command);
int GetStart(char *command);
int GetEnd(char *command);
void Insert(int start);
int Interact();
void List(int start, int end);
int LoadFile(char *fileName);
int Quit();
int SaveFile(char *fileName);

int main(int argc, char *argv[])
{
    int i;
    int result = SUCCESS;

    if (argc != 2)
    {
        result = ERROR_ARGUMENT_NUMBER;
    }
    else
    {
        if (argv[1][0] == '-')
            switch (argv[1][1])
            {
            case 'v':
                printf(
                    "edline Version 0.01 "
                    "2021 (c) A.Higano  \n");
                return result;

            case 'h':
            case '?':
                printf("ラインエデイタ edline (c) A.Higano 2021 \n\n"
                       "起動方法: edline ファイル名\n"
                       "\nコマンド:\n%s",
                       HelpMessage);
                return result;
            }

        FileName = argv[1];
        result = LoadFile(FileName);
        printf("%s:", FileName);
        if (result == ERROR_CANNOT_OPEN_FILE)
            printf("新しいファイルです\n");
        else
            printf("%d行読み込みました\n", LineCount);
        result = Interact();
    }

    for (i = 0; i < LineCount; i++)
        if (Line[i])
            free(Line[i]);
    if (Line)
        free(Line);

    if (result != SUCCESS)
        printf("%s\n", ErrorMessage[result]);
    return result;
}

int LoadFile(char *fileName)
{
    char buffer[MAX_CHAR];
    char c = '\0';
    FILE *fp;
    int i = 0;
    char **lineInitialValue = NULL;
    int result = SUCCESS;

    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        result = ERROR_CANNOT_OPEN_FILE;
    }
    else
    {
        while (NULL != fgets(buffer, MAX_CHAR, fp))
            LineCount++;
        rewind(fp);

        lineInitialValue = (char **)calloc(LineCount, sizeof(char *));
        Line = lineInitialValue;

        while (c != EOF)
        {
            c = getc(fp);
            if ((c == '\n') || ((c == EOF) && (i != 0)))
            {
                buffer[i] = '\n';
                buffer[i + 1] = '\0';
                *Line = (char *)malloc(i + 2);
                strcpy(*Line, buffer);
                Line++;
                i = 0;
            }
            else
            {
                buffer[i] = c;
                i++;
            }
        }
        if (0 != fclose(fp))
        {
            result = ERROR_CANNOT_CLOSE_FILE;
        }
        Line = lineInitialValue;
    }
    return result;
}

int Interact()
{
    char command[MAX_CHAR];
    char commandChar;
    int start, end;

    while (1)
    {
        printf("%c", '*');
        if (NULL == gets(command))
            continue;
        if (command != "")
        {
            commandChar = GetCommandChar(command);
            start = GetStart(command);
            end = GetEnd(command);

            switch (commandChar)
            {
            case 'q':
                return Quit();
            case 'l':
                if (start == NOT_SET)
                    start = 1;
                if (end == NOT_SET)
                    end = LineCount;
                List(start, end);
                break;
            case 'a':
                Append();
                break;
            case 'i':
                if (start == NOT_SET)
                    Append();
                else
                    Insert(start);
                break;
            case 'd':
                if (start == NOT_SET)
                {
                    printf("行を指定してください\n");
                }
                else
                {
                    if (end == NOT_SET)
                        end = start;
                    Delete(start, end);
                }
                break;
            case 'h':
            case '?':
                printf("%s", HelpMessage);
                break;
            default:
                if (start != NOT_SET)
                    Edit(start);
            }
        }
    }
}

char GetCommandChar(char *command)
{
    char commandChar[] = "adhilqADHILQ?";
    int i;

    for (i = 0; i < strlen(commandChar); i++)
        if (NULL != strchr(command, commandChar[i]))
            return tolower(commandChar[i]);
    return 0;
}

int GetStart(char *command)
{
    char digit[11];
    int i, j = 0;

    if (!isdigit(command[0]))
        return NOT_SET;
    for (i = 0; i < strlen(command); i++)
        if (isdigit(command[i]))
        {
            digit[j] = command[i];
            j++;
        }
        else
            break;

    digit[j] = '\0';
    i = atoi(digit);
    if (i < 1)
        return 1;
    if (i > LineCount)
        return LineCount;
    return i;
}

int GetEnd(char *command)
{
    char digit[11];
    int i, j = 0;
    char *start;

    start = strchr(command, '-');
    if (start == NULL)
        return NOT_SET;

    for (i = 1; i < strlen(start); i++)
        if (isdigit(start[i]))
        {
            digit[j] = start[j];
            j++;
        }
        else
            break;

    if (j == 0)
    {
        return NOT_SET;
    }
    else
    {
        digit[i] = '\0';
        i = atoi(digit);
        if (i < 1)
            return 1;
        if (i > LineCount)
            return LineCount;
        return i;
    }
}

void List(int start, int end)
{
    int i;
    if (LineCount > 0)
    {
        for (i = start; i <= end; i++)
            printf(FORMAT, i, Line[i - 1]);
    }
    else
    {
        printf("表示できる行がありません\n");
    }
}

void Edit(int start)
{
    char line[MAX_CHAR];

    if (LineCount > 0)
    {
        printf(FORMAT, start, Line[start - 1]);
        printf(FORMAT, start, "");
        if (NULL != gets(line))
        {
            strcat(line, "\n");
            free(Line[start - 1]);
            Line[start - 1] = (char *)malloc(strlen(line) + 1);
            strcpy(Line[start - 1], line);
        }
    }
    else
    {
        printf("編集できる行がありません\n");
    }
}
void Append()
{
    int i;
    char line[MAX_CHAR];
    char **oldLine = NULL;

    printf(FORMAT, LineCount + 1, "");
    while (NULL != gets(line))
    {
        strcat(line, "\n");
        LineCount++;
        if (!Line)
        {
            Line = (char **)calloc(LineCount, sizeof(char *));
        }
        else
        {
            oldLine = Line;
            Line = (char **)calloc(LineCount, sizeof(char *));
            for (i = 0; i < LineCount; i++)
                Line[i] = oldLine[i];
            free(oldLine);
        }
        Line[LineCount - 1] = (char *)malloc(strlen(line) + 1);
        strcpy(Line[LineCount - 1], line);
        printf(FORMAT, LineCount + 1, "");
    }
}

void Insert(int start)
{
    int i;
    char line[MAX_CHAR];
    char **oldLine = NULL;

    if (LineCount > 0)
    {
        printf(FORMAT, start, "");
        while (NULL != gets(line))
        {
            strcat(line, "\n");
            LineCount++;
            oldLine = Line;
            Line = (char **)calloc(LineCount, sizeof(char *));
            for (i = 0; i < start - 1; i++)
                Line[i] = oldLine[i];
            Line[start - 1] = (char *)malloc(strlen(line) + 1);
            strcpy(Line[start - 1], line);
            for (i = start; i < LineCount; i++)
                Line[i] = oldLine[i - 1];
            free(oldLine);
            start++;
            printf(FORMAT, start, "");
        }
    }
    else
    {
        Append();
    }
}

void Delete(int start, int end)
{
    int i;
    int howManyLines = end - start + 1;
    char **oldLine = NULL;

    if (howManyLines > 0)
    {
        if (LineCount > 0)
        {
            LineCount = LineCount - howManyLines;
            oldLine = Line;
            if (LineCount == 0)
                Line = NULL;
            else
                Line = (char **)calloc(LineCount, sizeof(char *));
            for (i = 0; i < start; i++)
                Line[i] = oldLine[i];
            for (i = start - 1; i < end; i++)
                free(oldLine[i]);
            for (i = start - 1; i < LineCount; i++)
                Line[i] = oldLine[i + howManyLines];
            free(oldLine);
        }
        else
        {
            printf("削除できる行がありません\n");
        }
    }
}
int Quit()
{
    char command;
    int result = SUCCESS;

    printf("保存しますか？(Y/N):\n");
    command = tolower(getchar());
    if (command == 'y')
        result = SaveFile(FileName);
    return result;
}
int SaveFile(char *fileName)
{
    FILE *fp;
    int i;
    int result = SUCCESS;

    if (LineCount > 0)
    {
        fp = fopen(fileName, "w");
        if (fp == NULL)
        {
            result = ERROR_CANNOT_OPEN_FILE;
        }
        else
        {
            for (i = 0; i < LineCount; i++)
                fputs(Line[i], fp);
            if (0 != fclose(fp))
                result = ERROR_CANNOT_CLOSE_FILE;
        }
    }
    else
    {
        remove(fileName);
    }
    return result;
}
