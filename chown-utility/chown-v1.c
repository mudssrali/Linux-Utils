#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

void doChown(const char *filepath, uid_t uid, gid_t gid);
int isExist(char *source);
gid_t getGID(const char *group_name);
uid_t getUID(const char *user_name);
char getType(int mode_t);
char **tokenize(char *arg);
int main(int argc, char *argv[])
{
        char *option;
        char *source;
        char *userName;
        char *groupName;

        if (argc == 1 || argc == 2)
        {
                if (argc == 1)
                        fprintf(stderr, "chown: missing operand\nTry 'chown --help' for more information.\n");
                if (argc == 2)
                        fprintf(stderr, "chown: missing operand after '%s'\nTry 'chown --help' for more information.\n", argv[1]);
                exit(0);
        }
        else if (argc == 3)
        {
                source = argv[2];
                if (isExist(source) == 0)
                {
                        char **infoToken = tokenize(argv[1]);
                        userName = infoToken[0];
                        groupName = infoToken[1];
                        // Handling cases
                        // visit: https://www.computerhope.com/unix/uchown.htm
                        if (userName != NULL && groupName == NULL)
                        {
                                // case1 "user:" (change only user)
                                uid_t uid = getUID(userName);
                                doChown(source, uid, -1);
                        }
                        else if (userName == NULL && groupName != NULL)
                        {
                                // case2 ":group" (change only group)
                                gid_t gid = getGID(groupName);
                                doChown(source, -1, gid);
                        }
                        else if (userName == NULL && groupName == NULL)
                        {
                                // case3 ":" (Do nothing)
                                doChown(source, -1, -1);
                        }
                        else
                        {
                                // case4 "user:group" (change user and group)
                                uid_t uid = getUID(userName);
                                gid_t gid = getGID(groupName);
                                doChown(source, uid, gid);
                        }
                }
                else
                {
                        fprintf(stderr, "chown: cannot access '%s': No such file or directory", source);
                        exit(0);
                }
        }
        else if (argc == 4)
        {
                source = argv[2];
                option = argv[3];
        }
}
char getType(int mode_t)
{
        if (S_ISDIR(mode_t))
                return 'd';
        else if (S_ISREG(mode_t))
                return '-';
        else if (S_ISLNK(mode_t))
                return 'l';
        else if (S_ISCHR(mode_t))
                return 'c';
        else if (S_ISBLK(mode_t))
                return 'b';
        else if (S_ISSOCK(mode_t))
                return 's';
        else if (S_ISFIFO(mode_t))
                return 'p';
        else
                return '?';
}
void doChown(const char *file_path, uid_t uid, gid_t gid)
{
        if (chown(file_path, uid, gid) == -1)
        {
                fprintf(stderr, "chown: changing ownership of '%s': Operation not permitted\n",file_path);
                exit(0);
        }
} 
uid_t getUID(const char *user_name)
{
        struct passwd *pwd;
        pwd = getpwnam(user_name);
        if (pwd == NULL)
        {
                fprintf(stderr, "chown: failed to get uid against user '%s'\n", user_name);
                exit(0);
        }
        return pwd->pw_uid;
}
gid_t getGID(const char *group_name)
{
        struct group *grp;
        grp = getgrnam(group_name);
        if (grp == NULL)
        {
                fprintf(stderr, "chown: failed to get gid aginst group '%s'\n", group_name);
                exit(0);
        }
        return grp->gr_gid;
}
int isExist(char *source)
{
        struct stat st = {0};
        int rv = 0;
        if (stat(source, &st) == -1)
        {
                rv = -1;
        }
        return rv;
}
char **tokenize(char *arg)
{
        //allocate memory
        char **arglist = (char **)malloc(sizeof(char *) * (3 + 1));
        for (int j = 0; j < 3 + 1; j++)
        {
                arglist[j] = (char *)malloc(sizeof(char) * 12);
                bzero(arglist[j], 12);
        }
        if (arg[0] == '\0') //if user has entered nothing and pressed enter key
                return 0;
        int argnum = 0; //slots used
        char *cp = arg; //pos in string
        char *start;
        int len;
        while (*cp != '\0')
        {
                while (*cp == ':' || *cp == '\t' || *cp == ' ') //skip leading spaces
                        cp++;
                start = cp; //start of the username
                len = 1;
                //find the end of agrgument or groupname
                while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t' || *cp == ':'))
                        len++;
                strncpy(arglist[argnum], start, len);
                arglist[argnum][len] = '\0';
                argnum++;
        }
        arglist[argnum] = NULL;
        return arglist;
}