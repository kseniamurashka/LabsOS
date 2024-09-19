#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> //getopt

#include <sys/types.h>
#include <string.h>
#include <regex.h>

#define RED "\033[1;91m"
#define RESET "\033[0m"

char *const substr(char *s, size_t pos, size_t count)
{
    static char buf[BUFSIZ];
    memset(buf, '\0', BUFSIZ);
    return strncpy(buf, s + pos, count);
}

void reverse(char s[], int n)
{
    char tmp[BUFSIZ];
    int i, j;
    for (i = 0; i < n; i++)
        tmp[i] = s[i];
    i -= 1;
    for (j = 0; j < n; j++, i--)
        s[j] = tmp[i];
}

int checkFreePattern(char *pattern, char *line, int curpos, int patternStart)
{
    int i = patternStart;
    int count = 0;
    int len = strlen(pattern);

    while (i < len && curpos + count < strlen(line))
    {
        if (pattern[i] == '[')
        {
            int negative = 0;
            int j = i + 1;
            if (i + 1 < len && pattern[i + 1] == '^')
            {
                negative = 1;
                j++;
            }
            char symbols[len];
            int not = 0;
            int symb_pos = 0;
            while (j < len && pattern[j] != ']')
            {
                symbols[symb_pos++] = pattern[j];
                j++;
            }
            if (j == len)
            {
                fprintf(stderr, "grep: Unmatched [, [^, [:, [., or [=\n");
                exit(1); ///////////////////////////////////////
            }
            if (curpos + count < strlen(line) && ((negative == 0 && strchr(symbols, line[curpos + count])) || (negative == 1 && strchr(symbols, line[curpos + count]) == NULL)))
                count++;
            else
            {
                count = 0;
                break;
            }

            i = j + 1;
        }
        else if (pattern[i] == '.')
        {
            if (curpos + count < strlen(line))
                count++;
            else
            {
                count = 0;
                break;
            }
            i++;
        }
        else
        {
            if (curpos + count < strlen(line) && pattern[i] == line[curpos + count])
                count++;
            else
            {
                count = 0;
                break;
            }
            i++;
        }
    }

    if (count != 0)
        return count;
    return 0;
}

int checkFreePatternReverse(char *pattern, char *line, int curpos, int patternStart)
{
    int i = patternStart;
    int count = 0;
    int len = strlen(pattern);

    while (i < len && curpos + count < strlen(line))
    {
        if (pattern[i] == ']')
        {
            int j = i + 1;
            char symbols[len];
            int negative = 0;
            while (j < len && pattern[j] != '[')
            {
                if (pattern[j] == '^' && j + 1 < len && pattern[j + 1] == '[')
                    negative = 1;
                else
                    symbols[j - i - 1] = pattern[j];
                j++;
            }
            if (j == len)
            {
                printf("grep: Unmatched [, [^, [:, [., or [=\n");
                exit(1); ///////////////////////////////////////
            }
            if (curpos + count < strlen(line) && ((negative == 0 && strchr(symbols, line[curpos + count])) || (negative == 1 && strchr(symbols, line[curpos + count]) == NULL)))
                count++;
            else
            {
                count = 0;
                break;
            }
            i = j + 1;
        }
        else if (pattern[i] == '.')
        {
            if (curpos + count < strlen(line))
                count++;
            else
            {
                count = 0;
                break;
            }
            i++;
        }
        else
        {
            if (curpos + count < strlen(line) && pattern[i] == line[curpos + count])
                count++;
            else
            {
                count = 0;
                break;
            }
            i++;
        }
    }
    if (count != 0)
        return count;
    return 0;
}

int lineHasPattern(char *pattern, char *line)
{
    regex_t preg;
    int err, regerr;
    err = regcomp(&preg, pattern, REG_EXTENDED);
    if (err != 0)
    {
        char buff[512];
        regerror(err, &preg, buff, sizeof(buff));
        printf("%s\n", buff);
        return 0;
    }

    regmatch_t pm;
    regerr = regexec(&preg, line, 0, &pm, 0);
    if (regerr == 0)
    {
        regfree(&preg);
        return 1;
    }
    else
    {
        regfree(&preg);
        return 0;
    }
}

