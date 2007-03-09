#include <iclTestImages.h>
#include <iclConverter.h>
#include <vector>
#include <string>
#include <map>
#include <iclImg.h>
#include <iclSize.h>
#include <iclFileWriter.h>
#include <stdlib.h>

using std::vector;
using std::string;
using std::map;
using namespace icl;

namespace {
  struct XPMColor{ 
    // {{{ open

  XPMColor(int r=0, int g=0, int b=0):
    r(r),g(g),b(b){}
  int r,g,b;
};

// }}}
  
  typedef map<string,XPMColor> colormap;
  typedef map<char,int> charmap;
  typedef vector<string> strvec;
  
  colormap g_mapXPMColors;
  charmap g_mapHexLut;
  
  void initColorMap(){
    // {{{ open

  static int done=0;
  if(done)return;
  done = 1;
  g_mapXPMColors["black"]=XPMColor(0,0,0);
  g_mapXPMColors["darkSlateGrey"]=XPMColor(47,79,79);
  g_mapXPMColors["slateGrey"]=XPMColor(112,128,144);
  g_mapXPMColors["grey"]=XPMColor(190,190,190);
  g_mapXPMColors["gray"]=XPMColor(190,190,190);
  g_mapXPMColors["gray100"]=XPMColor(240,240,240);
  g_mapXPMColors["gainsboro"]=XPMColor(220,220,220);
  g_mapXPMColors["white"]=XPMColor(255,255,255);

  g_mapXPMColors["violet"]=XPMColor(238,130,238);
  g_mapXPMColors["magenta"]=XPMColor(255,0,255);
  g_mapXPMColors["maroon"]=XPMColor(176,48,96);
  g_mapXPMColors["purple"]=XPMColor(160,32,240);

  g_mapXPMColors["firebrick"]=XPMColor(178,34,34);
  g_mapXPMColors["red"]=XPMColor(255,0,0);
  g_mapXPMColors["tomato"]=XPMColor(255,99,71);
  g_mapXPMColors["orange"]=XPMColor(255,165,0);
  g_mapXPMColors["gold"]=XPMColor(255,215,0);
  g_mapXPMColors["yellow"]=XPMColor(255,255,0);

  g_mapXPMColors["lemonChiffon"]=XPMColor(255,250,205);
  g_mapXPMColors["lightYellow"]=XPMColor(255,255,224); 
  g_mapXPMColors["wheat"]=XPMColor(245,222,179);
  g_mapXPMColors["tan"]=XPMColor(210,180,140);
  g_mapXPMColors["khaki4"]=XPMColor(139,134,78);
  g_mapXPMColors["darkKhaki"]=XPMColor(139,134,78);

  g_mapXPMColors["sandyBrown"]=XPMColor(244,164,96);
  g_mapXPMColors["peru"]=XPMColor(205,133,63);
  g_mapXPMColors["chocolate"]=XPMColor(210,105,30);
  g_mapXPMColors["sienna"]=XPMColor(160,82,45);

  g_mapXPMColors["lightSeaGreen"]=XPMColor(32,178,170);  
  g_mapXPMColors["darkGreen"]=XPMColor(0,100,0);  
  g_mapXPMColors["seaGreen"]=XPMColor(46,139,87);  
  g_mapXPMColors["limeGreen"]=XPMColor(50,205,50);  
  g_mapXPMColors["green"]=XPMColor(0,255,0);  
  g_mapXPMColors["paleGreen"]=XPMColor(152,251,152);
  
  g_mapXPMColors["navyBlue"]=XPMColor(0,0,128);
  g_mapXPMColors["navy"]=XPMColor(0,0,128);
  g_mapXPMColors["blue"]=XPMColor(0,0,255);
  g_mapXPMColors["dodgerBlue"]=XPMColor(30,144,255);
  g_mapXPMColors["skyBlue"]=XPMColor(135,206,235);
  g_mapXPMColors["lavender"]=XPMColor(230,230,250);
  g_mapXPMColors["cyan"]=XPMColor(0,255,255);
}

// }}}
  
