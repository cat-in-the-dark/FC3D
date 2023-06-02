all: build run

reload:
	cmake -Bcmake-build-debug

build: reload
	cmake --build cmake-build-debug -j 4

clean:
	rm -rf cmake-build-debug

run:
	./cmake-build-debug/FC3D

reload_web:
	emcmake cmake -Bcmake-build-web -DPLATFORM=Web

build_web: reload_web
	cmake --build cmake-build-web -j 4

run_web:
	python -m http.server -d cmake-build-web/ 3000

clean_web:
	rm -rf cmake-build-web