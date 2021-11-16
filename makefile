build:
	gcc -o pcu main.c input.c
clean:
	rm -f pcu
go:
	make build && ./pcu
