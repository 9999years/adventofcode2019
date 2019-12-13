const fs = require('fs')
const {expect} = require('chai')
const {gcd} = require('mathjs')

ASTEROID = '#'

function angle(from, to) {
    // Angle between a vector pointing straight up starting at 'from' and the
    // vector starting at 'from' pointing towards 'to'. Not necessarily the
    // smallest angle, always goes clockwise.
    //
    // Remember, 90 degrees is Math.PI / 2 radians.
    const y_diff = Math.abs(to.y - from.y)
    const x_diff = Math.abs(to.x - from.x)
    const quarter = Math.PI / 2

    // Check for right angles; division by 0 and such doesn't make Math.atan
    // happy.
    if (x_diff === 0) {
        return (to.y <= from.y
            ? 0 // 'to' is directly above 'from'
            : quarter * 2 // 'to' is directly below 'from'
        )
    } else if (y_diff === 0) {
        return (to.x >= from.x
            ? quarter // right
            : quarter * 3 // left
        )
    }

    if (to.y < from.y && to.x >= from.x) {
        // 'to' is up and to the right of 'from'
        return Math.atan(x_diff / y_diff)
    } else if (to.y >= from.y && to.x > from.x) {
        // 'to' is down and to the right of 'from'
        return Math.atan(y_diff / x_diff) + quarter
    } else if (to.y > from.y && to.x <= from.x) {
        // 'to' is down and to the left of 'from'
        return Math.atan(x_diff / y_diff) + 2 * quarter
    } else if (to.y <= from.y && to.x <= from.x) {
        // 'to' is up and to the left of 'from'
        return Math.atan(y_diff / x_diff) + 3 * quarter
    }
}

function* _along(from, to) {
    var dx = to.x - from.x
    var dy = to.y - from.y
    if (dx === 0 && dy === 0) {
        return
    }
    const divisor = gcd(dx, dy)
    if (divisor > 1) {
        dx /= divisor
        dy /= divisor
    }
    const pos = {
        x: to.x + dx,
        y: to.y + dy,
    }
    while (this.is_in_bounds(pos)) {
        yield pos
        pos.x += dx
        pos.y += dy
    }
}

function sort_clockwise(from, asteroids) {
    return asteroids.sort((a, b) => angle(from, a) - angle(from, b))
}

function _vaporize(from, pos) {
    this[pos.y][pos.x].asteroid = false
    this[pos.y][pos.x].obscured_by = []
    for (const test of this.along(from, pos)) {
        const test_ast = this[test.y][test.x]
        const inx = test_ast.obscured_by.indexOf(from)
        if (inx !== -1) {
            delete test_ast.obscured_by[inx]
            break
        }
    }
}

function _vaporize_all(from) {
    var ret = []
    var visible = this.find_visible(from)
    while (visible.length !== 0) {
        sort_clockwise(from, visible)
        for (const ast of visible) {
            ret.push(ast)
            this.vaporize(from, ast)
        }
        var visible = this.find_visible(from)
    }
    return ret
}

// 'from' and 'to' are {x: _, y: _} objects.
function _mark_obscured(from, to) {
    for (const pos of this.along(from, to)) {
        if (this[pos.y][pos.x].obscured_by.indexOf(from) === -1) {
            this[pos.y][pos.x].obscured_by.push(from)
        }
    }
}

function _maybe_obscure(from, to) {
    if (this[to.y][to.x].asteroid
        && this[to.y][to.x].obscured_by.indexOf(from) === -1) {
        this.mark_obscured(from, to)
        return to
    }
    return undefined
}

function _find_visible(from) {
    for (const {ast} of this.entries()) {
        ast.obscured_by = []
    }
    const visible = []
    for (const coord of this.spiral(from)) {
        visible.push(this.maybe_obscure(from, coord))
    }
    return visible.filter(i => i !== undefined)
}

