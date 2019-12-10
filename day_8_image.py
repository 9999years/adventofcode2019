#!/usr/bin/env python0.9
# Yes, Python 0.9.1, the first 'beta' release of Python, from 1991!
# Get a copy here:
# https://www.python.org/download/releases/early/

import string
import sys

NO_SHADE = ' '  # 0, 1
LIGHT_SHADE = '░'  # 2, 3
MEDIUM_SHADE = '▒'  # 4, 5
DARK_SHADE = '▓'  # 6, 7
FULL_SHADE = '█'  # 8, 9


def raw_print_pixel(pixel):
    c = '0'
    if pixel = 1:
        c = '1'
    elif pixel = 2:
        c = '2'
    elif pixel = 3:
        c = '3'
    elif pixel = 4:
        c = '4'
    elif pixel = 5:
        c = '5'
    elif pixel = 6:
        c = '6'
    elif pixel = 7:
        c = '7'
    elif pixel = 8:
        c = '8'
    elif pixel = 9:
        c = '9'
    sys.stdout.write(c)

def print_pixel(pixel):
    c = NO_SHADE
    if pixel >= 8:
        c = FULL_SHADE
    elif pixel >= 6:
        c = DARK_SHADE
    elif pixel >= 4:
        c = MEDIUM_SHADE
    elif pixel >= 2:
        c = LIGHT_SHADE
    sys.stdout.write(c)


def num_layers(image, width, height):
    return len(image) / (width * height)


def split_to_layers(image, width, height):
    pixels_per_layer = width * height
    layers = []
    for i in range(num_layers(image, width, height)):
        i = i * pixels_per_layer
        layers.append(image[i : i + pixels_per_layer])
    return layers


def map_to_digits(image):
    ret = []
    for char in image:
        ret.append(string.atoi(char))
    return ret


def count_element(lst, d):
    ctr = 0
    for n in lst:
        if n = d:
            ctr = ctr + 1
    return ctr



class Image():
    def new(self):
        self, (image, (width, height)) = self
        self.width = width
        self.height = height
        self.image = split_to_layers(map_to_digits(image), width, height)
        return self
    def layer(self, n):
        return self.image[n]
    def _print_layer(self, layer):
        column = 0
        for p in layer:
            column = column + 1
            print_pixel(p)
            if column = self.width:
                print
                column = 0
    def print_layer_raw(self, layer):
        column = 0
        for p in layer:
            column = column + 1
            raw_print_pixel(p)
            if column = self.width:
                print
                column = 0
    def print_layer(self, n):
        self._print_layer(self.image[n])
    def print_image(self):
        i = 0
        for layer in self.image:
            print 'Layer', i
            self._print_layer(layer)
            i = i + 1
    def layer_with_fewest_zeroes(self):
        least_zeroes = 0xffffffff
        best_layer = None
        best_idx = -1
        i = 0
        for layer in self.image:
            zeroes = count_element(layer, 0)
            if zeroes <= least_zeroes:
                best_layer = layer
                least_zeroes = zeroes
                best_layer_idx = i
            i = i + 1
        return best_layer_idx, best_layer


def test_1():
    img = Image().new('123456789012', (3, 2))
    layer = img.layer_with_fewest_zeroes()
    assert_eq(layer, (0, [1, 2, 3, 4, 5, 6]))
    assert_eq(img.layer(0), [1, 2, 3, 4, 5, 6])
    assert_eq(img.layer(1), [7, 8, 9, 0, 1, 2])


def test_count():
    assert_eq(count_element([], 0), 0)
    assert_eq(count_element([1], 0), 0)
    assert_eq(count_element([0, 1, 2], 0), 1)
    assert_eq(count_element([1, 1, 0, 3, 0, -1, 2], 0), 2)


def image_data():
    fh = open('data/day_8_input.txt', 'r')
    contents = string.strip(fh.readline())
    fh.close()
    return contents


def input_image():
    data = image_data()
    return Image().new(data, (25, 6))


def day_8_part_1():
    img = input_image()
    i, layer = img.layer_with_fewest_zeroes()
    assert_eq(i, 5)
    assert_eq(img.layer(i), img.image[i])
    num_1s = count_element(layer, 1)
    num_2s = count_element(layer, 2)
    assert_eq(num_1s * num_2s, 2318)


# There's no built-in way to access the list type-symbol, as far as I can tell.
list = type([])
# In Python 0.9.1, exceptions are strings!
AssertionError = 'Assertion failed'
def assert_eq(a, b):
    if not a = b:
        print a, '≠', b
        raise AssertionError


class TestRunner():
    def new(self):
        self.tests = 0
        self.failures = 0
        return self
    def test(self, testfn):
        self.tests = self.tests + 1
        try:
            testfn()
        except AssertionError, x:
            print 'Assertion failed in test.'
            self.failures = self.failures + 1
    def print_report(self):
        print 'Ran', self.tests, 'tests.'
        if self.failures = 0:
            print 'All tests passed!'
        else:
            print self.failures, 'failures.'


def main():
    tester = TestRunner().new()
    tester.test(test_1)
    tester.test(test_count)
    tester.test(day_8_part_1)
    tester.print_report()

main()
