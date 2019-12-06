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

# Remove generated files and binaries
clean:
	rm -f day_1_fuel
	rm -f day_5_intcode_2
	rm -f day_6_orbits.aux
