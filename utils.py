import numpy as np
from skimage.measure import block_reduce

__all__ = ['downsample']


def get_luminosity((r, g, b)):
    # https://en.wikipedia.org/wiki/Relative_luminance
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def downsample(data, factor_x, factor_y):
    data = block_reduce(data, (factor_x, factor_y, 1))

    y, x, _ = data.shape
    data = np.reshape(data, (y * x, 3))
    data = np.apply_along_axis(get_luminosity, 1, data)
    data = np.reshape(data, (y, x))

    return data
