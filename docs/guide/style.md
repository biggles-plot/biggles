## Styling Your Plots
The style properties of components (e.g., color) are controlled through a common set of keyword options passed during
object creation. For example,

```python
c = Curve(x, y, color="red")
```

changes the color of the curve to red. Keywords which are not relevant (for instance, setting `fontface` for a `Line`
object) are ignored. After creation, style keywords can be set using the style member function:

```python
c.style(linetype="dotted")
```

## Style Keywords
**color / fillcolor / linecolor = six-digit-hex-integer | string**

Set line or fill color (color sets both). A six digit hexadecimal integer, `0xRRGGBB`, is interpreted as an RGB triple,
where `RR`, `GG` and `BB` are the red, green and blue color contents. For instance, `0xffffff` is white and `0xff0000`
is red. Strings specify color names (eg `"red"`, `"lightgrey"`). Listings of acceptable names can be found in `rgb.txt`
(usually found in `/usr/lib/X11/`) and `colors.txt` (usually found in `/usr/share/libplot/`).

**linetype = string**

Line types are specified by name. Valid names include:

```
    "solid"          "dotdashed"
    "dotted"         "dotdotdashed"
    "shortdashed"    "dotdotdotdashed"
    "longdashed"
```

**linewidth = number**

**symbolsize = number**

**symboltype = character | string**

Symbol types can be specified by name or by character. If a character (i.e., a string of length 1) the font character
is used as the plot symbol. Valid symbol names include:

```
    "none"                    "filled circle"
    "dot"                     "filled square"
    "plus"                    "filled triangle"
    "asterisk"                "filled diamond"
    "circle"                  "filled inverted triangle"
    "cross"                   "filled fancy square"
    "square"                  "filled fancy diamond"
    "triangle"                "half filled circle"
    "diamond"                 "half filled square"
    "star"                    "half filled triangle"
    "inverted triangle"       "half filled diamond"
    "starburst"               "half filled inverted triangle"
    "fancy plus"              "half filled fancy square"
    "fancy cross"             "half filled fancy diamond"
    "fancy square"            "octagon"
    "fancy diamond"           "filled octagon"
```
**textangle = number**

Rotate text counterclockwise by this number of degrees.

**texthalign =** `"left"` **|** `"center"` **|** `"right"`

Where to horizontally anchor text strings.

**textvalign =** `"top"` **|** `"center"` **|** `"bottom"`

Where to vertically anchor text strings.
