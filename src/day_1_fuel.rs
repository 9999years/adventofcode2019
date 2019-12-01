/// Code for [AoC 2019, day 1][aoc1].
/// [aoc1]: https://adventofcode.com/2019/day/1
use std::cmp;

use lazy_static::lazy_static;

lazy_static! {
    static ref MODULE_MASSES: Vec<i32> = include_str!("../data/day_1_fuel_input.txt")
        .lines()
        .map(|i| i32::from_str_radix(i, 10).unwrap())
        .collect();
}

pub fn fuel_required(mass: i32) -> i32 {
    mass / 3 - 2
}

pub fn total_fuel_required() -> i32 {
    MODULE_MASSES.iter().map(|m| fuel_required(*m)).sum()
}

/// Fuel required for a given mass, accounting for the newly-added fuel. There's probably a
/// discrete formula for this...
pub fn adjusted_fuel_required(mass: i32) -> i32 {
    match cmp::max(0, fuel_required(mass)) {
        0 => 0,
        fuel => fuel + adjusted_fuel_required(fuel),
    }
}

pub fn adjusted_total_fuel_required() -> i32 {
    MODULE_MASSES
        .iter()
        .map(|m| adjusted_fuel_required(*m))
        .sum()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fuel_required_test() {
        assert_eq!(fuel_required(12), 2);
        assert_eq!(fuel_required(14), 2);
        assert_eq!(fuel_required(1969), 654);
        assert_eq!(fuel_required(100_756), 33_583);
    }

    #[test]
    fn total_fuel_required_test() {
        assert_eq!(total_fuel_required(), 3_296_269);
    }

    #[test]
    fn adjusted_fuel_required_test() {
        assert_eq!(adjusted_fuel_required(14), 2);
        assert_eq!(adjusted_fuel_required(1969), 966);
        assert_eq!(adjusted_fuel_required(100_756), 50_346);
    }

    #[test]
    fn adjusted_total_fuel_required_test() {
        assert_eq!(adjusted_total_fuel_required(), 4_941_547);
    }
}
