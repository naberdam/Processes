//Nerya Aberdam
//311416457
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>

#define MSG_OF_ERROR fprintf(stderr, "Error in system call\n")
#define PID_PRINT printf("%d\n", history[countCommands].pid)
#define PID_PRINT_WITH_I printf("%d ", history[i].pid)

typedef struct
{
	pid_t pid;
	char* args[100];
	size_t argc;
	int running;
	int isBackground;
	char doneOrRunning[10];
} InfoOfCommands;

InfoOfCommands history[100];

char* cancelBackSlashN(char* strToChange)
{
	int i = 0;
	for (; strToChange[i] != '\0'; ++i) {}
	if (strToChange[i - 1] == '\n') {
		strToChange[i - 1] = '\0';
	}
	return strToChange;
}
//function to delete apostrophe
char* deleteApostrophe(char* strToChange)
{
	int i = 0, j = 0;
	if (strToChange[i] == '"') {
		i = 1;
	}
	char* changedStr = (char*)malloc(strlen(strToChange));
	for (; strToChange[i] != '\0' && strToChange[i] != '"'; ++i, ++j) {
		changedStr[j] = strToChange[i];
	}
	changedStr[j] = '\0';
	return changedStr;
}
//function to delete the unnecessary spaces
char* deleteUnnecessarySpaces(char* strToChange)
{
	int firstApostrophes = 1;
	int i = 0;
	int j = 0;
	char* changedStr = (char*)malloc(strlen(strToChange) + 1);
	for (; strToChange[i] != '\0'; ++i, ++j) {
		if (strToChange[i] == '"' && firstApostrophes) {
			firstApostrophes = 0;
		} else if (strToChange[i] == '"' && !firstApostrophes) {
			firstApostrophes = 1;
		}
		if (strToChange[i] == ' ' && firstApostrophes) {
			while (strToChange[i] == ' ') { ++i; }
			--i;
		}
		changedStr[j] = strToChange[i];
	}
	changedStr[j] = '\0';
	return changedStr;
}
//check the expression that inside the apostrophes
char* expressionInsideApostrophes(char* a_str)
{
	char* strReturn = (char*)malloc(strlen(a_str) + 1);
	int i = 0;
	while (a_str[i] != '\0' && a_str[i] != '"') {
		++i;
	}
	//if there is not one apostrophe
	if (a_str[i] == '\0') {
		strcpy(strReturn, a_str);
		return strReturn;
	}
	++i;
	int j = 0;
	while (a_str[i] != '\0' && a_str[i] != '"') {
		strReturn[j] = a_str[i];
		++i;
		++j;
	}
	strReturn[j] = '\0';
	return strReturn;
}
//function to check if this procces need to be in background or foreground
void isBackgroundOrForeGround(int countCommands)
{
	if (strcmp(history[countCommands].args[history[countCommands].argc - 1], "&") == 0) {
		history[countCommands].isBackground = 1;
		history[countCommands].args[history[countCommands].argc - 1] = NULL;
		--history[countCommands].argc;
	}
}
//check if it is running or done
void checkIfRunning(int countCommands)
{
	int i = 0;
	for (; i < countCommands + 1; ++i) {
		// check if command is still in process.
		if (!(waitpid(history[i].pid, NULL, WNOHANG)) && (history[i].pid != getpid())) {
			history[i].running = 1;
			strcpy(history[i].doneOrRunning, "RUNNING");
		} else {
			//not running
			history[i].running = 0;
			strcpy(history[i].doneOrRunning, "DONE");
		}
	}
}
//check if there is errno
void checkErrno(int valueFromChdir)
{
	if (valueFromChdir) {
		int x = errno;
		printf("%d\n", x);
		if (errno != 0) {
			switch (errno) {
			case ENOENT:
				fprintf(stderr, "Error: No such file or directory\n");
				exit(EXIT_FAILURE);
			case ENOTDIR:
				fprintf(stderr, "Error: Not a directory\n");
				exit(EXIT_FAILURE);
			default:
				fprintf(stderr, "Error: in system call\n");
				exit(EXIT_FAILURE);
			}
		}
		errno = 0;
	}
}
//update the current and previous location
void updateCurrentAndPreviousPath(char* currentPath, char* previousPath, char* tempLast)
{
	strcpy(previousPath, tempLast);
	getcwd(currentPath, 256);
}
//delete tilda from start
void deleteTildaAndSlashFromStart(int countCommands, char* changedStr)
{
	int i = 2;
	int j = 0;
	for (; history[countCommands].args[1][i] != '\0'; ++i, ++j) {
		changedStr[j] = history[countCommands].args[1][i];
	}
}
//function that do the chdir.
void chdirOperation(char* directionForChdir, char* currentPath, char* previousPath, char* tempLast)
{
	int valueFromChdir = chdir(directionForChdir);
	checkErrno(valueFromChdir);
	updateCurrentAndPreviousPath(currentPath, previousPath, tempLast);
}