function _best_visibility() {
    var most_vis = -Infinity
    var best = undefined
    for (const candidate of this.to_array()) {
        const vis = this.find_visible(candidate).length
        if (vis > most_vis) {
            most_vis = vis
            best = candidate
        }
    }
    best.asteroids_visible = most_vis
    return best
}

function _is_in_bounds(coord) {
    return (
        coord.x >= 0
        && coord.y >= 0
        && coord.x < this.width
        && coord.y < this.height
    )
}

function* range_inc(start, stop) {
    step = 1
    if (stop < start) {
        step = -1
    }
    stop += step
    for (var i = start; i !== stop; i += step) {
        yield i
    }
}

function copy_coord(coord) {
    return {x: coord.x, y: coord.y}
}

function* _spiral(center) {
    // Suppose we start at the '#'. Then, we iterate in the following order, in
    // a clockwise spiral starting at the top-left corner of the #:
    // 555569
    // 811269
    // 84#269
    // 843369
    // 777769
    const top_left = {
        x: center.x - 1,
        y: center.y - 1,
    }
    const bottom_right = {
        x: center.x + 1,
        y: center.y + 1,
    }
    // While *at least one* corner of this iteration's rectangle is a valid coordinate:
    while (this.is_in_bounds({x: top_left.x, y: 0})
        || this.is_in_bounds({x: 0, y: top_left.y})
        || this.is_in_bounds({x: bottom_right.x, y: 0})
        || this.is_in_bounds({x: 0, y: bottom_right.y})) {
        const pos = {
            x: top_left.x,
            y: top_left.y,
        }
        // We start above `center` at the top and move right; the left half of
        // this segment is covered at the end.
        for (pos.x of range_inc(center.x, bottom_right.x)) {
            if (this.is_in_bounds(pos)) {
                yield copy_coord(pos)
            }
        }
        for (pos.y of range_inc(top_left.y + 1, bottom_right.y)) {
            if (this.is_in_bounds(pos)) {
                yield copy_coord(pos)
            }
        }
        for (pos.x of range_inc(bottom_right.x - 1, top_left.x)) {
            if (this.is_in_bounds(pos)) {
                yield copy_coord(pos)
            }
        }
        for (pos.y of range_inc(bottom_right.y - 1, top_left.y + 1)) {
            if (this.is_in_bounds(pos)) {
                yield copy_coord(pos)
            }
        }
        // Finally, covering the left half of the top segment.
        pos.y = top_left.y
        for (pos.x of range_inc(top_left.x, center.x - 1)) {
            if (this.is_in_bounds(pos)) {
                yield copy_coord(pos)
            }
        }

        top_left.x -= 1
        top_left.y -= 1
        bottom_right.x += 1
        bottom_right.y += 1
    }
}

function* _entries() {
    for (var y = 0; y < this.height; y++) {
        for (var x = 0; x < this.width; x++) {
            yield {
                x: x,
                y: y,
                ast: (this[y] || [])[x]
            }
        }
    }
}

function _to_array() {
    const ret = []
    for (const {x, y, ast} of this.entries()) {
        if (ast.asteroid) {
            ret.push({x: x, y: y})
        }
    }
    return ret
}

function _to_map(highlights) {
    const top_bar = '+' + '-'.repeat(this.width) + '+'
    const ret = [
        top_bar
    ]
    var row = []
    for (const {x, y, ast} of this.entries()) {
        if (x === 0) {
            row = []
        }
        var highlighted = false
        for (var i = 0; i < highlights.length; i++) {
            if (highlights[i] && highlights[i].x == x && highlights[i].y == y) {
                row.push('O')
                highlighted = true
                highlights.splice(i, i)
            }
        }
        if (!highlighted) {
            row.push(
                ast.asteroid
                    ? ASTEROID
                    : ' '
            )
        }
        if (x === this.width - 1) {
            ret.push('|' + row.join('') + '|')
        }
    }
    ret.push(top_bar)
    return ret.join('\n')
}

