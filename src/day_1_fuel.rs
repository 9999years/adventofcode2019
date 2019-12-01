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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fuel_required_test() {
        assert_eq!(fuel_required(12), 2);
        assert_eq!(fuel_required(14), 2);
        assert_eq!(fuel_required(1969), 654);
        assert_eq!(fuel_required(100_756), 33583);
    }

    #[test]
    fn total_fuel_required_test() {
        assert_eq!(total_fuel_required(), 3_296_269);
    }
}