void chdirOperationForHomeSlash(char* directionForChdir)
{
	int valueFromChdir = chdir(directionForChdir);
	checkErrno(valueFromChdir);
}
//function that responsible of the cd function
void cd(char* currentPath, char* previousPath, int countCommands)
{
	getcwd(currentPath, 1000);
	char tempLast[256] = { 0 };
	strcpy(tempLast, currentPath);
	if (history[countCommands].argc > 2) {
		fprintf(stderr, "Error: Too many arguments\n");
		return;
	}
	// cd or cd ~
	if (history[countCommands].args[1] == NULL ||
		(history[countCommands].args[1][0] == '~' && history[countCommands].args[1][1] == '\0')) {
		char* homeDirection = getenv("HOME");
		chdirOperation(homeDirection, currentPath, previousPath, tempLast);
		//go home
	} else if (history[countCommands].args[1][0] == '~' && history[countCommands].args[1][1] != '\0') {
		char* homeDirection = getenv("HOME");
		chdirOperationForHomeSlash(homeDirection);
		char changedStr[256] = { 0 };
		deleteTildaAndSlashFromStart(countCommands, changedStr);
		chdirOperation(changedStr, currentPath, previousPath, tempLast);
		//go back
	} else if (!strcmp(history[countCommands].args[1], "..")) {
		chdirOperation("..", currentPath, previousPath, tempLast);
		//go to previous location
	} else if (history[countCommands].args[1][0] == '-') {
		chdirOperation(previousPath, currentPath, previousPath, tempLast);
		//otherwise
	} else {
		chdirOperation(history[countCommands].args[1], currentPath, previousPath, tempLast);
	}
}

int isThereTwoApostrophe(char* stringTocheck) {
	int i = 0;
	for (; stringTocheck[i] != '\0'; ++i) {
		//there is Apostrophe
		if (stringTocheck[i] == '"') { return 1;}
	}
	//there is not Apostrophe
	return 0;
}
//parsing string that provides from the user
void parseStringFromUser(int countCommands, char* commandsFromUser)
{
	char* tmp = deleteUnnecessarySpaces(cancelBackSlashN(commandsFromUser));
	char delim[2];
	delim[0] = ' ';
	delim[1] = 0;
	char delimApostrophe[2];
	delimApostrophe[0] = '"';
	delimApostrophe[1] = 0;
	char* expApostrophes = expressionInsideApostrophes(tmp);
	history[countCommands].argc = 0;
	char* token = strtok(tmp, delim);
	//there is chars in token
	while (token) {
		history[countCommands].args[history[countCommands].argc++] = strdup(token);
		token = strtok(0, delim);
		if (token) {
			if (token[0] == '"') {
				token = deleteApostrophe(token);
				strcat(token, " ");
				strcat(token, deleteApostrophe(strtok(0, delimApostrophe)));
			}
		}
	}
	free(tmp);
}

void deleteSpacesFromStart(char* commandsFromUser) {
	int i = 0;
	int j = 0;
	char changedStr[1000] = {0};
	for (; commandsFromUser[i] == ' ' && commandsFromUser[i] != '\0' ; ++i) {}
	for (; commandsFromUser[i] != '\0' ; ++j, ++i) {
		changedStr[j] = commandsFromUser[i];
	}
	changedStr[j] = '\0';
	strcpy(commandsFromUser, changedStr);
}

int main()
{
	int countCommands = 0;
	char path[1000] = { 0 };
	char previousPath[1000] = { 0 };
	getcwd(previousPath, 1000);
	while (1) {
		char commandsFromUser[10000];
		printf("> ");
		fgets(commandsFromUser, 1000, stdin);
		deleteSpacesFromStart(commandsFromUser);
		if (commandsFromUser[0] == '\n') {
			continue;
		}
		parseStringFromUser(countCommands, commandsFromUser);
		history[countCommands].pid = getpid();
		history[countCommands].args[history[countCommands].argc] = NULL;
		isBackgroundOrForeGround(countCommands);
		checkIfRunning(countCommands);
		//exit
		if (!strcmp(history[countCommands].args[0], "exit")) {
			PID_PRINT;
			exit(EXIT_SUCCESS);
			//cd
		} else if (!strcmp(history[countCommands].args[0], "cd")) {
			PID_PRINT;
			cd(path, previousPath, countCommands);
			//jobs
		} else if (!strcmp(history[countCommands].args[0], "jobs")) {
			int i = 0;
			for (; i < countCommands; ++i) {
				int j = 0;
				if (history[i].isBackground) {
					PID_PRINT_WITH_I;
					for (; j < history[i].argc; ++j) { printf("%s ", history[i].args[j]); }
					printf("\n");
				}
			}
			//history
		} else if (!strcmp(history[countCommands].args[0], "history")) {
			int i = 0;
			strcpy(history[countCommands].doneOrRunning, "RUNNING");
			for (; i <= countCommands; ++i) {
				int j = 0;
				if (history[i].argc) {
					PID_PRINT_WITH_I;
					for (; j < history[i].argc; ++j) { printf("%s ", history[i].args[j]); }
					printf("%s\n", history[i].doneOrRunning);
				}
			}
			//anything else
		} else {
			history[countCommands].running = 1;
			pid_t pid = fork();
			if (pid < 0) {
				MSG_OF_ERROR;
				/*sleep(1);*/
			} else if (pid > 0) { //parent
				int status;
				history[countCommands].pid = pid;
				//wait for child
				if (!history[countCommands].isBackground) { waitpid(history[countCommands].pid, &status, 0); }
				//sleep for one second for prompt
				sleep(1);
			} else { //fork returned 0, which means we are in the child process
				history[countCommands].pid = getpid();
				PID_PRINT;
				char pathForExecv[256] = "/bin/";
				strcat(pathForExecv, history[countCommands].args[0]);
				execv(pathForExecv, history[countCommands].args);
				MSG_OF_ERROR;
				//if something go wrong then exit
				exit(1);
			}
		}
		if (history[countCommands].argc > 0) { countCommands++; }
	}
	return 0;
}
