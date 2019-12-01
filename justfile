# The Tyranny of the Rocket Equation
day-1:
	rustc --crate-type lib --test day_1_fuel.rs
	./day_1_fuel

# Remove generated files and binaries
clean:
	rm day_1_fuel
