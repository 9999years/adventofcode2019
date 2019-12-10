#!/usr/bin/env python0.9
# Yes, Python 0.9.1, the first 'beta' release of Python, from 1991!
# Get a copy here:
#   https://www.python.org/download/releases/early/
# And be sure to apply the patch in ./day_8_python.patch or integer
# multiplication won't work on x64.

try:
    import string
    import sys
except NameError, x:
    # Python 0.9 note: Only single quotes (') can be used to delimit strings;
    # double quotes are not allowed.
    print 'Could not import a builtin module; check that DEFPYTHONPATH in the'
    print 'Makefile includes the "lib" directory under the Python 0.9.1 source tree.'
    raise x


# Python 0.9 note: Lists (which it calls "sequences") are the only iterable
# objects, and we have basically no tools for dealing with them. So, to make
# our lives a bit easier, we define zip() and enumerate() in the naive ways;
# unfortunately, these take O(n) memory.
def zip(as, bs):
    i = 0
    ret = []
    for a in as:
        ret.append((a, bs[i]))
        # Python 0.9 note: there are no compound assignment operators, so we
        # have to do this.
        i = i + 1
    return ret


def enumerate(itr):
    return zip(range(len(itr)), itr)


# Print a single digit as a string. The statement
#   print pixel
# adds a newline at the end, so that won't work, and sys.stdout.write only
# takes strings as arguments.
#
# Arguments:
#   pixel (int): The pixel to print
DIGITS = '0123456789'
def raw_print_pixel(pixel):
    sys.stdout.write(DIGITS[pixel])


# Arguments:
#   pixel (int): The pixel to print
SHADE = '  ░░▒▒▓▓██'
def print_pixel(pixel):
    sys.stdout.write(SHADE[pixel])


# Gets the number of layers from given image data and dimensions.
# Arguments:
#   image (sequence of int): The image data
#   width (int): The image width
#   height (int): The image height
# Returns: int
def num_layers(image, width, height):
    return len(image) / (width * height)


# Splits image data to a series of layers.
# Arguments:
#   image (sequence of int): The image data
#   width (int): The image width
#   height (int): The image height
# Returns: sequence of sequence of int
def split_to_layers(image, width, height):
    pixels_per_layer = width * height
    layers = []
    for i in range(num_layers(image, width, height)):
        i = i * pixels_per_layer
        layers.append(image[i : i + pixels_per_layer])
    return layers


# Maps a string of digits to a sequence of ints.
# Arguments:
#   image: a string of digits
# Returns: sequence of int
def map_to_digits(image):
    ret = []
    for char in image:
        # Python 0.9 note: int() only converts floats to ints by truncation, to
        # convert a string to an int we need string.atoi (!!!)
        ret.append(string.atoi(char))
    return ret


# Counts occurances of an element `el` in a sequence.
# Arguments:
#   seq (seq of T): The sequence to analyse
#   el (T): The element to count
# Returns: int
def count_element(seq, el):
    ctr = 0
    for n in seq:
        # Python 0.9 note: in conditionals, a single '=' is used for equality
        # checks.
        if n = el:
            ctr = ctr + 1
    return ctr


