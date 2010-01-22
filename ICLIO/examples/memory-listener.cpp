#include <ICLUtils/ProgArg.h>
#include <ICLIO/XCFMemoryListener.h>

using namespace icl;

int main(int n, char **ppc){
  pa_explain("-m","define memory name (xcf:ShortTerm at default)");
  pa_explain("-x","define xpath to listen on (if not given, all memory events are printed)");
  pa_explain("-e","define event types to listen on. this is a | separated list of REPLACE|INSERT and REMOVE. (At default all event types are used)");
  pa_explain("-t","if this flag is set, a timestamp is printed each time when a document matched the given xpath");
  pa_explain("-n","if this flag is set, documents are printed without the pretty flag");
  pa_explain("-p","print only matching sub locations");
  pa_init(n,ppc,"-m(1) -x(1) -e(1) -t -p");
  
  std::string memName = pa_subarg<std::string>("-m",0,"xcf:ShortTerm");
  std::string xpath = pa_subarg<std::string>("-x",0,"");
  
  std::string events = pa_subarg<std::string>("-e",0,"");
  
  XCFMemoryListener l(memName,xpath,events);
  
  l.setPrintPretty(!pa_defined("-n"));
  l.setPrintTimeStamps(pa_defined("-t"));
  l.setPrintSubLocationsOnly(pa_defined("-p"));
  
  l.run(); // start() would create an extra thread!
}


