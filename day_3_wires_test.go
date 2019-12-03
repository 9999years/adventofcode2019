// I HATE GO!!!!!!!!!!
// I hate Go with all my heart. The standard library is both incredibly lacking
// and bloated, the type system is a joke, the community is insufferably sure
// that their programming abilities are unimpeachable, and Rob Pike is an
// asshole.
//
// ...This is code for Advent of Code 2019, day 3: Crossed Wires
// https://adventofcode.com/2019/day/3

package main

import "unicode/utf8"
import "strconv"
import "strings"
import "math"
import "io/ioutil"
import "testing"
import "fmt"

func abs_int(i int) int {
	if i < 0 {
		return -i
	} else {
		return i
	}
}

// A direction. Up, down, left, or right.
type Direction int

const (
	U Direction = iota
	R
	D
	L
)

func direction_from_rune(r rune) Direction {
	switch r {
	case 'U':
		return U
	case 'R':
		return R
	case 'D':
		return D
	case 'L':
		return L
	default:
		panic(fmt.Sprintf("Direction created from invalid rune %q", r))
	}
}

// Is this direction either left or right?
func (this Direction) is_lr() bool {
	switch this {
	case L, R:
		return true
	default:
		return false
	}
}

type Coord struct {
	x int
	y int
}

func coord(x, y int) Coord {
	return Coord{
		x: x,
		y: y,
	}
}

// Gets a reference to this coordinate.
func (this Coord) ref() *Coord {
	return &this
}

func (this Coord) add(other Coord) Coord {
	return Coord{
		x: this.x + other.x,
		y: this.y + other.y,
	}
}

func (this Coord) mul(other Coord) Coord {
	return Coord{
		x: this.x * other.x,
		y: this.y * other.y,
	}
}

func (this Coord) mul_scalar(other int) Coord {
	return this.mul(coord(other, other))
}

func (this Coord) equals(other Coord) bool {
	return this.x == other.x && this.y == other.y
}

func (this Coord) is_zero() bool {
	return this.x == 0 && this.y == 0
}

// The [taxicab metric](https://en.wikipedia.org/wiki/Taxicab_metric)-magnitude
// of this coordinate.
func (this Coord) taxi_magnitude() int {
	return this.taxi_dist(coord(0, 0))
}

func (this Coord) taxi_dist(other Coord) int {
	return abs_int(other.x-this.x) + abs_int(other.y-this.y)
}

func (this Coord) x_is_between(start, end int) bool {
	return this.x >= start && this.x <= end
}

func (this Coord) y_is_between(start, end int) bool {
	return this.y >= start && this.y <= end
}

// Gives the unit coordinate for a given direction; useful with mul_scalar.
func (this Direction) as_coord() Coord {
	switch this {
	case U:
		return coord(0, 1)
	case R:
		return coord(1, 0)
	case D:
		return coord(0, -1)
	case L:
		return coord(-1, 0)
	default:
		panic("Unreachable")
	}
}

// Invariant: start.x <= end.x, start.y <= end.y.
// Invariant: this.start.x == this.end.x || this.start.y == this.end.y
//
// A line segment, either horizontal or vertical. The start coordinate is
// guaranteed to be smaller than the end coordinate; therefore, the `flipped`
// field tells if the internal representation has been switched from the
// original segment's direction; sometimes we need the original start or end
// coordinates.
type Segment struct {
	start   Coord
	end     Coord
	flipped bool
}

func (this Segment) length() int {
	// Remember, a Segment is always horizontal or vertical.
	return this.start.taxi_dist(this.end)
}

func (this Segment) equals(other Segment) bool {
	return this.start.equals(other.start) && this.end.equals(other.end)
}

// The distance from `start` to `c`, or `-1` if `c` doesn't lie on this segment.
func (this Segment) steps_to(c Coord) int {
	switch {
	case this.is_vertical() &&
		c.x == this.start.x &&
		c.y_is_between(this.start.y, this.end.y):
		return abs_int(c.y - this.orig_start().y)
	case this.is_horizontal() &&
		c.y == this.start.y &&
		c.x_is_between(this.start.x, this.end.x):
		return abs_int(c.x - this.orig_start().x)
	}
	return -1
}

