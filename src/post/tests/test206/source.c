struct address_space_operations
{
    int (*direct_IO) ();
}
c;
struct file_operations
{
    int write;
    int aio_read;
};
int a, f, g, l;
unsigned long b, d, e;
generic_segment_checks (unsigned long *p1)
{
}

generic_file_aio_read ()
{
    generic_segment_checks (&b);
    a = (*c.direct_IO) ();
    b = a;
}

generic_write_checks (unsigned long *p1)
{
}

__generic_file_aio_write ()
{
    generic_segment_checks (&d);
    e = d;
    generic_write_checks (&e);
}

generic_file_aio_write ()
{
    f = __generic_file_aio_write;
}

cifs_user_write (unsigned long p1)
{
    generic_write_checks (&p1);
}

cifs_file_aio_write ()
{
    g = generic_file_aio_write;
}

struct file_operations h = &cifs_user_write;
struct file_operations i = { &generic_file_aio_read, &cifs_file_aio_write };

cifs_writepages ()
{
    l = -1;
    return l;
}

struct address_space_operations k = &cifs_writepages;
