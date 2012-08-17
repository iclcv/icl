#!/bin/sed -f

s/GUI \(.+\)("vbox\[\(.*\)]")/GUI \1 = VBox()\2/g
s/GUI \(.+\)("hbox")/HBox \1/g
s/GUI \(.+\)("vbox")/VBox \1/g
s/GUI \(.+\)("vsplit")/VSplit \1/g
s/GUI \(.+\)("hsplit")/HSplit \1/g
s/GUI \(.+\)("tab(\".*\")")/Tab("\1") \1/g
s/GUI *("hbox")/HBox()/g
s/GUI *("hbox\[\(.*\)]")/HBox()\1/g
s/GUI *("vbox")/VBox()/g
s/GUI *("vbox\[\(.*\)]")/VBox()\1/g
s/GUI *("hsplit")/HSplit()/g
s/GUI *("hsplit\[\(.*\)]")/HSplit()\1/g
s/GUI *("vsplit")/VSplit()/g
s/GUI *("vsplit\[\(.*\)]")/VSplit()\1/g
s/GUI *("hscroll")/HScroll()/g
s/GUI *("hscroll\[\(.*\)]")/HScroll()\1/g
s/GUI *("vscroll")/VScroll()/g
s/GUI *("vscroll\[\(.*\)]")/VScroll()\1/g
s/GUI *("tab(\([^)]*\))")/Tab("\1")/g
s/"togglebutton(\([^)]*\),\([^)]*\))/Button\("\1","\2")/g
s/"button(\([^)]*\))/Button\("\1"\)/g
s/"label(\([^)]*\))/label\("\1"\)/g
s/"buttongroup(\([^)]*\))/ButtonGroup\("\1"\)/g
s/"checkbox(\([^)]*\),checked)/CheckBox\("\1"\,true)/g
s/"checkbox(\([^)]*\),unchecked)/CheckBox\("\1"\)/g
s/"checkbox(\([^)]*\))/CheckBox\("\1"\)/g
s/"slider(\(.*\),vertical)/Slider\(\1,true)/g
s/"slider(\([^)]*\))/Slider\(\1\)/g
s/"fslider(\(.*\),vertical)/FSlider\(\1,true)/g
s/"fslider(\([^)]*\))/FSlider\(\1\)/g
s/"int(\([^)]*\))/Int\(\1\)/g
s/"float(\([^)]*\))/Float\(\1\)/g
s/"string("\([^"]*\)",\([0-9]*\))/String\("\1",\2\)/g
s/"disp(\([^)]*\))/Disp\("\1"\)/g
s/"image(\([^)]*\))/Image\("\1"\)/g
s/"draw(\([^)]*\))/Draw\("\1"\)/g
s/"draw3D(\([^)]*\))/Draw3D\("\1"\)/g
s/"plot(\([^)]*\))/Plot\("\1"\)/g
s/"plot/Plot\()/g
s/"combo(\([^)]*\))/Combo\("\1"\)/g
s/"spinner(\([^)]*\))/Spinner\("\1"\)/g
s/"fps(\([^)]*\))/Fps\("\1"\)/g
s/"camcfg(\([^)]*\))/CamCfg\("\1"\)/g
s/"prop(\([^)]*\))/Prop\("\1"\)/g
s/"color(\([^)]*\))/ColorSelect\("\1"\)/g
s/"ps(\([^)]*\))/Ps\("\1"\)/g
s/"multidraw(\([^)]*\))/MultiDraw\("\1"\)/g
s/"dummy(\([^)]*\))/Dummy\(\)/g
s/"!show"/Show\(\)/g
s/\[@/@/g
s/\]"//g
s/@handle=\([^;]*\);/@handle("\1");/g
s/@handle=\([^@]*\)@/@handle("\1")@/g
s/@handle=\(.*\)$/@handle("\1")/g
s/@out=\([^;]*\);/@out("\1");/g
s/@out=\([^@]*\)@/@out("\1")@/g
s/@out=\(.*\)$/@out("\1")/g
s/@label=\([^;]*\);/@label("\1");/g
s/@label=\([^@]*\)@/@label("\1")@/g
s/@label=\(.*\)$/@label("\1")/g
s/@tooltip=\([^;]*\);/@tooltip("\1");/g
s/@tooltip=\([^@]*\)@/@tooltip("\1")@/g
s/@tooltip=\(.*\)$/@tooltip("\1")/g
s/@minsize=\([0-9]\)*x\([0-9]*\)/@minSize(\1,\2)/g
s/@minsize=\([0-9]\)*x\([0-9]*\)/@minSize(\1,\2)/g
s/@maxsize=\([0-9]\)*x\([0-9]*\)/@maxSize(\1,\2)/g
s/@size=\([0-9]\)*x\([0-9]*\)/@size(\1,\2)/g
s/@/./g
s/ )"/") /g
s/)"/")/g



    