# A space-image-format image.
#
# Python 0.9 note: Unfortunately, suites (blocks of statements) in cannot
# contain blank lines, which makes this look... kinda cramped, and it mean that I
# can't run this file through black.
class Image():  # The parens are mandatory.
    # Python 0.9 note: There's no __init__ function, so you need to make a
    # 'new' method.
    def new(self):
        # Python 0.9 note: When you call Image().new(image, (width, height)) it
        # all gets bundled into a tuple, because arguments *are* tuples more
        # directly than they are in later Pythons. Therefore, we need to unpack
        # our arguments into local variables. Fortunately, tuple-unpacking
        # assingment is implemented, even nestedly! Unfortunately, this means
        # that if you pass the wrong arguments you get a *very* confusing error
        # at runtime.
        self, (image, (width, height)) = self
        self.width = width
        self.height = height
        self.image = split_to_layers(map_to_digits(image), width, height)
        return self
    def layer(self, n):
        return self.image[n]
    def _print_layer(self, layer):
        for column, p in enumerate(layer):
            if column > 0 and column % self.width = 0:
                print
            print_pixel(p)
        print
    def print_layer_raw(self, layer):
        for column, p in enumerate(layer):
            if column > 0 and column % self.width = 0:
                print
            raw_print_pixel(p)
        print
    def print_layer(self, n):
        self._print_layer(self.image[n])
    def print_image(self):
        for i, layer in enumerate(self.image):
            print 'Layer', i
            self._print_layer(layer)
    def layer_with_fewest_zeroes(self):
        # Python 0.9 note: the int max size is platform dependent, and this
        # magical value is "good enough, probably".
        least_zeroes = 0xffffffff
        best_layer = None
        best_idx = -1
        for i, layer in enumerate(self.image):
            zeroes = count_element(layer, 0)
            if zeroes <= least_zeroes:
                best_layer = layer
                least_zeroes = zeroes
                best_layer_idx = i
        return best_layer_idx, best_layer
    def _paint_top(self):
        # Python 0.9 note: negative indices (like self.image[-2]) are *only*
        # allowed in slice notation, e.g. self.image[-2:-1], but that turns
        # into more trouble than it's worth, so we just use len(...) - n here.
        #
        # Python 0.9 note: Linebreaks are not ignored inside parentheses, so we
        # need to use a backslash.
        new_last = paint_layers( \
            self.image[len(self.image) - 2], \
            self.image[len(self.image) - 1])
        del self.image[-2:]
        self.image.append(new_last)
    def paint(self):
        while len(self.image) > 1:
            self._paint_top()


# Returns the result of overlaying the pixel 'atop' above the pixel 'below'.
# Arguments:
#   atop (int): The top layer's pixel
#   below (int): The bottom layer's pixel
# Returns: int
PIXEL_BLACK = 0
PIXEL_WHITE = 1
PIXEL_TRANSPARENT = 2
def paint_pixel(atop, below):
    if atop = PIXEL_TRANSPARENT:
        return below
    else:
        return atop


# Returns the result of overlaying the layer 'atop' above the layer 'below'.
# Does not mutate its arguments.
# Arguments:
#   atop (seq of int): The top layer
#   below (seq of int): The bottom layer
# Returns: seq of int
def paint_layers(atop, below):
    ret = []
    for a, b in zip(atop, below):
        ret.append(paint_pixel(a, b))
    return ret


# Various unit tests.
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
    contents = string.strip(fh.readline(16000))
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


def test_paint():
    img = Image().new('0222112222120000', (2, 2))
    img.paint()
    assert_eq(img.image[0], [0, 1, 1, 0])


def day_8_part_2():
    img = input_image()
    img.paint()
    data = ( \
          '0110010010111100110011100' \
        + '1001010010100001001010010' \
        + '1001011110111001000011100' \
        + '1111010010100001000010010' \
        + '1001010010100001001010010' \
        + '1001010010100000110011100' )
    assert_eq(Image().new(data, (25, 6)).image[0], img.image[0])


# Python 0.9 note: Exceptions are strings!
AssertionError = 'Assertion failed'
def assert_eq(a, b):
    # Python 0.9 note: There's no 'not equals' operator, so we have to do 'not
    # a = b'.
    if not a = b:
        print a, '≠', b
        raise AssertionError


# A simple test-runner that counts how many tests fail. Doesn't catch other
# exceptions, because:
# Python 0.9 note: There's no way (that I could figure out) to catch all
# exceptions (e.g. a bare 'except:') while capturing the exception into a
# variable. Exception guards, of course, are implemented as string comparisons.
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
    tester.test(test_paint)
    tester.test(day_8_part_2)
    tester.print_report()


# Python 0.9 note: No __name__ or anything like that.
main()