func (this Segment) x_is_between(start, end int) bool {
	return this.start.x >= start && this.start.x <= end
}

func (this Segment) y_is_between(start, end int) bool {
	return this.start.y >= start && this.start.y <= end
}

func (this Segment) is_vertical() bool {
	return this.start.x == this.end.x
}

func (this Segment) is_horizontal() bool {
	return this.start.y == this.end.y
}

func segment(start, end Coord) Segment {
	return Segment{
		start:   start,
		end:     end,
		flipped: false,
	}
}

func segment_from_dir(start Coord, d Direction, length int) Segment {
	ret := Segment{
		start:   start,
		end:     start.add(d.as_coord().mul_scalar(length)),
		flipped: false,
	}

	if ret.start.x > ret.end.x || ret.start.y > ret.end.y {
		// Flipped; maintain invariants.
		tmp := ret.start
		ret.start = ret.end
		ret.end = tmp
		ret.flipped = true
	}

	return ret
}

/// Returns the segment represented by a string, starting at this segment's endpoint.
func (this Segment) parse_after(s string) Segment {
	r, size := utf8.DecodeRuneInString(s)
	// Let's assume that the rune is valid, so I'm not gonna check if r is RuneError.
	// lol that Go just *lets* you... not.
	length, _ := strconv.Atoi(s[size:])
	return segment_from_dir(this.orig_end(), direction_from_rune(r), length)
}

func (this Segment) orig_start() Coord {
	if this.flipped {
		return this.end
	} else {
		return this.start
	}
}

func (this Segment) orig_end() Coord {
	if this.flipped {
		return this.start
	} else {
		return this.end
	}
}

// Disgusting. Returns the intersection closest to the origin, or nil if one
// doesn't exist.
func (this Segment) intersection(other Segment) *Coord {
	switch {
	case this.is_horizontal() && other.is_vertical() &&
		other.x_is_between(this.start.x, this.end.x) &&
		this.y_is_between(other.start.y, other.end.y):
		return coord(other.start.x, this.start.y).ref()
	case this.is_vertical() && other.is_horizontal() &&
		this.x_is_between(other.start.x, other.end.x) && other.y_is_between(this.start.y, this.end.y):
		return coord(this.start.x, other.start.y).ref()
	}
	return nil
}

type Path []Segment

func (this Path) steps_to(c Coord) int {
	var steps = 0
	for _, seg := range this {
		seg_steps := seg.steps_to(c)
		if seg_steps == -1 {
			steps += seg.length()
		} else {
			return steps + seg_steps
		}
	}
	return -1
}

func parse_wire(s string) Path {
	segments := strings.Split(s, ",")
	var path Path
	prev := segment(coord(0, 0), coord(0, 0))
	for _, seg := range segments {
		prev = prev.parse_after(seg)
		path = append(path, prev)
	}
	return path
}

func parse_wires(s string) []Path {
	wires := strings.Split(strings.TrimSpace(s), "\n")
	var paths []Path
	for _, wire := range wires {
		paths = append(paths, parse_wire(wire))
	}
	return paths
}

func taxi_min(coords []*Coord) *Coord {
	dist := math.MaxInt32
	var closest *Coord
	for _, c := range coords {
		c_dist := c.taxi_magnitude()
		if c_dist <= dist {
			closest = c
			dist = c_dist
		}
	}
	return closest
}

func steps_min_intersection(path1, path2 Path) int {
	steps := math.MaxInt32
	for _, c := range path1.intersections(path2) {
		c_steps := path1.steps_to(*c) + path2.steps_to(*c)
		if c_steps <= steps {
			steps = c_steps
		}
	}
	return steps
}

