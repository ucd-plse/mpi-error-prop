
int writeA(int a, int b) {return 0; }
int writeB(int a, int b) {return 0; }
int writeC(int a, int b) {return 0; }
int writeD(int a, int b) {return 0; }
int write1(int a, int b) {return 0; }
int write2(int a, int b) {return 0; }
int write3(int a, int b) {return 0; }
int write4(int a, int b) {return 0; }
int write5(int a, int b) {return -5; }
int write6(int a, int b) {return 0; }

int hsg_switch;

struct inode_operations
{
    int (*write)(int a, int b);
};

struct inode_operations inode_ops_table [] =  {
    { .write = write4 },
    { .write = write5 },
    { .write = write6 },
};

int main ()
{
    struct inode_operations *i_op;
    int retval;
    int x;
    
    retval = (i_op->write)(55,66);

    return 0; // retval may be out of scope
}
