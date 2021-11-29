build:
	gcc -Wall -o pcu main.c input.c
clean:
	rm -f pcu
go:
	make build && ./pcu
debug:
	gcc -Wall -o pcu -g main.c input.c
