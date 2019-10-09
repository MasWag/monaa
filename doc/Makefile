monaa.1: man_header man.md
	cat $^ | sed '/# Usage Manual/ d; s/^#//;' | awk '/#/{$$0=toupper($$0)}1;'|  pandoc -o $@ --standalone

tre2ta.1: man_header tre2ta_man.md
	cat $^ | sed 's/% MONAA(1)/% TRE2TA(1)/;s/% May 2018/% October 2019/;' | sed '/# .* Usage Manual/ d; s/^#//;' | awk '/#/{$$0=toupper($$0)}1;'|  pandoc -o $@ --standalone
