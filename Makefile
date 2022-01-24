release: server client
debug: server-dbg client-dbg

bdir:
	mkdir -p build/

clean:
	rm -rf build/

server: bdir
	cd server && $(MAKE)
	mv server/build/* build/server/

server-dbg: bdir
	cd server && $(MAKE) debug
	mv server/build/* build/server/

client: bdir
	cd client && $(MAKE)
	mv client/build/* build/client/

client-dbg: bdir
	cd client && $(MAKE) debug
	mv client/build/* build/client/
