#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

class FOO {
public:
    int foo;   
    FOO() {
        foo = 123;
        printf ("FOO::FOO(%p);\n", this);
    }
    ~FOO() {
    }
};

FOO g_foo;

int main ( int argc, char *argv[] ) {
    FOO* foo = new FOO;   
    int pid = (int)getpid();
    for (int i=0;i<pid+3;i++) {
        printf ("  PID[%d]: Hello again! counter=%d, foo=%d\n", pid, i, g_foo.foo++);
       // usleep((useconds_t) (1000000));
    }
    delete foo;
    return (int)0xBADC0DE0+ (int)pid;
}
