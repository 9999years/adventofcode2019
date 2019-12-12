const fs = require('fs')
const {expect} = require('chai')
const {gcd} = require('mathjs')

ASTEROID = '#'

function to_asteroid_array(asteroid_map) {
    const ret = []
    for (var y = 0; y < asteroid_map.length; y++) {
        for (var x = 0; x < asteroid_map[y].length; x++) {
            if (asteroid_map[y][x].asteroid) {
                ret.push({x: x, y: y})
            }
        }
    }
    return ret
}

// 'from' and 'to' are {x: _, y: _} objects.
function _mark_obscured(from, to) {
    var dx = to.x - from.x
    var dy = to.y - from.y
    if (dx === 0 && dy === 0) {
        return
    }
    const divisor = gcd(dx, dy)
    dx /= divisor
    dy /= divisor
    var x = to.x + dx
    var y = to.y + dy
    console.log({
        from: from,
        to: to,
        dx: dx,
        dy: dy,
    })
    while (0 < x && x < this.width && 0 < y && y < this.height) {
        console.log({x: x, y: y})
        this[y][x].obscured_by.push(from)
        x += dx
        y += dy
    }
}

function _find_visible(from) {
    // TODO: This should move in a spiral outwards from 'from'
    const visible = []
    for (var x = 0; x < this.width; x++) {
        for (var y = 0; y < this.height; y++) {
            if (this[y][x].asteroid && this[y][x].obscured_by.indexOf(from) === -1) {
                const pos = {x: x, y: y}
                visible.push(pos)
                this.mark_obscured(from, pos)
            }
        }
    }
    return visible
}

// Returns an asteroid map.
function parse_asteroid_map(str) {
    const lines = str.split('\n')
    const ret = []
    for (var y = 0; y < lines.length; y++) {
        const row = []
        for (var x = 0; x < lines[y].length; x++) {
            row.push({
                asteroid: lines[y][x] === ASTEROID,
                obscured_by: []
            })
        }
        ret.push(row)
    }
    ret.mark_obscured = _mark_obscured
    ret.find_visible = _find_visible
    ret.width = ret[0].length
    ret.height = ret.length
    return ret
}

const ASTEROID_MAP_1 = `
.#..#
.....
#####
....#
...##`.trim()

it('parse_asteroid_map', function () {
    const asteroids = parse_asteroid_map(ASTEROID_MAP_1)
    expect(to_asteroid_array(asteroids)).to.eql([
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


// console.log(fs.readFileSync('data/day_10_asteroid_map.txt', 'utf-8'))
