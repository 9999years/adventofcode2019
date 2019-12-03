# The Tyranny of the Rocket Equation
day-1:
	rustc --crate-type lib --test day_1_fuel.rs
	./day_1_fuel

day-2:
	racket ./day_2_intcode.rkt

# Crossed Wires
day-3:
	go test -v ./day_3_wires_test.go

# Remove generated files and binaries
clean:
	rm day_1_fuel
