#!/bin/bash
source='icl-project'

echo "quickbuilding  "$source
pdflatex $source.tex
echo "Success opening Document "$source
evince $source.pdf &

