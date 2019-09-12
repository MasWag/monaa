monaa.1: man_header man.md
	cat $^ | sed '/# Usage Manual/ d; s/^#//;' | awk '/#/{$$0=toupper($$0)}1;'|  pandoc -o $@ --standalone
