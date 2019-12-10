#!/usr/bin/env python0.9
# Yes, Python 0.9.1, the first "beta" release of Python, from 1991!
# Get a copy here:
# https://www.python.org/download/releases/early/

def pixels_per_layer(dimensions):
    width, height = dimensions
    return width * height

def split_to_layers(image, dimensions):
    width, height = dimensions

def map_to_digits(image):
    ret = []
    for char in image:
        ret.append(int(char))
    return ret
