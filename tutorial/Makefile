TEX_SOURCES=$(wildcard *.tex)
CHAPTER_SOURCES=$(wildcard chapters/*.tex)
EXAMPLES=$(wildcard examples/*.cpp)
PDF_LATEX_FLAGS=-halt-on-error
SCP?=scp
VIEWER?=evince

export SCP

.PHONY: all weiter show allshow build_examples clean distclean pdfclean examplesclean edit spellcheck publish publish_script publish_folien publish_aufgaben aufgaben aufgabenclean publish_musterloesungen


all: Howto-ICL.pdf

#aufgaben:
#	make -C./aufgaben

build_examples:
	make -C./examples

Howto-ICL.pdf: $(TEX_SOURCES) $(EXAMPLES) $(CHAPTER_SOURCES)
	pdflatex $(PDF_LATEX_FLAGS) Howto-ICL.tex
	pdflatex $(PDF_LATEX_FLAGS) Howto-ICL.tex

clean: pdfclean

pdfclean:
	rm -f *.aux *.toc *.log *.pdf *.dvi

examplesclean: clean
	make -C./examples clean


distclean: clean
	rm -f *~ svn-commit*.tmp

show: 
	$(VIEWER) Howto-ICL.pdf &

allshow: all show

#publish_script: vorlesung-c++.pdf
#	$(SCP) vorlesung-c++.pdf porta.techfak.uni-bielefeld.de:/vol/www/data/techfak/persons/fschmidt/Vorlesung-C++-2008/

#publish_folien:
#	$(SCP) folien/*.odp porta.techfak.uni-bielefeld.de:/vol/www/data/techfak/persons/fschmidt/Vorlesung-C++-2008/Folien/
#	$(SCP) folien/*.pdf porta.techfak.uni-bielefeld.de:/vol/www/data/techfak/persons/fschmidt/Vorlesung-C++-2008/Folien/

#publish_aufgaben:
#	$(SCP) aufgaben/*.pdf porta.techfak.uni-bielefeld.de:/vol/www/data/techfak/persons/fschmidt/Vorlesung-C++-2008/Uebungsaufgaben/

#publish_musterloesungen:
#	make -C aufgaben publish_musterloesungen

#publish: publish_script publish_folien publish_aufgaben


edit:
	$(EDITOR) chapters/the-image-class.tex

weiter: all show edit

spellcheck: Howto-ICL.tex
	aspell --mode=tex --lang=de -c Howto-ICL.tex


#teilnehmertabelle.pdf:
#	pdflatex teilnehmertabelle.tex

gude:
	@echo "GUDE "

laune:
	@echo " LAUNE"

alder: 
	@echo " ALDER !!!!!"
