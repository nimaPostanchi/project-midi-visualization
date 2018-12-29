#pragma once

#include <iostream>

/// <summary>
/// Color type.
/// </summary>
struct Color final
{
    /// <summary>
    /// Red component.
    /// </summary>
    double r;
        
    /// <summary>
    /// Green component.
    /// </summary>
    double g;
            
    /// <summary>
    /// Blue component.
    /// </summary>
    double b;

    /// <summary>
    /// Default constructor. Initialized the color to black.
    /// </summary>
    constexpr Color() : Color(0, 0, 0) { }

    /// <summary>
    /// Constructor.
    /// </summary>
    constexpr Color(double r, double g, double b)
        : r(r), g(g), b(b) { }

    /// <summary>
    /// Ensures all color components (r, g, b) are within the [0, 1] interval.
    /// If a color component is less than zero, it is set to zero.
    /// If a color component is greater than one, it is set to one.
    /// </summary>
    void clamp();

    /// <summary>
    /// Returns a new color which is equal to this color, except that
    /// all color components moved to the [0, 1] interval. See clamp.
    /// </summary>
    Color clamped() const;

    /// <summary>
    /// Inverts this color.
    /// </summary>
    void invert();

    /// <summary>
    /// Returns the inversion of this color.
    /// </summary>
    Color inverted() const;
};

Color operator +(const Color&, const Color&);
Color operator -(const Color&, const Color&);
Color operator *(const Color&, double);
Color operator *(double, const Color&);
Color operator *(const Color&, const Color&);
Color operator /(const Color&, double);

Color& operator +=(Color&, const Color&);
Color& operator -=(Color&, const Color&);
Color& operator *=(Color&, double);
Color& operator /=(Color&, double);

bool operator ==(const Color&, const Color&);
bool operator !=(const Color&, const Color&);

std::ostream& operator <<(std::ostream&, const Color&);

// Example usage: Color c = colors::black();
namespace colors
{
    constexpr Color black()        { return Color{ 0, 0, 0 }; }
    constexpr Color white()        { return Color{ 1, 1, 1 }; }
    constexpr Color red()          { return Color{ 1, 0, 0 }; }
    constexpr Color green()        { return Color{ 0, 1, 0 }; }
    constexpr Color blue()         { return Color{ 0, 0, 1 }; }
    constexpr Color yellow()       { return Color{ 1, 1, 0 }; }
    constexpr Color magenta()      { return Color{ 1, 0, 1 }; }
    constexpr Color cyan()         { return Color{ 0, 1, 1 }; }
    constexpr Color orange()       { return Color{ 1, 0.64, 0 }; }
}
