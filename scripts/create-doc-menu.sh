cd doc
cp menu.html-template menu.html
A="<!-- ICL_PROJECT_ENTRIES -->"
B="<tr><td> <a href=\"../"
C="\/index.html\" target=\"MainFrame\">"
D="</a></td>\n"
for T in $@ ; do 
    echo "creating entry menu entry for package $T"
    sed -i "s|$A|$B$T$C$T$D$A|g" menu.html
done 

# Old::        $menuTable .= "<tr><td> <a href=\"../".$project."/index.html\" target=\"MainFrame\">".$project."</a></td>\n";