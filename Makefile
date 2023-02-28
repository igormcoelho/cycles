all: test demo

demo:
	cd examples/demo_cptr && make

test:
	cd tests && make

bench:
	cd tests && make bench

clean:
	rm -f vgcore.*
	rm -f tests/vgcore.*
	rm -f examples/demo_cptr/vgcore.*
	rm -f examples/examples/vgcore.*