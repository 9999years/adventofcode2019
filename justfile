# The Tyranny of the Rocket Equation (Rust)
day-1:
	rustc --crate-type lib --test day_1_fuel.rs
	./day_1_fuel

# Program Alarm (Racket)
day-2:
	racket ./day_2_intcode.rkt

# Crossed Wires (Go)
day-3:
	go test -v ./day_3_wires_test.go

# Secure Container (Befunge)
day-4:
	@echo "vvvvvv Last password checked."
	@echo "       vvvv Total number of valid passwords."
	bef -q ./day_4_secure_container.bf
	@echo
	@echo "vvvvvv Last password checked."
	@echo "       vvvv Total number of valid passwords."
	bef -q ./day_4_pt_2_secure_container.bf
	@echo

# Sunny with a Chance of Asteroids (Nim)
day-5:
	nim compile --run ./day_5_intcode_2.nim

# Universal Orbit Map (LaTeX3)
day-6:
	latex ./day_6_orbits.tex

# Amplification Circuit (C)
day-7:
	gcc ./day_7_intcode_3.c \
		-o day_7_intcode_3 \
		-std=c99 \
		-Wall -Wpedantic
	./day_7_intcode_3

# Space Image Format (Python 0.9)
day-8:
	@echo "$(printf "Make sure to get a copy of Python 0.9.1 from \033[4m\033[36mhttps://www.python.org/download/releases/early/\033[0m")"
	@echo "You'll also likely want to apply the patch in ./day_8_python.patch to avoid spurious integer overflow detection errors."
	python0.9 day_8_image.py

# Sensor Boost (C++17)
day-9:
	mkdir -p etc/day_9_build
	cp etc/CMakeLists.txt etc/CMakeLists.txt.in etc/day_9_build/
	cd etc/day_9_build && [ -e ./Makefile ] \
		|| cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .
	cd etc/day_9_build \
		&& cp ./compile_commands.json ../../ \
		&& make
	./etc/day_9_build/day_9_intcode_4

# Monitoring Station (Node.js)
day-10:
	mkdir -p ./etc/day_10_build
	cp ./day_10_asteroids.js ./etc/package.json ./etc/day_10_build/
	mkdir -p ./etc/day_10_build/data \
		&& cp data/day_10_asteroid_map.txt etc/day_10_build/data
	cd ./etc/day_10_build/ && [ -d ./node_modules ] \
		|| npm install
	cd ./etc/day_10_build && npm test

# Remove generated files and binaries
clean:
	rm -f day_1_fuel
	rm -f day_5_intcode_2
	rm -f day_6_orbits.aux
	rm -f day_6_orbits.log
	rm -f regression-test.log
	rm -f day_7_intcode_3
	rm -rf etc/day_9_build