// Returns an asteroid map.
function parse_asteroid_map(str) {
    const lines = str.split('\n')
    const ret = []
    ret.width = lines[0].length
    ret.height = lines.length
    ret.entries = _entries
    var row = []
    for (const {x, y} of ret.entries()) {
        if (x === 0) {
            row = []
        }
        row.push({
            asteroid: lines[y][x] === ASTEROID,
            obscured_by: []
        })
        if (x === ret.width - 1) {
            ret.push(row)
        }
    }
    ret.mark_obscured = _mark_obscured
    ret.maybe_obscure = _maybe_obscure
    ret.find_visible = _find_visible
    ret.is_in_bounds = _is_in_bounds
    ret.to_array = _to_array
    ret.best_visibility = _best_visibility
    ret.vaporize = _vaporize
    ret.vaporize_all = _vaporize_all
    ret.spiral = _spiral
    ret.along = _along
    ret.to_map = _to_map
    return ret
}


function file_to_map(fname) {
    return parse_asteroid_map(
        fs.readFileSync(fname, 'utf-8').trim()
    )
}

function day_10_asteroid_map() {
    if (day_10_asteroid_map._map === undefined) {
        day_10_asteroid_map._map = file_to_map('data/day_10_asteroid_map.txt')
    }
    return day_10_asteroid_map._map
}

const ASTEROID_MAP_1 = `
.#..#
.....
#####
....#
...##`.trim()

const ASTEROID_MAP_2 = `
......#.#.
#..#.#....
..#######.
.#.#.###..
.#..#.....
..#....#.#
#..#....#.
.##.#..###
##...#..#.
.#....####`.trim()

const ASTEROID_MAP_3 = `
#.#...#.#.
.###....#.
.#....#...
##.#.#.#.#
....#.#.#.
.##..###.#
..#...##..
..##....##
......#...
.####.###.`.trim()

const ASTEROID_MAP_4 = `
.#..#..###
####.###.#
....###.#.
..###.##.#
##.##.#.#.
....###..#
..#.#..#.#
#..#.#.###
.##...##.#
.....#.#..`.trim()

const ASTEROID_MAP_5 = `
.#..##.###...#######
##.############..##.
.#.######.########.#
.###.#######.####.#.
#####.##.#.##.###.##
..#####..#.#########
####################
#.####....###.#.#.##
##.#################
#####.##.###..####..
..######..##.#######
####.##.####...##..#
.#####..#.######.###
##...#.##########...
#.##########.#######
.####.#.###.###.#.##
....##.##.###..#####
.#.#.###########.###
#.#.#.#####.####.###
###.##.####.##.#..##`.trim()

it('parse_asteroid_map', function () {
    const asteroids = parse_asteroid_map(ASTEROID_MAP_1)
    expect(asteroids.to_array()).to.eql([
        {x: 1, y: 0},
        {x: 4, y: 0},

        {x: 0, y: 2},
        {x: 1, y: 2},
        {x: 2, y: 2},
        {x: 3, y: 2},
        {x: 4, y: 2},

        {x: 4, y: 3},

        {x: 3, y: 4},
        {x: 4, y: 4},
    ])
})

it('find_visible', function () {
    const asteroids = parse_asteroid_map(ASTEROID_MAP_1)
    expect(asteroids.find_visible({x: 3, y: 4})).to.eql([
        {x: 4, y: 3},
        {x: 4, y: 4},
        {x: 3, y: 2},
        {x: 4, y: 2},
        {x: 1, y: 2},
        {x: 2, y: 2},
        {x: 0, y: 2},
        {x: 4, y: 0},
    ])
})

it('range up', function () {
    const r = range_inc(1, 3)
    expect(r.next().value).to.equal(1)
    expect(r.next().value).to.equal(2)
    expect(r.next().value).to.equal(3)
    expect(r.next()).to.eql({
        value: undefined,
        done: true,
    })
})

