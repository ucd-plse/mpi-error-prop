typedef int PRUint32;
typedef PRUint32 nsresult;

#define NS_FROZENCALL __cdecl
#define NS_VISIBILITY_DEFAULT
#define NS_EXTERNAL_VIS     NS_VISIBILITY_DEFAULT
#define NS_EXPORT NS_EXTERNAL_VIS
#define NS_EXTERN_C
#define EXPORT_XPCOM_API(type) NS_EXTERN_C NS_EXPORT type NS_FROZENCALL
#define XPCOM_API(type) EXPORT_XPCOM_API(type)



EXPORT_XPCOM_API(nsresult) NS_GetFrozenFunctions() {
  return -5;
}

void callNS_GetFrozenFunctions() {
  nsresult error = NS_GetFrozenFunctions();
  error = 0;
}

int main() {
  callNS_GetFrozenFunctions();
  return 0;
}