  void initHexLut(){
    // {{{ open

  static int done=0;
  if(done)return;
  done = 1;
  char _c[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  for(int i=0;i<16;i++) g_mapHexLut[_c[i]]=i;  
}

// }}}
  
  XPMColor getHexColor(string s,bool &ok){
    // {{{ open
    if(s.length()==6){
    int ai[6]={0,0,0,0,0,0};
    for(int i=0;i<6;i++){
      if(g_mapHexLut.find(s[i]) != g_mapHexLut.end()){
        ai[i] = g_mapHexLut[s[i]];
      }else{
        ok=0;
        return XPMColor();
      }
    }
    ok = 1;
    return XPMColor(ai[1]+16*ai[0],ai[3]+16*ai[2],ai[5]+16*ai[4]);
  }
  ok=0;
  return XPMColor();
}

  // }}}
  
  XPMColor getGrayColor(string s, bool &ok){
    // {{{ open

    if(s.find("gray",0)== 0){
      string n = &(s[4]);
      if(n.length() > 0){
        int g = atoi(n.c_str());
        ok = 1;
        return XPMColor(g,g,g);
      }
    }    
    ok = 0;
    return XPMColor();
  }

  // }}}
  
  XPMColor getColor(string s){
    // {{{ open

  initColorMap();
  initHexLut();
  bool ok;
  XPMColor c;
  
  if(s.length() < 2) return XPMColor(0,0,0);
  if(s[0]=='#'){
    s=&(s[1]);
  }
  if(g_mapXPMColors.find(s) != g_mapXPMColors.end()){
    return g_mapXPMColors[s];
  }
  c = getHexColor(s,ok);
  if(ok)return c;
  c = getGrayColor(s,ok);
  if(ok) return c;
 
  printf("no color found for [%s] \n",s.c_str());
  return XPMColor();    
}

// }}}
  
  static char *ppc_woman_xpm[] = {
    // {{{ open

  "63 60 14 1",
  "  c black",
  ". c cyan",
  "X c #800000",
  "o c #804000",
  "O c #808000",
  "+ c #FF8000",
  "@ c yellow",
  "# c #808080",
  "$ c #8080FF",
  "% c #80FF80",
  "& c #FF8080",
  "* c #FFFF80",
  "= c #C0C0C0",
  "- c gray100",
  "$$$$$$$ooooooo++ooooooooooooooooooooooooo++ooooooo$$$$$$$$$$$$$",
  "$$$$$$ooooo+++ooooooooooooooooooooooooooooo+++ooooo$$$$$$$$$$$$",
  "$$$$$$ooo++ooooo+oooooo=o==++=o==ooooooo+ooooo++ooo$$$$$$$$$$$$",
  "$$$$$ooo++oo  ooooooo==++++++++++++=ooooooo  oo++ooo$$$$$$$$$$$",
  "$$$$$oo+ooo    oooo=+++++++++++++++++=oooo    ooo+oo$$$$$$$$$$$",
  "$$$$oooooo     oo=++++++*++++++++++++++=oo     oooooo$$$$$$$$$$",
  "$$$oooooo      o+++*+*+*+*++*+++++++++++=o      oooooo$$$$$$$$$",
  "$$$ooooo      o=++*+*+*+*+*++++++++++++++=o      ooooo$$$$$$$$$",
  "$$ooooo      o=++*+*+*+*+*++++++++++++++++=o      ooooo$$$$$$$$",
  "$$ooooo     o=++*+*+*+*+*++++++++++++++++++=o     ooooo$$$$$$$$",
  "$oooooo    o=++*+*+*+*+++++++++++++++++++++=oo    oooooo$$$$$$$",
  "$oooo+    o=++*+*+*+++++*+++++++++++++++++++=oo    +oooo$$$$$$$",
  "$ooo+    oo++*+*+*++++++++++++++++++++++++==+=oo    +ooo$$$$$$$",
  "$ooo+    o=+*+*+*+++*++++++++++++++++++++++===oo    +ooo$$$$$$$",
  "oooo+     +++*+*++++++++++++++++++++++++++====$     +oooo$$$$$$",
  "ooo+oo    ++++*++++++++++++++++++++++++=o=====$    oo+ooo$$$$$$",
  "ooo+oo    ++*+==ooooO++++++++++++++=OoooooO===$    oo+ooo$$$$$$",
  "ooo+o     ++++=======ooo++++++++=ooooooooooooo$$    o+ooo$$$$$$",
  "ooo+o    =+++++++=======o++++++=ooooo$$$$$oooO=$    o+oooo$$$$$",
  "oo+oo    =+++++++++++++==o++++oooo=$%$%========+    oo+ooo$$$$$",
  "oo+oo    =++++++++++++++=*=+++oO$%=====+=++++=$o    oo+ooo$$$$$",
  "oo+oo    =+++oooooooooo++=*+++o$$=oooooooooo+=$o    oo+ooo$$$$$",
  "oo+oo    o++oo-  -   #ooo+=*++O$ooo  --   -oo=$o    oo+ooo$$$$$",
  "oo+oo     ++o-- --   ---oo*+++Ooo--- -    --o=$     oo+oo$$$$$$",
  "oo+oo     +ooo- -    -oooo+*++Ooooo-      -ooo$     oo+oo$$$$$$",
  "oo+ooo    ++++o-    --o+++*+++O%===--    -o+++$    ooo+oo$$$$$$",
  "oo+ooo    =++++ooo--oo+++++*++=$==+oo--ooo++==$    ooo+oo$$$$$$",
  "oo+o      o++++++oooo+++++*+++=$++++oooo+++=+=$      o+oo$$$$$$",
  "ooo       o++++++++++++++++++++$+++++++++++++$$       ooo$$$$$$",
  "ooo        =+++++++++++++++++++=++++++++++===%        oooo$$$$$",
  "oooo       =+++*+++++++++++++++++++++++++++++$       ooooo$$$$$",
  "oo+oooo    o++++*&*+++++++++++++++++++++++==$o  X oooo+ooo$$$$$",
  "o++oooo     +++*&*&*++++++++++oo+++++++++===$  X  oooo++oo$$$$$",
  "o+ooooo     =+++*&*&+++++++oo=$$+++++++++=+=%  X  ooooo+oo$$$$$",
  "o+oooooo    o++*&*&*++++++o=$$.=++++++++==+%o  X oooooo+oo$$$$$",
  "o+oooooo     +++*&*+*+++++++o+o+++++++++=+=$   X oooooo+oo$$$$$",
  "o+oo  oo     =+++*+*++++++++++++++++++++=+=%   X oo  oo+oo$$$$$",
  "o+o    oo    o++++*+*++++++++++++++++++===%o   Xoo   =o+oo$$$$$",
  "o+o    oo    o=++*+*+++++++++++++++++++++=$o   Xoo    o+oo$$$$$",
  "ooo    oo     o+++*+*++++*ooooo+++++++=+==o   = oo    oooo$$$$$",
  "ooo    ooo     =+++*+++ooooo%$$$$o++++==$o    =ooo    oooo$$$$$",
  "ooo     oo      ++++*++++o$$$%oo*++++=+=%    ==oo     oooo$$$$$",
  "oooo    o+o     =++++++++++*+*+*++++++=$    X o+o X  ooooo$$$$$",
  "oooo    oo+o     =+++*++++++++++++++=+=$   XXo+oo X  =ooo$$$$$$",
  "oooo    =o+oo     ++++++++++$%+++++===%   X oo+ooX  =oooo$$$$$$",
  "$ooo    o=o+oo    =++++++++oooo+++=++$   X oo+ooo   =ooo$$$$$$$",
  "$ooo     =oo+o     =++++++**+*+++++==   X  o+ooo   ==ooo$$$$$$$",
  "$oooo    =oo+oo     =+++++++++++++==$ XX =oo+=o  XX oooo$$$$$$$",
  "$ooooo   =  o+oo     =++++++++++=+=$ XX X=o+==   X =oooo$$$$$$$",
  "$ooooo   =   o+oo    +=++++++++=+=$oX XX=o+==   X  =oooo$$$$$$$",
  "$ooooo   =   o+oo    oo=+++++++=$$oX XX =o+=    X =oooXo$$$$$$$",
  "$oooooo ==    o+o    oooo==+=$$$ooXXX   =+=   XX ==oooXo$$$$$$$",
  "$oooooo       oo+o  ooo+XXooooooXXXoo  o===   X = ooXXXo$$$$$$$",
  "$$oooooo   =    +oooooo++XXXXXXXXXoooooo==     ==ooooXXX$$$$$$$",
  "$$ooooooo  =      oooo++++XXXXXXooooooo XXXX  = ooooooXX$$$$$$$",
  "$$$ooooooo         ooo+++++++XXXooooXXXX= XX ==oooooXXXX$$$$$$$",
  "$$$Xooooooo        ooo++@+++++++Xooooo  XX   =oXXXXX   X$$$$$$$",
  "$$$$$$ooooo        ooo+++@+++++++oooooXXX X   XXXXX     X$$$$$$",
  "$$$$$$oooooo       ooo+++++++++ooooooXXXXXXXXXXXXXX      X$$$$$",
  "$$$$$$$oo$$$$     oooooooo++++++ooooooo XXXXXXXXXXXX     X$$$$$"
};

// }}}
  
  static char *ppc_tree_xpm[] = {
    // {{{ open

  "63 60 11 1",
  "  c black",
  ". c navy",
  "X c blue",
  "o c #008000",
  "O c green",
  "+ c cyan",
  "@ c #804000",
  "# c red",
  "$ c #FF8000",
  "% c yellow",
  "& c gray100",
  "++++++++++++++++++++++++++++++++++oooooooooooooooo+++++++++++++",
  "+++++++++++++++++++++++++++++++o+ooooOoOooooooooooo+++++++++&&+",
  "+++&&&++++++++++++++++++++++ooooOoOOOOOOOOOoo%OoOooo+++++++&&&&",
  "++&&&&&+++++++++++++++++ooooooOoOOoOoOOOOOO%OoOOOOoooo+++++&&&&",
  "+&&&&&&+++++++++++++++oOooooOooO%oOoOOOOOO%OO%OoOOooooo+++++&&+",
  "+&&&&&++++++oooooooOOooOoOooOOOO%OO%OO%%OOOO%OO%%OOooooo+++++++",
  "++&&&+++++ooooooooOOOOO%OOOOO%%OO%O%%%O%%OO%%%%%ooooOoooo++++++",
  "+++++++oooooooOoOoOOOOOoOOOOOo%%O%%OOO%O%O%%OO%oOOOooOoooo+++++",
  "+++++++ooooooOOOO$OOOO%O%OoOoo%%o%%%%OO%%OOO%O%%OOoooOOoooo++++",
  "++++++oooooo$oOoOOOO%OOO%O%OooO%OOO%%%%%OOOOOO%OoOOOOOOooooo+++",
  "+++++ooo$$oOO%O%OOOO%OOO%o%OO%OO%%o%%O%OOOOOOoOOOOOOooOoooooo++",
  "++++oooOOOoO%OOOoOOO%O%OOOO%OO%OOOOOo%%%OOOoooooOooOOOOooooooo+",
  "+++oOOOOOOO%O%OO%%OOO%OOO%OOOOOOOOoOo%%OooOOOoOooOooOOoOOooooo+",
  "++oooO$OOOOOOO%O%OOO%O%O%%ooOOOOoOooooooOOOooooooooooOOoooooooo",
  "++ooOOoOOoO%O%%%%%OoOoooo%ooOoOOOOoooOoOOoOoooooooooOOOoOO&oooo",
  "+oooOOooOoOO%OOoOOooooo%ooooooOOOooooOoOoOOoooOOoOooOOOOoOOOooo",
  "+ooooOOOOOOoOOOOOOoOoooooooooooOOOooooooooooooooooooOOoOOooOooo",
  "++oooOOOOOOOoo%o%OOOoooooooOoOoOOOoooOooooOooOOOoooOOOoOoOOOooo",
  "+++oooooOOooo%ooooOOooOoooooooOOOOoooooooOooOOOOOooooOoOOoOoo++",
  "+++++oooooooooooooOoooooooooooOoOOoooooooooOOOoOOOooooOoOOOooo+",
  "++++++++++ooooooooooooooooo oooooooooooooooOOoOOOOoooOo&OOOOoo+",
  "+++&&&&&++++++++ooooo@      +++++oooooooooOoOOoOoooooOooooOoo++",
  "++&&&&&&&++++&&++++++@@      @+++++ooooooooooOOoooooOOOOOOooo++",
  "+&&&&&&&&&&&&&&&++++++@@@ @   @+++++@ooooooooOoooooooOooooooo++",
  "+&&&&&&&&&&&&&&&+++&&&++@@@ @   @++  @ooooooooooooOoooOoooooo++",
  "+&&&&&&&&&&&&&&&&&&&&&&+++@ @@@  @@ @@@ooooooooOoOo&OoOoooooo++",
  "&&&&&&&&&&&&&&&&&&&&&&&++++ +@@@@@@@  @++ooooooooooOooooooooo++",
  "&&&&&&&&&&&&&&&&&&&&&&&&++++++@@$@@@ @++++oooooooOOooOooooooo++",
  "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&@$@@@ @&&&&&&ooooooOOOOoooooo&&&",
  "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&$@@@@@@&&&&&&&ooooooooooooooo&&&",
  "&&&&&&&&&&&&&&&&&&&&&&&&&&&&+@$$$$@@ @&&&&&&&&oooooooooooooo&&&",
  "+++++++++++++++++++++++++++@@@$$@@@@ @+++++++++oooooooooooo++++",
  "+++++++++++++++++++++++++@@$$@$$@@@  @+++++++++++ooooooooo++&&+",
  "+++++++++++++++++++++++@@$$$@$$@@@@ @&oooo++++++++ooooooo++++++",
  "++++++++++++++++++++++@@$$@@$$@@@  @&&oOOOoo++++++++oooo+++++++",
  "XXXXXXXXXXXXXXXXXXXX@@@$$@$$@@@@  @@@&oOOOOOoo+++++++++++++++++",
  "++++++++++++++++++@@@@$$@@$@@@XXXXXXX@@@oooOOoooXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXooooooo@@@@@@@@@@+++++@@@@oooooooo+++++++++++++++",
  "X+++++++++++oooooooooo@@o@@@@@@XXXXXXXXX+oooooooXXXXXXXXXXXXXXX",
  "XXXXXXXXXXooooooooooooooooo@@@+++++++++++++++ooo+++++++++++++++",
  "XXXXXXXXoooooooooooooooooooo@@XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXoooooooooooooooooooooo@@XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXooooooooooooooooooooooooo@@XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  ".XXXXooooooooooooooooooooooooooXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "..XoooOoooooooooooooOooooooooooooXXXX..XXXXXXXXXXXXXXXXXX......",
  "..oooooooooooooooooOOoooooooooooooXX...........................",
  ".ooooooooooooo#ooooooooooOoooooooooo...........................",
  "oooOoooo##ooo#%#ooooooooooooOoooooooo..........................",
  "ooooooooooOooo#OoooOoooooooooooooooooo.........................",
  "oooooOoooOoooooOOoooooooooOoooOooOooooooo......................",
  "OoOoooOoooooooOoOoOooooooooooooooooooooooo.....................",
  "OoOooOOoooOOoOooOOooo##oooOOooooooooooooooooooooooooooooX......",
  "ooOOoOOO##O##OoOOOOo#%%#ooOoooooooooooooooooooooooooooooooooo..",
  "OoOOOOO##%####OOOOOO#%##oOooooOOOoooooooooooooooooooooooooooooo",
  "OOOOOOO##%$$##OOOOOOO##oOOoOoooOooooOoooooooooooooooooooooooooo",
  "OOOOOOOO##$$#OOOOoOoooOooOOOoooOoooooOooOOooooooooooooooooooooo",
  "OOOOOOooO#$$#OOOOOoOooOooooOOoooOOoooOoooOooOoooooooooooooooooo",
  "OOOOOOooo####oOOOOOooOooooooOOoOoooooooOoooooOooooooooooooooooo",
  "OOOOOOoOoo##OoOOOOOoOoooOooOooOOOooooooooooooooOooooooooooooooo",
  "OOOOOOOooOOoooOOOOOOOOOOooooOooOoOoooOoOOooooooooooooOooooooooo"
};

// }}}
  
  static char *ppc_house_xpm[] = {
    // {{{ open

  "63 60 15 1",
  "  c navy",
  ". c blue",
  "X c #008000",
  "o c green",
  "O c #008080",
  "+ c cyan",
  "@ c #800000",
  "# c #804000",
  "$ c red",
  "% c #FF8000",
  "& c yellow",
  "* c #808080",
  "= c #8080FF",
  "- c #C0C0C0",
  "; c gray100",
  ".........................;....;;--..................++.++......",
  ".......................;;;;;.;;;--;.................+++;+.++.+.",
  ".......................;;;;;;---;;*..............+.+++++++.++++",
  "......................;;;;-;-;-;;***......+++..+++++.+.++++++++",
  "......................;;;--;;;******+.+++++++++.+++;++++.++++++",
  "....................;..;--;-*******++++++++++++++++++++++;;++++",
  "++...;.................-;-;;*****.+++++++++++++;+++++++++++++++",
  ".+.....................;;;******++++++++++++++++++++++X++++++++",
  "+++.++......++++++.+++.--******+;.+++++++++++++++++++X+++++++++",
  "...+++....+.++++.+++++--******+.++++++++++++++++++ooXXX++++++++",
  ".+++.......+++++++++++--*****.++;+++++++++++++++++ooXXXXX++++++",
  "++++.+...+.++++++++;++---**++++.++++++++++++++++&oX&XXXXX++++++",
  "++++++.++++++++++++++++---+....++++;;+++;+++++++oXXoooXXXX+++++",
  "+;++...++++++++++++$$$@@+++++++;;;;+;@@$#+++++++ooX&XXXXXX+++++",
  "++++++++++++++++;++$$$@@@;++;+;;;@@@@$$$$#;+++o&&XX&XXXXX++++++",
  "++++++++++++$++++$+$$$@@@$$$$;@@@$$$$$$$$#++;&&ooXXXXXoXXX+++++",
  "++++++++++++++;++;;$$$@@@$@@@@$$$$$$$$$$$$#;&o&&&&Xo&XXXXX+++++",
  "++.++++++++++++++;;$$$@@@@$$$$$$@@@$$$$$$$$#o&oo&&XXXXXXXX+++++",
  "++++++++++++;++;;$$@@@@$$$$$$$@@$$$@@$$$$$$$#&&&&XoXXXXXXX++;++",
  "+++++++++++;;;;;#@@$$$$$$$$@@@$$$$$$$@@@@$$$#$ooooXXXXXXXXX++++",
  "+++..++++;;;;;;#@@@$$$$$$@@$$$$$$$$$$$@@@@$$$#oooXXXXXXXX+X++++",
  "+;+**;;;;;;;;;#@@%%@$$$$$$@@@@$$$$@@@@$$@@@$$$#oooXoXXXXXXX;+;;",
  ";;****;;;;;;;#@@%&%@$$$$$$@$$$@@@@$$$&&$@$@$$$@#oXoXXXXXXX;+;;;",
  ";$****;;;;;;#@@%%&%%@$$$$$$@$$$@$&&$&&&$@@@@@$@@oXXXXXXXXX;;++;",
  "$*****;;;;;#@@&%%&%%&@$$$$$$@$$@$&&$&&&$@$@@$$@@@XXXXXXXXX;;;+;",
  "******;;;;;@@%&%%&%%&%@$$$$$$@$@$&&$&&&@@@@@@$@@@@XXXXXXXX;;;;;",
  "******$;;%#@@%&%%&%%&%@$$$$$$@$@$&&$&@@$$@$$$X@@oXXXXXXXXX;;;;;",
  "*******$&#@@&%%&%&%&%%&@$$$$$$@@$@@@$@@@@@$#@@.XoooXXXXXXX;;;;;",
  "*******%#@@%&%%&%&%&%%&%@$$$$$$@@@@@@@@@$@@@.#oooXXXXXXXXX;;;;;",
  "*******#@@%%&%%&%&%&%%&%%@$$$$$$$$$$$$#@@..==#ooXXXXXXXXXX;;;;;",
  "*******@@&%%&%%&%&%&%%&%%%@$$$$$$$$$@@@..====#ooXXXXXXXX%###%%%",
  "******@@#########&%%%%%%%%@$$$$$$$@@...======#ooXXXXXXXX;######",
  "%%%**@%#+++++++++#########+@$$$#@@...========#oXXXXXXXXXX;##X##",
  "%%%%%%;#$OOOOOOOO++++++++#$$@#@@...==========#oXXXXXXXXXX%%%#%%",
  "%%%%%%%#$++++++++OOOOOOOO#$# . ..=====###====#;#XXXXXXXXXXX%%%%",
  "%%%%%%%#$++++++++++++++++#=. . =====##&&#====##;##XXXXXXXXXXXXX",
  "%%%%%%%#$++++++++++OOOOOO# . ========&&&#====######XXXXXXXXXXXX",
  "%XX%%%%o+++++++++++++++++# = ==&##===&&&#====#;XX#XXXXXXXXXXXXX",
  "XXXXXXoo++++++++++++++OOO# ==###&#&==&&&#====#XXXXXXXXXXXoXooXX",
  "XXXXXXXo++####+++++++++++#===&&&&#&===&##====#XXXXXXXXXooXXXooX",
  "%XXXXXXo++#&&#+++++++++OO#===&&&&#===##======#XXXXoXXXXXXoXXXoo",
  "XXXXXXX#++#&&###++++++++#O===&&&&#==========##XXXXXXXXXoooXXooX",
  "XXXXXXo#++#&&#+#####++++#$====&###========##XXXooXooXXXXXoXXoXo",
  "XXXXXXo#++#&&#+#%%%#++++#$===##=========##XXXXXXoXXooXXXXooXooo",
  "XXXXXXo#++##&#+#%%%#++++#$============##XXXXXXXXXoXXooooooXoXoo",
  "XXXXXXo#+++###+#%%%#++++#$=========X##XoXXooXoXoooXooooooooXXoo",
  "XXXXXXo#+++++++#%###++++#========X##XXXXXXXoXooXooXXooooXoXoXoo",
  "XXXXXXo#+++++++#%###++++#======X##oXXXXooooooXXXooooooooooooooo",
  "XXXXXXX#X++++++#%%%#++++#===$X##oXXooXooXXXooXoooooXooooooooooo",
  "XXXoooX#########$%%#######XX##XoXXoXoXooXooXooooooooooooooooooo",
  "XXooXXXXXXXXXXX#oXoXXXXooXX#XXXoXXXXXXooooXooooooooooooooooooo&",
  "oXXoXXXXXooXXoXoXoXXoXXXXXXXoooXXoooXoXXXooooooooooooooooooo&&&",
  "XXoXXooXXXXXXoXXoXoXoXXXXoXoooXooXXooXXooXooooo&X&ooooX&oo&&&o&",
  "oXXoooXXoXXXXooXXXoXXoooooXXXXXooooXXXooooooo&&&&Xo&&&&&&&&&&&&",
  "ooXoXXXXXXoXXXXXoooooXXoXXXXooXoXXoooooooooo&X&&&&o&X&&&&&&&&o&",
  "ooXoXXXooXXoXoXXooooXXXooXXooXoooooooooooo&oo&&&&&&&&&&&&&&&&&&",
  "oooooooooXooXoXXoooooooXXXooXoooooooooo&&o&&&o&&&&&&&&&&&&o&&o&",
  "ooooooooooXoXooXoooooooXooooooooooooooo&&&&&&&&&&&&&&&&&&&&&o&&",
  "ooooooooooooXooXooooooooooooooooooooooo&ooo&&&&&&&&&&&&&&&&&&&&",
  "oooooooooooooooooooooooooooooooooooooo&ooo&oo&&&&&&&&&&&&&&&&&&"
};

// }}}
  
  strvec tokenize(string s,string sDelims){
    // {{{ open
    
    unsigned int iPos;
    unsigned int iLastPos = 0;
    vector<string> oTokens;
    iPos = s.find_first_of( sDelims, iLastPos );
    // we don't want empty tokens
    if (iPos != iLastPos)
      oTokens.push_back(s.substr(iLastPos,iPos-iLastPos));
    
    while(iPos != string::npos){
      iLastPos = iPos;
      iPos = s.find_first_of( sDelims, iLastPos+1 );
      oTokens.push_back(s.substr(iLastPos+1,iPos-iLastPos-1));
    }
    return oTokens;
  }
  // }}}
  
  Img8u *read_xpm(char **p){
    // {{{ open

  // header
  strvec headA = tokenize(*p++," ");
  int w = atoi(headA[0].c_str());
  int h = atoi(headA[1].c_str());
  int colors = atoi(headA[2].c_str());
  int one = atoi(headA[3].c_str()); (void)one;
  
  // parsing colors:
  XPMColor lut[256];
  for(int i=0;i<colors;i++){
    string line(*p++);
    if(line.length() < 5){
      printf ("error paring color line \"%s\" \n",line.c_str());
      continue;
    }    
    lut[(int)line[0]] = getColor(&(line[4]));
  }

  // creating image
  Img8u *image = new Img8u(Size(w,h),formatRGB);
  Img8u::iterator it[3] = {  image->getIterator(0),
                             image->getIterator(1),
                             image->getIterator(2)  };

  // paring image content
  for(int y=0;y<h;y++){
    for(char *line = *p++; *line; line++, ++it[0], ++it[1], ++it[2]){
      XPMColor c = lut[(int)(*line)];
      *(it[0]) = c.r;
      *(it[1]) = c.g;
      *(it[2]) = c.b;
    }
  }
  return image;
}

// }}}
}

namespace icl{
  
