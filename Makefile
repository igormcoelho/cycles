all: test demo

demo:
	cd src/demo_cptr && make

test:
	cd tests && make

bench:
	cd tests && make bench

clean:
	rm -f vgcore.*
	rm -f tests/vgcore.*
	rm -f src/demo_cptr/vgcore.*
	rm -f src/examples/vgcore.*