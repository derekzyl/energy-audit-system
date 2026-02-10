import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';

class AppTheme {
  // Modern Dark/Tech Palette
  static const Color background = Color(0xFF101014); // Deep dark background
  static const Color surface = Color(0xFF1E1E24); // Slightly lighter surface
  static const Color primary = Color(
    0xFF00E676,
  ); // Neon Green (Energy Efficient)
  static const Color secondary = Color(0xFF2979FF); // Electric Blue
  static const Color warning = Color(0xFFFFAB00); // Amber
  static const Color error = Color(0xFFFF3D00); // Deep Orange
  static const Color textHigh = Color(0xFFFFFFFF);
  static const Color textMed = Color(0xB3FFFFFF);

  static ThemeData get darkTheme {
    return ThemeData(
      brightness: Brightness.dark,
      scaffoldBackgroundColor: background,
      primaryColor: primary,
      colorScheme: const ColorScheme.dark(
        primary: primary,
        secondary: secondary,
        surface: surface,
        error: error,
      ),
      textTheme: GoogleFonts.outfitTextTheme(
        ThemeData.dark().textTheme,
      ).apply(bodyColor: textHigh, displayColor: textHigh),
      cardTheme: CardThemeData(
        color: surface,
        elevation: 0,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(16),
          side: BorderSide(color: Colors.white.withOpacity(0.05)),
        ),
      ),
      appBarTheme: const AppBarTheme(
        backgroundColor: Colors.transparent,
        elevation: 0,
        centerTitle: true,
      ),
      useMaterial3: true,
    );
  }
}
