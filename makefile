default:
	cc main.c -o app -lncurses

clean:
	rm app

install:
	make default
	mv app ~/../usr/bin/tfm
