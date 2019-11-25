static inline void * ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void * ptr)
{
	return (long) ptr;
}

static char *foo()
{
	return ERR_PTR(-5);
}

int main() 
{
	int err = PTR_ERR(foo());	
	return 0;
}