func (this Path) intersections(other Path) []*Coord {
	var intersections []*Coord
	for _, seg1 := range this {
		for _, seg2 := range other {
			intersection := seg1.intersection(seg2)
			if intersection != nil && !intersection.is_zero() {
				intersections = append(intersections, intersection)
			}
		}
	}
	return intersections
}

func taxi_min_intersection(path1 Path, path2 Path) *Coord {
	return taxi_min(path1.intersections(path2))
}

func AssertIntEq(t *testing.T, a, b int) {
	if a != b {
		t.Fatalf("Integers %v and %v not equal", a, b)
	}
}

func AssertCoordEq(t *testing.T, c1, c2 Coord) {
	if c1.x != c2.x || c1.y != c2.y {
		t.Fatalf("Coordinates %v and %v not equal", c1, c2)
	}
}

func AssertPathEq(t *testing.T, p1, p2 Path) {
	if len(p1) != len(p2) {
		t.Fatalf(
			"Paths have different lengths (%v and %v).\nLeft:  %v\nRight: %v",
			len(p1), len(p2),
			p1, p2,
		)
	}

	for i, s := range p1 {
		if !s.equals(p2[i]) {
			t.Fatalf(
				"Paths differ at index %v; segments %v and %v are not equal.\nLeft:  %v\nRight: %v",
				i,
				s, p2[i], // the two segments
				p1, p2,
			)
		}
	}
}

func TestTaxicabMetric(t *testing.T) {
	AssertIntEq(t, coord(3, 3).taxi_magnitude(), 6)
	AssertIntEq(t, coord(0, 0).taxi_dist(coord(3, 3)), 6)
	AssertIntEq(t, coord(3, 0).taxi_dist(coord(3, 3)), 3)
}

func TestExample0(t *testing.T) {
	p1 := parse_wire("R8,U5,L5,D3")
	AssertPathEq(t, p1, Path{
		segment(coord(0, 0), coord(8, 0)),
		segment(coord(8, 0), coord(8, 5)),
		segment(coord(3, 5), coord(8, 5)),
		segment(coord(3, 2), coord(3, 5)),
	})
	p2 := parse_wire("U7,R6,D4,L4")
	intersections := p1.intersections(p2)
	AssertCoordEq(t, *intersections[0], coord(6, 5))
	AssertCoordEq(t, *intersections[1], coord(3, 3))
	AssertIntEq(t, steps_min_intersection(p1, p2), 30)
}

func TestExample1(t *testing.T) {
	p1 := parse_wire("R75,D30,R83,U83,L12,D49,R71,U7,L72")
	p2 := parse_wire("U62,R66,U55,R34,D71,R55,D58,R83")
	AssertCoordEq(t, *taxi_min_intersection(p1, p2), coord(155, 4))
	AssertIntEq(t, coord(155, 4).taxi_magnitude(), 159)
	AssertIntEq(t, steps_min_intersection(p1, p2), 610)
}

func TestExample2(t *testing.T) {
	p1 := parse_wire("R98,U47,R26,D63,R33,U87,L62,D20,R33,U53,R51")
	p2 := parse_wire("U98,R91,D20,R16,D67,R40,U7,R15,U6,R7")
	AssertCoordEq(t, *taxi_min_intersection(p1, p2), coord(124, 11))
	AssertIntEq(t, coord(124, 11).taxi_magnitude(), 135)
	AssertIntEq(t, steps_min_intersection(p1, p2), 410)
}

func get_puzzle_paths() (Path, Path) {
	buf, _ := ioutil.ReadFile("data/day_3_wires.txt")
	wires := parse_wires(string(buf))
	return wires[0], wires[1]
}

func TestPuzzlePart1(t *testing.T) {
	p1, p2 := get_puzzle_paths()
	AssertIntEq(t, taxi_min_intersection(p1, p2).taxi_magnitude(), 266)
}

func TestPuzzlePart2(t *testing.T) {
	p1, p2 := get_puzzle_paths()
	AssertIntEq(t, steps_min_intersection(p1, p2), 19242)
}
