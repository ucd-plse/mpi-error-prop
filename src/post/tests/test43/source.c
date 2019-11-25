
int main() {

  int err = -5;
  int var1 = err;
  int var2 = var1;
  int var3, var4, var5, var6, var7;

  int number;

  if (number > 0) {
    var3 = var2;
    var4 = var3;
    var3 = 0; /*overwriting error for copy, not for transfer*/
    var6 = var2;
  }
  else {
    var5 = var2;
    var6 = var5;
  }

  var6 = 0; /* overwriting error */

  return 0; /*var4 goes out of scope*/
}

