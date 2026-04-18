#ifndef UI_THEME_H
#define UI_THEME_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

namespace ui {
namespace theme {

using namespace ftxui;

// --- Colors ---
// Primary & Accent
const Color kPrimaryColor = Color::RGB(41, 128, 185); // Blue
const Color kSecondaryColor = Color::RGB(39, 174, 96); // Green
const Color kAccentColor = Color::RGB(231, 76, 60);    // Red
const Color kWarningColor = Color::RGB(241, 196, 15);  // Yellow

// Background & Typography
const Color kBgColor = Color::RGB(30, 30, 30);
const Color kPanelBgColor = Color::RGB(45, 45, 48);
const Color kTextColor = Color::RGB(230, 230, 230);
const Color kTextMutedColor = Color::RGB(150, 150, 150);

// Selection
const Color kSelectionBgColor = Color::RGB(60, 100, 160);
const Color kSelectionTextColor = Color::RGB(255, 255, 255);

// --- Decorators ---

inline auto WindowFrame() {
  return border; 
}

inline auto DefaultPanel() {
  return bgcolor(kPanelBgColor) | color(kTextColor) | WindowFrame();
}

inline auto FocusedPanel() {
  return bgcolor(kPanelBgColor) | color(kTextColor) | WindowFrame();
}

inline auto AppTitle() {
  return bold | color(kPrimaryColor) | bgcolor(kBgColor);
}

inline auto SelectionStyle() {
  return bgcolor(kSelectionBgColor) | color(kSelectionTextColor) | bold;
}

} // namespace theme
} // namespace ui

#endif // UI_THEME_H