int findAnySubstr(char *line, char *pattern, int pattern_start)
{
    int pos = 0;
    int count = 0;
    int len = strlen(pattern);

    while (pos < strlen(line))
    {
        int checking = checkFreePattern(pattern, line, pos, pattern_start);
        if (checking != 0)
        {
            printf("%s", RED);
            for (int i = pos; i < pos + checking; i++)
            {
                printf("%c", line[i]);
            }
            printf("%s", RESET);

            pos += checking;
        }
        else
        {
            printf("%c", line[pos++]);
        }
    }
    printf("\n");
    return 1;
}

int highlightBeginOfStr(char *line, char *pattern, int pattern_start)
{
    int count = checkFreePattern(pattern, line, 0, pattern_start);
    if (count == 0)
        return 0;

    printf("%s", RED);
    for (int i = 0; i < count; i++)
    {
        printf("%c", line[i]);
    }
    printf("%s", RESET);
    for (int i = count; i < strlen(line); i++)
    {
        printf("%c", line[i]);
    }
    printf("\n");

    return 1;
}

int highlightEndOfStr(char *line, char *pattern, int pattern_start)
{
    reverse(pattern, strlen(pattern));
    reverse(line, strlen(line));
    int count = checkFreePatternReverse(pattern, line, 0, pattern_start);
    if (count == 0)
        return 0;

    reverse(pattern, strlen(pattern));
    reverse(line, strlen(line));

    int pos = strlen(line) - count;
    for (int i = 0; i < pos; i++)
    {
        printf("%c", line[i]);
    }
    printf("%s", RED);
    for (int i = pos; i < strlen(line); i++)
    {
        printf("%c", line[i]);
    }
    printf("%s\n", RESET);
    return 1;
}

int highlightBeginOfWord(char *line, char *pattern)
{
    int pos = 0;
    int count = 0;
    int len = strlen(pattern);

    while (pos < strlen(line))
    {
        if (pos != 0 && line[pos - 1] != ' ')
        {
            printf("%c", line[pos++]);
            continue;
        }

        int checking = checkFreePattern(pattern, line, pos, 2);
        if (checking != 0)
        {
            printf("%s", RED);
            for (int i = pos; i < pos + checking; i++)
            {
                printf("%c", line[i]);
            }
            printf("%s", RESET);

            pos += checking;
        }
        else
        {
            printf("%c", line[pos++]);
        }
    }
    printf("\n");
    return 1;
}

void parsePattern(char *pattern, char line[])
{
    if (line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = 0;
    if (lineHasPattern(pattern, line) == 0)
        return;

    size_t len = strlen(pattern);
    if (pattern[0] == '^')
    {
        highlightBeginOfStr(line, pattern, 1);
    }
    else if (pattern[len - 1] == '$')
    {
        highlightEndOfStr(line, pattern, 1);
    }
    else if (len > 1 && ((pattern[0] == '\\' && pattern[1] == '<') || (pattern[0] == '\\' && pattern[1] == 'b')))
    {
        highlightBeginOfWord(line, pattern);
    }
    else if (len > 1 && ((pattern[len - 1] == '>' && pattern[len - 2] == '\\') || (pattern[len - 1] == '\\' && pattern[len - 2] == 'b')))
    {
        strcat(line, " ");
        char *new_p = substr(pattern, 0, strlen(pattern) - 2);
        strcat(new_p, " ");
        findAnySubstr(line, new_p, 0);
    }
    else
    {
        findAnySubstr(line, pattern, 0);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    char *pattern = argv[1];

    if (argc == 2) {
		char* line = NULL;
		size_t len = 0;
		while (getline(&line, &len, stdin) != -1) {
			parsePattern(pattern, line);
		}
		if (line != NULL) {
			free(line);
		}
		return 0;
	}


    char *path_name = argv[2];

    FILE *file = fopen(path_name, "r");
    if (file == NULL)
    {
        printf("Can't open file\n");
        return 0;
    }

    char *line;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1)
    {
        parsePattern(pattern, line);
    }
    if (line != NULL) free(line);
    
    fclose(file);
    return 0;
}
