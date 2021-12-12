#!/bin/bash
source='icl-project'

echo "Cleaning up"

toclear="png aux bbl blg dvi log lot toc lof out -blx.bib nav run.xml snm acn glo ist pdf pgf syg tdo"

for i in $toclear;
do
  rm -f $source*$i
done

rm -f sections/*.aux
rm -f plots/*.aux
rm -f sections/*~
rm -f plots/*~
rm -f mylib.bib
