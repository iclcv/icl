#!/bin/bash
source='icl-project'
options='-shell-escape -interaction batchmode'

./clear.sh

pdflatex $options $source.tex
bibtex $source.aux
pdflatex $options $source.tex
pdflatex $options $source.tex
pdflatex $options $source.tex

