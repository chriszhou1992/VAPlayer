make: main.c
	gcc -o mp1 main.c gui.c gui.hp.c vr.c vp.c ar.c ap.c `pkg-config --cflags --libs gstreamer-interfaces-0.10 gtk+-2.0 gstreamer-0.10`
clean:
	rm -rf mp1
	rm -rf *~
