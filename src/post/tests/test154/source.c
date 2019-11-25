// testing variable filtering, transformation OFF

int main() {


  // assigning error constant
  long a = -5; // yes
  int b = -6;  //yes
  float c = -5; // no

  // same type
  long d = a; // yes
  int e = b;  // yes

  // long/int
  long f = b; // yes
  int g = a;  // yes

  // pointers
  int *h; // no
  h = &g; // *h yes
  int *i; // no
  *i = -7; // *i yes

  long *j; // no
  j = &f; // *j yes
  long *k; // no
  *k = -8; // *k yes

  int* l; // no
  *l = 0;
  long* m; // no
  *m = 2;

  void *n = (void *) -5; //no
  void *o = (void*)-6; //no
  
  return 0; // copy: abdefg*h*i*j*k, transfer: de*i*k
}
