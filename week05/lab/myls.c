// myls.c ... my very own "ls" implementation

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
# include <bsd/string.h>
#endif
#include <sysexits.h>
#include <unistd.h>

#define MAXDIRNAME 256
#define MAXFNAME 256
#define MAXNAME 24

char *rwxmode (mode_t, char *);
char *username (uid_t, char *);
char *groupname (gid_t, char *);

int main (int argc, char *argv[])
{
	// string buffers for various names
	char uname[MAXNAME+1]; // UNCOMMENT this line
	char gname[MAXNAME+1]; // UNCOMMENT this line
	char mode[MAXNAME+1]; // UNCOMMENT this line
	char fullpath[MAXFNAME];

	// collect the directory name, with "." as default
	char dirname[MAXDIRNAME] = ".";
	if (argc >= 2)
		strlcpy (dirname, argv[1], MAXDIRNAME);

	// check that the name really is a directory
	struct stat info;
	if (stat (dirname, &info) < 0)
		err (EX_OSERR, "%s", dirname);

	if (! S_ISDIR (info.st_mode)) {
		errno = ENOTDIR;
		err (EX_DATAERR, "%s", dirname);
	}

	// open the directory to start reading
	DIR *df; // UNCOMMENT this line
	df = opendir(dirname);
   	if (df == NULL) {
     	printf("Cannot open directory %s\n", argv[1]);
     	return 1;
   	}

	// read directory entries
	struct dirent *entry; // UNCOMMENT this line
   	while((entry = readdir(df))!=NULL){
    	if(entry->d_name[0] == '.') {
			continue;
		}
    	strcpy(fullpath, dirname);
    	strcat(fullpath, "/");
    	strcat(fullpath, entry->d_name);
    	lstat(fullpath, &info);
    	printf("%s  %-8.8s %-8.8s %8lld  %s\n",rwxmode(info.st_mode, mode),username(info.st_uid, uname),groupname(info.st_gid, gname),(long long)info.st_size,entry->d_name);
   }

	// finish up
	 closedir(df); // UNCOMMENT this line

	return EXIT_SUCCESS;
}

// convert octal mode to -rwxrwxrwx string
char *rwxmode(mode_t mode, char *str)
{
   
    for(int i = 0; i < 10; i++){
	  str[i]= '-';
    }   

    if ( S_ISDIR(mode) ) {
      str[0] = 'd';    // directory  
	}   
    else if ( S_ISLNK(mode) )	{
      str[0] = 'l';    //symbolic link
	}
    if ( mode & S_IRUSR ) {
		str[1] = 'r';   
	}
    if ( mode & S_IWUSR ) {
		str[2] = 'w';
	}
    if ( mode & S_IXUSR ) {
		str[3] = 'x';
	}
    if ( mode & S_IRGRP ) {
		str[4] = 'r';
	}   
    if ( mode & S_IWGRP ) {
		str[5] = 'w';
	}
    if ( mode & S_IXGRP ) {
		str[6] = 'x';
	}
    if ( mode & S_IROTH ) {
		str[7] = 'r';
	}    
    if ( mode & S_IWOTH ) {
		str[8] = 'w';
	}
    if ( mode & S_IXOTH ) {
		str[9] = 'x';
	}
    str[10] = '\0';
    return str;
   
}

// convert user id to user name
char *username (uid_t uid, char *name)
{
	struct passwd *uinfo = getpwuid (uid);
	if (uinfo != NULL)
		snprintf (name, MAXNAME, "%s", uinfo->pw_name);
	else
		snprintf (name, MAXNAME, "%d?", (int) uid);
	return name;
}

// convert group id to group name
char *groupname (gid_t gid, char *name)
{
	struct group *ginfo = getgrgid (gid);
	if (ginfo != NULL)
		snprintf (name, MAXNAME, "%s", ginfo->gr_name);
	else
		snprintf (name, MAXNAME, "%d?", (int) gid);
	return name;
}
