#include "create_combinations.h"
#include "show_arglists.h"

#include <QProcess>
#include <iostream>


void run_command_at_host(const std::string &host, const std::string &cmd){
  string fullcmd = "ssh ";
  fullcmd += host + " \"" +cmd + "\"";
  QProcess p;
  cout << "full cmd is: '" << fullcmd << '\'' << endl;
  p.start(fullcmd.c_str());
  p.waitForFinished(60*1000);
  QByteArray a = p.readAllStandardOutput();
  cout << "read this data from process" << endl << a.data() << endl;
}

int main(int n, char **ppc){
  //std::vector<arglist> l = create_combinations();
  //show_arglists(l,false);
  
  run_command_at_host("priamos.techfak.uni-bielefeld.de","find projects");

  return 0;
}
