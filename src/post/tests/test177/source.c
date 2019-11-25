
#define __must_check            __attribute__((warn_unused_result))
#define NORET_TYPE    /**/
#define unlikely(x) (!!(x))
#define __always_inline         inline __attribute__((always_inline))
#define MAX_ERRNO	4095
#define IS_ERR_VALUE(x) unlikely((x) >= (unsigned long)-MAX_ERRNO)


static inline void * __must_check ERR_PTR(long error)
{
	return (void *) error;
}

static inline long __must_check  __always_inline PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long __must_check __always_inline IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}



NORET_TYPE void panic(const char * fmt, ...) {}

struct request {
  int result;
  int status;
};


int main() {

  struct request* r = ERR_PTR(-5);
  int x = 0;
  long temp_1, temp_2, temp_3, temp_4;
  
  // forcing it
  temp_1 = IS_ERR(r);
  temp_2 = temp_1; // breaks pattern

  if (temp_2)
    panic("error!");

  int value = r->result;

  r = ERR_PTR(-6);
  // forcing it
  temp_1 = IS_ERR(r);
  temp_2 = temp_1; // breaks pattern
  temp_3 = temp_2; 
  temp_4 = temp_3;
  
  if (! temp_4) {
    value = r->result;
  }
  else {
    panic("error!");
  }

  return 0;
}