  Img8u *TestImages::internalCreate(const string &name){
    if(name == "women"){
      return read_xpm(ppc_woman_xpm);
    }else if(name == "tree"){
      return read_xpm(ppc_tree_xpm);
    }else if(name == "house"){
      return read_xpm(ppc_house_xpm);
    }else if(name == "parrot"){
      return createImage_macaw()->asImg<icl8u>();
    }else if(name == "flowers"){
      return createImage_flowers()->asImg<icl8u>();
    }else if(name == "windows"){
      return createImage_windows()->asImg<icl8u>();
    }else{
      ERROR_LOG("TestImage "<< name << "not found!");
      return 0;
    }
  }

  ImgBase* TestImages::create(const string& name, format f, depth d){
    // {{{ open

    Img8u *src = internalCreate(name);
    if(!src) return 0;
    
    if(src->getDepth() != d || src->getFormat() != f){
      ImgBase *dst = imgNew(d,src->getSize(),f);
      Converter(src,dst);
      delete src;
      return dst;
    }else{
      return src;
    }
  }

  // }}}

  
  ImgBase* TestImages::create(const string& name, const Size& size,format f, depth d){
    // {{{ open

    Img8u *src = internalCreate(name);
    if(!src) return 0;

    if(src->getDepth() != d || src->getFormat() != f || src->getSize() != size ){
      ImgBase *dst = imgNew(d,size,f);
      Converter(src,dst);
      delete src;
      return dst;
    }else{
      return src;
    }
  }

  // }}}

  void TestImages::xv(const ImgBase *image, const string& nameIn, long msec){
    // {{{ open
    string name = nameIn;
    if(image->getChannels() != 3){
      name+=".pgm";
    }
    FileWriter(name).write(image);
    system(string("xv ").append(name).append(" &").c_str());
    usleep(msec*1000);
    system(string("rm -rf ").append(name).c_str());
  }

  // }}}
 
}
