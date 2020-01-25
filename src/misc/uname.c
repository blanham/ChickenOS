#include <string.h>

#define UNAME_LEN 65

struct uname {
    char sysname[UNAME_LEN];
    char nodename[UNAME_LEN];
    char release[UNAME_LEN];
    char version[UNAME_LEN];
    char machine[UNAME_LEN];
    char domainname[UNAME_LEN];
};

int sys_uname(struct uname *uname)
{
    // TODO: verify uname pointer
    strcpy(uname->sysname, "ChickenOS");
    strcpy(uname->nodename, "Chicken");
    strcpy(uname->release, "0.2.5");
    strcpy(uname->version, "Compiled some time");
    strcpy(uname->machine, "i686");
    strcpy(uname->domainname, "lanham.com");

    return 0;
}