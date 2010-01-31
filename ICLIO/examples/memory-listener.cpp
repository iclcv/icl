#include <ICLUtils/ProgArg.h>
#include <ICLIO/XCFMemoryListener.h>

using namespace icl;

int main(int n, char **ppc){
  paex
  ("-m","define memory name (xcf:ShortTerm at default)")
  ("-x","define xpath to listen on (if not given, all memory events are printed)")
  ("-e","define event types to listen on. this is a | separated list of REPLACE|INSERT and REMOVE. (At default all event types are used)")
  ("-t","if this flag is set, a timestamp is printed each time when a document matched the given xpath")
  ("-n","if this flag is set, documents are printed without the pretty flag")
  ("-p","print only matching sub locations");
  painit(n,ppc,"-memory|-m(memory-name=xcf:wb) "
         "-xpath|-x(xpath=/) "
         "-events|-e(string=REPLACE|INSERT|REMOVE) "
         "-print-time-stamps|-t "
         "-print-sub-locations-only|-p");
  
  XCFMemoryListener l(*pa("-m"),*pa("-x"),*pa("-e"));
  
  l.setPrintPretty(!pa("-n"));
  l.setPrintTimeStamps(pa("-t"));
  l.setPrintSubLocationsOnly(pa("-p"));
  
  l.run(); 
}