it('range_inc down', function () {
    const r = range_inc(3, 1)
    expect(r.next().value).to.equal(3)
    expect(r.next().value).to.equal(2)
    expect(r.next().value).to.equal(1)
    expect(r.next()).to.eql({
        value: undefined,
        done: true,
    })
})

it('spiral', function () {
    const asteroids = parse_asteroid_map(`
...
...
...
    `.trim())
    // 112
    // 4#2
    // 433
    // Note that 'coords' is backwards so coords.pop() works correctly.
    const coords = [
        {x: 0, y: 0}, // This is the last one.
        {x: 0, y: 1},
        {x: 0, y: 2},
        {x: 1, y: 2},
        {x: 2, y: 2},
        {x: 2, y: 1},
        {x: 2, y: 0},
        {x: 1, y: 0}, // This is the first coordinate returned by the iterator.
    ]
    for (var c of asteroids.spiral({x: 1, y: 1})) {
        expect(c).to.eql(coords.pop())
    }
})

it('best_visibility', function () {
    const asteroids = parse_asteroid_map(ASTEROID_MAP_1)
    expect(asteroids.best_visibility()).to.eql({
        x: 3,
        y: 4,
        asteroids_visible: 8,
    })
})

it('larger map 1', function () {
    expect(parse_asteroid_map(ASTEROID_MAP_2).best_visibility()).to.eql({
        x: 5,
        y: 8,
        asteroids_visible: 33,
    })
})

it('larger map 2', function () {
    expect(parse_asteroid_map(ASTEROID_MAP_3).best_visibility()).to.eql({
        x: 1,
        y: 2,
        asteroids_visible: 35,
    })
})

it('larger map 3', function () {
    expect(parse_asteroid_map(ASTEROID_MAP_4).best_visibility()).to.eql({
        x: 6,
        y: 3,
        asteroids_visible: 41,
    })
})

it('largest map', function () {
    expect(parse_asteroid_map(ASTEROID_MAP_5).best_visibility()).to.eql({
        x: 11,
        y: 13,
        asteroids_visible: 210,
    })
})

it('day 10, part 1', function () {
    expect(day_10_asteroid_map().best_visibility()).to.eql({
        x: 11,
        y: 11,
        asteroids_visible: 221,
    })
})

it('vaporization order', function () {
    const vaporized = parse_asteroid_map(ASTEROID_MAP_5).vaporize_all({x: 11, y: 13})
    // Add a blank value at the start so our test indexes look more sensible;
    // the 1st asteroid to be vaporized is at index 1, etc.
    vaporized.unshift({})
    expect(vaporized[1]).to.eql({x: 11, y: 12})
    expect(vaporized[2]).to.eql({x: 12, y: 1})
    expect(vaporized[3]).to.eql({x: 12, y: 2})
    expect(vaporized[10]).to.eql({x: 12, y: 8})
    expect(vaporized[20]).to.eql({x: 16, y: 0})
    expect(vaporized[50]).to.eql({x: 16, y: 9})
    expect(vaporized[100]).to.eql({x: 10, y: 16})
    expect(vaporized[199]).to.eql({x: 9, y: 6})
    expect(vaporized[200]).to.eql({x: 8, y: 2})
    expect(vaporized[201]).to.eql({x: 10, y: 9})
    expect(vaporized[299]).to.eql({x: 11, y: 1})
})

it('day 10, part 2', function () {
    const TWO_HUNDREDTH = 199
    const vaporized_200th = {
        x: 8,
        y: 6,
    }
    expect(
        day_10_asteroid_map().vaporize_all({x: 11, y: 11})[TWO_HUNDREDTH]
    ).to.eql(vaporized_200th)

    // This is the result for the second part.
    expect(vaporized_200th.x * 100 + vaporized_200th.y).to.equal(806)
})

