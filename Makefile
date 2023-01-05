all: test demo

demo:
	cd src/demo_cptr && make

test:
	cd tests && make

clean:
	rm -f vgcore.*
	rm -f tests/vgcore.*